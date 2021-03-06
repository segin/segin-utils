#include <stdlib.h> /* NULL */
#include "ppc.h"
#include "as.h" /* parseargs(), parseln() */
#include "mem.h"
#include "str.h" /* skipsp(), ctype(), etc */
#include "misc.h" /* flerrexit(), readfile(), file, infile */
#include "str.h"
#include "io.h"
#include "lineno.h"

arr_t defines;
arr_t macros;
arr_t files;
arr_t reps;
arr_t amacros;
arr_t adef;

int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc;
char *repstart, *data;

void strcheck(char c) {
	switch (c) {
		case '\\':
			if (str)
				++esc;
			return;
		case '"':
			if (!(str & 1 && esc))
				str ^= 1;
			break;
		case '\'':
			if (!(str & 2 && esc))
				str ^= 2;
			break;
		case '\n':
			str = 0;
			break;
	}
	esc = 0;
}

void strip(char *in) {
	{ /* strip comments */
		char *p = in;
		int iscomment = 0, nestcomment = 0;

		esc = str = 0;
		while (*p) {
			if (!iscomment && !nestcomment)
				strcheck(*p);
			if (!str) {
				if (p[0] == '/' && p[1] == '*') {
					++nestcomment;
				} else if (nestcomment && p[0] == '*' && p[1] == '/') {
					--nestcomment;
					p[0] = p[1] = ' ';
					++p;
				}
				if (iscomment || nestcomment) {
					if (ctype(*p) & CT_NL)
						iscomment = 0;
					else *p = ' ';
				} else if (*p == ';') {
					*p = ' ';
					iscomment = 1;
				}
			}
			++p;
		}
	}

	{ /* fix escaped newlines and remove dos newlines */
		char *p, *c;
		int i, nl = 0;
		p = c = in;
		while (*c) {
			switch (*c) {
				case '\\':
					i = 1;
					while (c[i] == '\r')
						++i;
					if (c[1] == '\n') {
						c += i + 1;
						++nl;
					}
					break;
				case '\r':
					/* dos newlines will be added by mfwrite(), if the user wants them in output */
					++c;
					break;
				case '\n':
					if (nl)
						while (nl--)
							*(p++) = '\n';
			}
			*p = *c;
			++p, ++c;
		}
		*p = 0;
	}
}

define_t *deffind(char *name) {
	define_t *def = ((define_t *) defines.data);
	int i;
	for (i = 0; i < defines.count; ++i, ++def)
		if (cmpid(def->name, name))
			return def;
	return NULL;
}

macro_t *macrofind(char *name) {
	macro_t *mac = ((macro_t *) macros.data);
	int i;
	for (i = 0; i < macros.count; ++i, ++mac)
		if (cmpid(mac->name, name))
			return mac;
	return NULL;
}

char *sppsub(char *in, char end);

sll ppgetnum(char **in) {
	sll ret;
	char *p, *t;
	if (**in == '[') {
		++*in;
		p = t = sppsub(*in, ']');
		ret = numarg(&p);
		if (*p)
			flerrexit("exess data in preprocessor evaluation group");
		free(t);
		while (*((*in)++) != ']'); /* sppsub made sure ']' is there */
	} else return getval(in);
	return ret;
}

typedef struct {
	define_t *parent;
	char *argv[ARG_MAX];
	int argc;
	char *in;
} sstate_t;

