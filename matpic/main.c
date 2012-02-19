/***********************
 * assembler interface *
 ***********************/

#include "host.h" /* EXIT_SUCCESS, errexit(), FILE, stdin, stdout, fread(),
                     realloc(), free(), NULL */
#include "as.h"
#include "dis.h"
#include "misc.h" /* errexit(), cleanup() */
#include "mem.h" /* BLOCK */
#include "ihex.h"
#include "ppc.h"

char *infile = "<stdin>";
unsigned int address = 0, line = 1;

int main(int argc, char *argv[]) {
	FILE *infd = stdin;
	FILE *outfd = stdout;
	char *a, *code = NULL, *pcode;
	int i, len;
	/* teh command line options */
	int disasm = 0, ppm = 0, through = 0;

	for (i = 0; i < argc; ++i) {
		if (*(argv[i]) == '-') {
			a = argv[i] + 1;
			argv[i] = NULL;
			while (*a) {
				switch (*a) {
					case 'd':
						++disasm;
						break;
					case 'p':
						ppm = 1; /* only preprocess */
						break;
					case 'P':
						ppm = 2; /* don't preprocess */
						break;
					case 't': /* assemble then disassemble */
						++through;
						break;
					default:
						errexit("invalid argument");
				}
				++a;
			}
		}
	}

	{ /* read teh filez */
		int pos = 0, mem = 0;

		while (!feof(infd)) {
			if (ferror(infd))
				errexit("failed to read file");
			if (pos == mem) {
				if (mem + BLOCK < mem)
					errexit("wtf integer overflow");
				code = (char *) realloc((void *) code, mem + BLOCK);
				mem += BLOCK;
				if (code == NULL)
					errexit("out of memory");
			}
			pos += fread((void *) code, 1, mem - pos, infd);
		}
		code[pos] = 0;
	}

	if (!disasm) {
		if (ppm == 1) { /* only preprocess */
			len = preprocess(code, &code);
			goto done;
		} if (ppm == 2) { /* no preprocess */
			pcode = code;
		} else preprocess(code, &pcode);
		assemble(pcode);
		free(code); /* release the monster */
		free(pcode);
		len = getihex(&code);
	}
	if (disasm || through) {
		readihex(code);
		free(code);
		len = disassemble(&code);
	}
	done:
	fwrite((void *) code, 1, len, outfd);
	cleanup();

	return EXIT_SUCCESS;
}

