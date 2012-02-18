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

char *infile = "<stdin>";
unsigned int address = 0, line = 1;

int main(int argc, char *argv[]) {
	FILE *infd = stdin;
	FILE *outfd = stdout;
	char *a, *code = NULL;
	int i, len, disasm = 0;

	for (i = 0; i < argc; ++i) {
		if (*(argv[i]) == '-') {
			a = argv[i] + 1;
			argv[i] = NULL;
			while (*a) {
				switch (*a) {
					case 'd':
						++disasm;
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
		assemble(code);
		free(code); /* release the monster */
		len = getihex(&code);
	} else {
		readihex(code);
		free(code);
		len = disassemble(&code);
	}
	fwrite((void *) code, 1, len, outfd);
	cleanup();

	return EXIT_SUCCESS;
}