void _ppsub(ioh_t *out, char *in, amacro_t *am, char end) {
	int i;
	char *s, *id;
	define_t *def;
	arr_t sstack;
	sstate_t state = { NULL };

	arr_new(&sstack, sizeof(sstate_t));
	esc = str = 0;
	while (1) {
		s = in;
		while (!(ctype(*in) & (CT_NL | CT_NUL)) && (str || (!(ctype(*in) & (CT_LET | CT_SEP)) && *in != '[' && *in != '@' && *in != '#' && *in != end))) {
			strcheck(*in);
			++in;
		}
		mfwrite(out, s, in - s);
		if (*in == '#') {
			++in;
			if (*in == '#')
				++in;
			else mfwrite(out, "#", 1);
			continue;
		}
		if (*in == end || ctype(*in) & (CT_NL | CT_NUL)) {
			sstate_t *s = arr_top(sstack, sstate_t);
			s = arr_pop(sstack, sstate_t);
			if (s == NULL) {
				if (end != 0 && *in != end)
					flerrexit("expecting '%c'", end);
				break;
			}
			--(state.parent->active);
			for (--state.argc; state.argc >= 0; --state.argc)
				free(state.argv[state.argc]);
			memcpy(&state, s, sizeof(sstate_t));
			in = state.in;
			continue;
		}
		id = in;
		if (!str) {
			if (*in == '@') {
				++in;
				if (*in == '@') {
					++in;
					if (reps.count == 0)
						flerrexit("'@@' outside of rep block");
					mfprintf(out, "%ut", arr_top(reps, rep_t)->repno);
					continue;
				}
				i = ppgetnum(&in);
				if (am == NULL)
					flerrexit("macro argument requested outside of macro");
				if (i > am->argc - am->macro->argc)
					flerrexit("macro wants nonexistant argument @%i", i);
				if (i == 0)
					mfprintf(out, "%ut", am->argc - am->macro->argc);
				else mfprint(out, am->argv[am->macro->argc + i - 1]);
				continue;
			}
			if (*in == '[') {
				/* bracelets around the number cause it could be signed */
				mfprintf(out, "(%lldt)", (sll) ppgetnum(&in));
				continue;
			}
			id = in;
			getid(&in);
			if (state.parent != NULL && state.argc) { /* substitute %define arguments */
				for (i = 0; i < state.argc; ++i) {
					if (cmpid(id, state.parent->argv[i])) {
						_ppsub(out, state.argv[i], NULL , 0);
						break;
					}
				}
				if (i != state.argc)
					continue;
			}

			if (am != NULL && am->argc) { /* substitute macro arguments */
				for (i = 0; i < am->macro->argc; ++i) {
					if (cmpid(id, am->macro->argv[i])) {
						_ppsub(out, am->argv[i], NULL, 0);
						break;
					}
				}
				if (i != am->macro->argc)
					continue;
			}

			if ((def = deffind(id)) != NULL && !def->active) {
				arr_add(&sstack, &state);
				state.parent = def;
				state.argc = 0;
				if (*in == '(') {
					++in;
					while (!(ctype(*in) & (CT_NL | CT_NUL))) {
						s = in;
						if (state.argc + 1 < def->argc)
							while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ',' && *in != ')')
								++in;
						else while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')')
								++in;
						state.argv[state.argc] = mstrldup(s, in - s);
						if (state.argv[state.argc] == NULL)
							errexit("mstrldup() failure");
						++state.argc;
						if (state.argc == ARG_MAX)
							flerrexit("too many arguments for macro");
						if (*in == ')')
							break;
						if (*in == ',')
							++in;
					}
					if (*in != ')')
						flerrexit("missing ')'");
					++in;
				}
				if (state.argc < def->argc)
					flerrexit("too few arguments for macro");
				state.parent = def;
				arr_top(sstack, sstate_t)->in = in;
				in = def->val;
				++def->active;
				continue;
			}
		}
		mfwrite(out, id, in - id);
	}
	arr_free(&sstack);
}

void ppsub(ioh_t *out, char *in, char end) {
	_ppsub(out, in, arr_top(amacros, amacro_t), end);
}

char *sppsub(char *in, char end) {
	char *ret;
	ioh_t *h = mmemopen(0);
	if (h == NULL)
		errexit("mmemopen() failed");
	ppsub(h, in, end);
	mfwrite(h, "\0", 1);
	ret = mmemget(h);
	if (ret == NULL)
		errexit("out of memory");
	mfclose(h);
	return ret;
}

