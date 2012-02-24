/*************
 * assembler *
 *************/

#include <stdlib.h> /* NULL */
#include <string.h> /* memcpy() */
#include "as.h"
#include "str.h"
#include "mem.h"
#include "arch.h"
#include "misc.h" /* getargs(), clearfile(), getstr(), file, address, line */

int llbl = -1;

arr_t inss = { NULL, 0, 0, 0 };
arr_t labels = { NULL, 0, 0, 0 };

int countargs(char *src) {
	int n = 1;
	if (src == NULL)
		return 0;
	while(!(alfa[(unsigned char) *src] & (CT_NUL | CT_NL))) {
		if (*src == ',')
			++n;
		++src;
	}
	return n;
}

int insfind(char *lp, char *ip, char *argp) {
	oc_t *oc = arch->ocs;
	ins_t ins;
	int args[ARG_MAX];

	ins.line = line;
	if (cmpid(ip, "org")) {
		if (getargs(&argp, args) != 1)
			flerrexit("invalid number of arguments to org directive");
		ins.type = IT_ORG;
		ins.d.org.address = args[0] * arch->align;
		address = ins.d.org.address;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "file")) {
		if (argp == NULL)
			flerrexit("'file' directive needs an argument");
		clearfile();
		file = getstr(&argp, 0);
		if (!(alfa[(unsigned char) *argp] & (CT_NUL | CT_NL)))
			flerrexit("invalid data after file directive");
		line = 0; /* will be incremented soon enough */
		ins.type = IT_FIL;
		ins.d.file.file = argp;
		arr_add(&inss, &ins);
		return 1;
	}
	if (cmpid(ip, "line")) {
		if (getargs(&argp, args) != 1)
			flerrexit("'line' directive wants exactly 1 argument");
		line = args[0] - 1;
		return 1;
	}
	if (cmpid(ip, "data")) {
		int n = countargs(argp);
		ins.type = IT_DAT;
		ins.d.data.args = argp;
		address += n * arch->align;
		for (; n > 0; --n)
			arr_add(&inss, &ins);
		return 1;
	}

	while (oc->name != NULL) {
		if (cmpid(ip, oc->name)) {
			ins.type = IT_INS;
			memcpy((void *) ins.d.ins.oc, (void *) oc->oc, sizeof(oc->oc));
			ins.d.ins.len = oc->len;
			ins.d.ins.atype = oc->atype;
			ins.d.ins.args = argp;
			arr_add(&inss, &ins);
			address += ins.d.ins.len;
			return 1;
		}
		++oc;
	}
	return 0;
}

void assemble(char *code) {
	arr_new(&inss, sizeof(ins_t));
	arr_new(&labels, sizeof(label_t));

	{ /* first pass */
		label_t label, *li;
		ins_t ins;
		char *lp, *ip, *argp;
		int wp, i;
		unsigned int addrl;

		clearfile();
		line = 1;
		address = 0;
		while (*code) {
			ins.line = line;
			addrl = address; /* case there is a label we have the address at start of line */
			lp = NULL;
			argp = NULL;
			wp = getword(&code, &ip);
			if (wp & (WP_LABEL | WP_LOCAL)) { /* we have label and we're sure about it */
				if (!(wp & WP_TSPC) && !(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
					flerrexit("invalid character in local label"); /* can only happen with local label */
				lp = ip;
				wp = getword(&code, &ip);
			}
			if (ip == NULL) {
				if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
					flerrexit("invalid identifier");
				goto endln;
			}
			if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = code;
				else flerrexit("invalid identifier");
			}
			if (insfind(lp, ip, argp))
				goto endln;
			if (lp == NULL) {
				if (!(wp & WP_PSPC)) {
					lp = ip;
					wp = getword(&code, &ip);
					if (ip == NULL) {
						if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL)))
							flerrexit("invalid identifier");
						goto endln;
					}
					if (!(alfa[(unsigned char) *code] & (CT_NL | CT_NUL))) {
						if (wp & WP_TSPC)
							argp = code;
						else flerrexit("invalid identifier");
					}
					if (!insfind(lp, ip, argp))
						flerrexit("no such instruction/directive");
				} else flerrexit("no such instruction/directive");
			}
			endln:
			if (lp != NULL) { /* we has a lebel */
				label.name = lp;
				label.address = addrl / arch->align;
				for (label.local = 0; *lp == '.'; ++label.local, ++lp);
				label.parent = -1;
				for (i = labels.count - 1; i >= 0; --i) { /* find owner */
					li = (label_t *) ((label_t *) labels.data) + i;
					if (li->local < label.local) {
						label.parent = i;
						break;
					}
				}
				for (i = 0; i < labels.count; ++i) { /* check if already exists */
					li = (label_t *) ((label_t *) labels.data) + i;
					if (cmpid(li->name, label.name) && label.parent == li->parent)
						flerrexit("duplicate label");
				}
				arr_add(&labels, &label);
				ins.type = IT_LBL;
				ins.d.lbl.lbl = labels.count - 1;
				arr_add(&inss, &ins);
			}
			while (!(alfa[(unsigned char) *code] & (CT_NUL | CT_NL)))
				++code;
			skipnl(&code);
			++line;
		}
		ins.type = IT_END;
		arr_add(&inss, &ins);
	}
	{ /* second pass */
		ins_t *ins = (ins_t *) inss.data;
		int i, c, args[ARG_MAX];
		char *s;

		clearfile();
		address = 0;

		while (ins->type != IT_END) {
			line = ins->line;
			switch (ins->type) {
				case IT_INS:
					c = getargs(&(ins->d.ins.args), args);
					arch->acmp(ins->d.ins.oc, ins->d.ins.atype, c, args);
					++address;
					break;
				case IT_ORG:
					address = ins->d.org.address;
					break;
				case IT_DAT:
					c = getargs(&(ins->d.data.args), args);
					for (i = 0; i < c; ++i) {
						ins->d.data.value = args[i];
						++ins;
					}
					if (i)
						--ins;
					address += c;
					break;
				case IT_FIL:
					clearfile();
					s = ins->d.file.file;
					file = getstr(&s, 0);
					break;
				case IT_LBL:
					llbl = ins->d.lbl.lbl;
					break;
			}
			++ins;
		}
	}
	llbl = -1; /* important */
	clearfile();
}