void skipblock(char *start, char *end) {
	int d = 0;
	char *s = nextln;
	run = 0;
	lineno_inc();
	while (1) {
		if (!parseln(s))
			flerrexit("end of file within '%s' block", start);
		if (ip != NULL) {
			if (cmpid(ip, start))
				++d;
			if (cmpid(ip, end)) {
				if (argp != NULL)
					flerrexit("'%s' takes no arguments", end);
				if (d == 0)
					break;
				--d;
			}
		}
		if (run == 0) {
			lineno_inc();
			s = nextln;
		}
	}
	run = 0;
}

void define(char *argp, int eval) {
	define_t def, *p;
	char *s;
	if (argp == NULL)
		flerrexit("too few arguments for define directive");
	if (*argp == '<') {
		++argp;
		s = sppsub(argp, '>');
		arr_add(&garbage, &s);
		while (*(argp++) != '>'); /* sppsub() shoulda made sure it is there */
		skipsp(&s);
		def.name = getid(&s);
		skipsp(&s);
		if (def.name == NULL || *s)
			flerrexit("macro name expands to nothing useful");
	} else def.name = getid(&argp);
	def.argc = 0;
	if  (*argp == '(') {
		++argp;
		while (1) {
			s = getid(&argp);
			skipsp(&argp);
			if (s == NULL)
				flerrexit("syntax error in macro parameter list");
			if (s != NULL) {
				def.argv[def.argc] = s;
				++def.argc;
			}
			if (*argp == ')') {
				++argp;
				break;
			}
			if (*argp != ',')
				flerrexit("syntax error in macro parameter list");
			++argp;
			skipsp(&argp);
			if (def.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
		if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on define directive");
	} else if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error on define directive");
	if (eval) {
		def.val = sppsub(argp, 0);
		arr_add(&garbage, &def.val);
	} else def.val = argp;
	def.active = 0;
	if ((p = deffind(def.name)) != NULL)
		memcpy(p, &def, sizeof(define_t));
	else arr_add(&defines, &def);
}

void macro(ioh_t *out, char *argp) {
	macro_t mac;
	char *s;

	if (ignore) {
		skipblock("macro", "endm");
		mfprintf(out, "%%line %ut", lineno_get());
		return;
	}

	if (argp == NULL)
		flerrexit("too few arguments");
	mac.active = 0;
	if (*argp == '<') {
		s = sppsub(argp + 1, '>');
		arr_add(&garbage, &s);
		mac.name = getid(&s);
		if (mac.name == NULL || *s)
			flerrexit("macro name expands to nothing useful");
		while (*(argp++) != '>'); /* again sppsub ensured us end char is there */
	} else {
		s = getid(&argp);
		if (s == NULL)
			flerrexit("syntax error on macro directive");
		mac.name = mstrldup(s, idlen(s));
		arr_add(&garbage, &mac.name);
	}
	mac.argc = 0;
	mac.file = lineno_getrealfile();
	mac.line = lineno_getreal();
	if (skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL))) {
		while (1) {
			s = getid(&argp);
			skipsp(&argp);
			if (s == NULL)
				flerrexit("syntax error in macro parameter list");
			mac.argv[mac.argc] = s;
			++mac.argc;
			if (ctype(*argp) & (CT_NL | CT_NUL))
				break;
			if (*argp != ',')
				flerrexit("syntax error in macro parameter list");
			++argp;
			skipsp(&argp);
			if (mac.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
	}
	if (!(ctype(*argp) & (CT_NL | CT_NUL)))
		flerrexit("syntax error on macro directive");

	mac.val = nextln;
	skipblock("macro", "endm");
	mfprintf(out, "%%line %ut", lineno_get() + 1);

	{ /* add macro */
		macro_t *p;
		if ((p = macrofind(mac.name)) != NULL)
			memcpy(p, &mac, sizeof(macro_t));
		else arr_add(&macros, &mac);
	}
}

int ppfind(ioh_t *out, char *ip, char *argp) {
	char *s;

	if (cmpid(ip, "endm")) {
		if (amacros.count) {
			amacro_t *mac = arr_pop(amacros, amacro_t);
			--((macro_t *) (mac->macro))->active;
			for (--mac->argc; mac->argc >= 0; --mac->argc)
				free(mac->argv[mac->argc]);
			nextln = mac->nextln;
			lineno_dropctx();
			mfprint(out, "%endexp");
		} else flerrexit("endm without prior macro directive");
		return 1;
	}
	if (cmpid(ip, "macro")) {
		macro(out, argp);
		return 1;
	}
	if (cmpid(ip, "if")) {
		sll arg;
		if (argp == NULL)
			flerrexit("too few arguments for if directive");
		argp = sppsub(argp, 0);
		parseargs(argp, "n", &arg);
		++level;
		if (!ignore && !arg)
			ignore = level;
		free(argp);
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		parseargs(argp, "i", &s);
		++level;
		if (!ignore && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		parseargs(argp, "i", &s);
		++level;
		if (!ignore && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			flerrexit("too many arguments");
		if (!level)
			flerrexit("endif without prior if/ifdef/ifndef");
		if (level == ignore)
			ignore = 0;
		--level;
		return 1;
	}
	if (cmpid(ip, "else")) {
		if (argp != NULL)
			flerrexit("syntax error on else directive");
		if (!level)
			flerrexit("else without prior if/ifdef/ifndef");
		if (level == ignore || !ignore)
			ignore = (ignore ? 0 : level);
		return 1;
	}
	if (ignore)
		return 0;
	if (cmpid(ip, "endrep")) {
		rep_t *rep = arr_top(reps, rep_t);
		if (reps.count == 0)
			flerrexit("endrep without prior 'rep'");
		if (--rep->count > 0) {
			++rep->repno;
			nextln = rep->start;
			lineno_set(rep->line);
			mfprintf(out, "%%line %ut", lineno_getreal() + 1);
			run = 0;
		} else --reps.count;
		return 1;
	}
	if (cmpid(ip, "rep")) {
		sll arg;
		argp = sppsub(argp, 0);
		parseargs(argp, "n", &arg);
		free(argp);
		if (arg) {
			rep_t rep;
			rep.count = arg;
			rep.start = nextln;
			rep.repno = 0;
			rep.line = lineno_get();
			arr_add(&reps, &rep);
			run = 0;
		} else {
			skipblock("rep", "endrep");
			mfprintf(out, "%%line %ut", lineno_getreal() + 1);
		}
		return 1;
	}
	if (cmpid(ip, "xdefine")) {
		define(argp, 1);
		return 1;
	}
	if (cmpid(ip, "define")) {
		define(argp, 0);
		return 1;
	}
	if (cmpid(ip, "undef")) {
		define_t *def;
		parseargs(argp, "i", &s);
		def = deffind(s);
		if (def != NULL) {
			--defines.count;
			memcpy(def, ((define_t *) defines.data + defines.count), sizeof(define_t));
		}
		return 1;
	}
	if (cmpid(ip, "msg")) {
		parseargs(argp, "s", &s);
		flmsg(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "error")) {
		parseargs(argp, "s", &s);
		flerrexit(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "warn")) {
		parseargs(argp, "s", &s);
		flwarn(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "include")) {
		file_t ofile;
		parseargs(argp, "s", &s);
		data = readfile(s);
		if (data == NULL)
			flerrexit("failed to include file '%s'", s);
		ofile.nextln = nextln;
		lineno_pushfile(s, 0, 1);
		arr_add(&files, &ofile);
		mfprintf(out, "%%file \"%s\"", s);
		nextln = data;
		strip(data);
		arr_add(&garbage, &data);
		return 1;
	}
	if (cmpid(ip, "radix")) {
		setradix(argp);
		/* we want to reprint it instead of just return 0
		 * to make sure that this directive is not preprocessed */
		mfprint(out, "%radix ");
		mfwrite(out, argp, idlen(argp));
		return 1;
	}
	{
		macro_t *mac;

		if ((mac = macrofind(ip)) != NULL) {
			amacro_t am;
			char *e, *p;
			if (mac->active)
				return 0;
			am.argc = 0;
			if (argp != NULL) {
				p = argp = mstrldup(argp, linelen(argp));
				argp = sppsub(argp, 0);
				free(p);
				p = argp;
				while (1) {
					s = argp;
					while (!(ctype(*argp) & (CT_NL | CT_NUL)) && *argp != ',')
						++argp;
					if (s == argp)
						flerrexit("syntax error");
					e = argp;
					while (ctype(*(e - 1)) & CT_SPC)
						--e;
					am.argv[am.argc] = mstrldup(s, e - s);
					if (am.argv[am.argc] == NULL)
						errexit("mstrldup() failure");
					++am.argc;
					if (ctype(*argp) & (CT_NL | CT_NUL))
						break;
					if (*argp == ',') {
						++argp;
						skipsp(&argp);
						if (ctype(*argp) & (CT_NL | CT_NUL))
							flerrexit("syntax error");
					}
					if (am.argc == ARG_MAX)
						flerrexit("too many arguments for macro");
				}
				free(p);
			}
			if (am.argc < mac->argc)
				flerrexit("too few arguments for macro");
			am.nextln = nextln;
			am.macro = mac;
			arr_add(&amacros, &am);
			++mac->active;
			nextln = mac->val;
			lineno_pushmacro(mac->name, mac->file, mac->line); /* TODO keep file and line */
			mfprintf(out, "%%expands %s, \"%s\", %ut", mac->name, mac->file, mac->line);
			run = 0;
			return 1;
		}
	}
	return 0;
}

/* preprocess(in, ret)
 *
 * description
 *   strips comments from in (on the string itself replaces them with space)
 *   evaluates preprocessor directives and builds with them preprocessed data
 *   that will be sent to out i/o handle
 *
 */
void preprocess(ioh_t *out, char *in) {
	int r;
	file_t *f;
	char *label;
	ioh_t *tout;

	arr_new(&defines, sizeof(define_t));
	arr_new(&macros, sizeof(macro_t));
	arr_new(&files, sizeof(file_t));
	arr_new(&reps, sizeof(rep_t));
	arr_new(&amacros, sizeof(amacro_t));
	lineno_init();
	lineno_pushfile(infile, 1, 0);
	level = ignore = 0;

	tout = mmemopen(1);
	if (tout == NULL)
		errexit("mmemopen fail");

	strip(in);
	proceed:
	run = 0;
	while (parseln(in)) {
		r = 0;
		label = lp;
		if (ip != NULL) {
			mftrunc(tout, 0);
			if ((r = ppfind(tout, ip, argp))) {
				if (label != NULL) {
					mfwrite(out, label, idlen(label));
					mfprint(out, ":");
				}
				mfwrite(out, mmemget(tout), mmemlen(tout));
				run = 0;
			}
		}
		if (prefix && !r) {
			flwarn("unhandled preprocessor directive");
			run = 0;
		}
		if (run == 0) {
			if (!r && !ignore)
				ppsub(out, in, 0);
			mfprint(out, "\n");
			lineno_inc();
			in = nextln;
		}
	}
	if (reps.count)
		flerrexit("expected 'endrep' before EOF");
	if ((f = arr_pop(files, file_t)) != NULL) {
		in = f->nextln;
		lineno_dropctx();
		lineno_inc();
		mfprintf(out, "%%endfile\n");
		goto proceed;
	}

	mfclose(tout);
	lineno_end();
	for (--garbage.count; garbage.count >= 0; --garbage.count)
		free(*((void **) garbage.data + garbage.count));
	arr_free(&amacros);
	arr_free(&reps);
	arr_free(&files);
	arr_free(&macros);
	arr_free(&defines);
}
