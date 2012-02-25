#include <stdlib.h> /* NULL */
#include "mem.h"
#include "str.h" /* skipsp(), ctype(), etc */
#include "misc.h" /* flerrexit(), getword(), readfile(), getstr(), clearfile(), file */
#include "ppc.h"
#include "io.h"

arr_t defines = { NULL, 0, 0, 0 };
arr_t macros = { NULL, 0, 0, 0 };
macro_t *cmacro;

int level, ignore; /* depth & state of if/ifdef/ifndef directives */
int str, esc, macro, maclvl;

void strcheck(char c) {
	switch (c) {
		case '\\':
			if (str)
				++esc;
			return;
		case '"':
			if (!(str && esc) || str & 2)
				str ^= 1;
			break;
		case '\'':
			str ^= 2;
			break;
		case '\n':
			str = 0;
			break;
	}
	esc = 0;
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

void _ppsub(ioh_t *out, char *in, define_t *parent, int argc, char *argv[], char end) {
	char *s, *id;
	char *_argv[ARG_MAX];
	int _argc, i;
	define_t *def;

	macro = esc = str = 0;
	while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != end) {
		s = in;
		while (!(ctype(*in) & (CT_NL | CT_NUL | CT_LET | CT_SEP)) && *in != '[') {
			if (esc)
				esc = 0;
			else strcheck(*in);
			++in;
		}
		mfwrite(out, s, in - s);
		if (!str && *in == '[') {
			char *p;
			++in;
			p = id = sppsub(in, ']');
			mfprintf(out, "%u", numarg(&p));
			if (*p)
				flerrexit("exess data in preprocessor evaluation group");
			free(id);
			while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ']')
				++in;
			if (*in != ']')
				flerrexit("expected ']' after expression");
			++in;
			continue;
		}
		id = in;
		getid(&in);
		if (!str) {
			if (parent != NULL && argc) {
				for (i = 0; i < argc; ++i) {
					if (cmpid(id, parent->argv[i])) {
						_ppsub(out, argv[i], NULL, 0, NULL, 0);
						break;
					}
				}
				if (i != argc)
					continue;
			}

			if ((def = deffind(id)) != NULL) {
				_argc = 0;
				if (*in == '(') {
					++in;
					while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')') {
						s = in;
						if (_argc + 1 < def->argc)
							while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ',' && *in != ')')
								++in;
						else while (!(ctype(*in) & (CT_NL | CT_NUL)) && *in != ')')
								++in;
						_argv[_argc] = strldup(s, in - s);
						if (_argv[_argc] == NULL)
							errexit("strldup() failure");
						if (*in == ',')
							++in;
						++_argc;
						if (_argc == ARG_MAX)
							flerrexit("too many arguments for macro");
					}
					if (*in != ')')
						flerrexit("missing ')'");
					++in;
				}
				if (_argc < def->argc)
					flerrexit("too few arguments for macro");

				def->active = 1;
				_ppsub(out, def->val, def, _argc, _argv, 0);
				def->active = 0;
				for (--argc; argc >= 0; --argc)
					free(argv[argc]);
				continue;
			}
		}
		mfwrite(out, id, in - id);
	}
}

void ppsub(ioh_t *out, char *in, char end) {
	_ppsub(out, in, NULL, 0, NULL, end);
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

void _preprocess(ioh_t *out, char *in, int lvl);

int ppfind(ioh_t *out, char *lp, char *ip, char *argp) {
	char *s;
	int wp;
	int args[ARG_MAX];

	if (cmpid(ip, "endm")) {
		if (macro) {
			macro = 0;
			return 1;
		}
		if (maclvl > 0) {
			--maclvl;
			return 1;
		}
		errexit("endm without prior macro directive");
	}
	if (macro)
		return 1;
	if (cmpid(ip, "if")) {
		if (argp == NULL)
			flerrexit("too few arguments for if directive");
		s = argp = sppsub(argp, 0);
		if (getargs(&argp, args) != 1)
			flerrexit("too many arguments for if directive");
		++level;
		if (!ignore && !args[0])
			ignore = level;
		free(s);
		return 1;
	}
	if (cmpid(ip, "ifdef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifdef directive");
		++level;
		if (!ignore && deffind(s) == NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "ifndef")) {
		wp = getword(&argp, &s);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on ifndef directive");
		++level;
		if (!ignore && deffind(s) != NULL)
			ignore = level;
		return 1;
	}
	if (cmpid(ip, "endif")) {
		if (argp != NULL)
			flerrexit("syntax error on endif directive");
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
	if (cmpid(ip, "define")) {
		define_t def, *p;
		wp = getword(&argp, &def.name);
		if (def.name == NULL)
			flerrexit("syntax error on define directive");
		def.argc = 0;
		if  (!(wp & WP_TSPC) && *argp == '(') {
			++argp;
			while (1) {
				getword(&argp, &s);
				if (s == NULL && *argp != ')')
					flerrexit("syntax error in macro parameter list");
				if (s != NULL) {
					def.argv[def.argc] = s;
					++(def.argc);
				}
				if (*argp == ')') {
					++argp;
					break;
				}
				if (*argp != ',')
					flerrexit("syntax error in macro parameter list");
				++argp;
				if (def.argc == ARG_MAX)
					flerrexit("too many arguments for macro");
			}
			if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
				flerrexit("syntax error on define directive");
		} else {
			if (!(wp & WP_TSPC) && !(ctype(*argp) & (CT_NL | CT_NUL)))
				flerrexit("syntax error on define directive");
		}
		def.val = argp;
		def.active = 0;
		if ((p = deffind(def.name)) != NULL)
			memcpy(p, &def, sizeof(define_t));
		else arr_add(&defines, &def);
		return 1;
	}
	if (cmpid(ip, "msg")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flmsg(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "error")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flerrexit(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "warn")) {
		if (!argp)
			flerrexit("too few arguments for msg directive");
		s = getstr(&argp, 1);
		skipsp(&argp);
		if (s == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on msg directive");
		flwarn(s);
		free(s);
		return 1;
	}
	if (cmpid(ip, "include")) {
		char *data, *ofile = file, *s = argp;
		int oline = line;
		if (!argp)
			flerrexit("too few arguments for msg directive");
		file = getstr(&argp, 0);
		if (file == NULL || !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on include directive");
		s = getstr(&s, 1);
		data = readfile(s);
		free(s);
		if (data == NULL)
			flerrexit("failed to include file '%s'", file);
		mfprintf(out, "file \"%s\"\n", file);
		line = 1;
		_preprocess(out, data, maclvl);
		free(data);
		free(file);
		file = ofile;
		line = oline;
		mfprintf(out, "file \"%s\"\nline %u\n", file, oline + 1);
		return 1;
	}
	if (cmpid(ip, "macro")) {
		macro_t mac, *p;
		if (lp == NULL)
			flerrexit("macro definition need be preceded by a label");
		mac.argc = 0;
		mac.name = lp;
		while (1) {
			getword(&argp, &s);
			if (s == NULL && *argp != ')')
				flerrexit("syntax error in macro parameter list");
			if (s != NULL) {
				mac.argv[mac.argc] = s;
				++(mac.argc);
			}
			if (ctype(*argp) & (CT_NL | CT_NUL))
				break;
			if (*argp != ',')
				flerrexit("syntax error in macro parameter list");
			++argp;
			if (mac.argc == ARG_MAX)
				flerrexit("too many arguments for macro");
		}
		if (!skipsp(&argp) && !(ctype(*argp) & (CT_NL | CT_NUL)))
			flerrexit("syntax error on define directive");
		if (!*argp)
			flerrexit("end of file after macro");
		mac.val = argp + 1;
		if ((p = macrofind(mac.name)) != NULL)
			memcpy(p, &mac, sizeof(macro_t));
		else arr_add(&macros, &mac);
		macro = 1;
		return 1;
	}
	{
		macro_t *mac;
		if ((mac = macrofind(ip)) != NULL) {
			++maclvl;
			_preprocess(out, mac->val, maclvl);
			mfprintf(out, "line %i\n", line + 1);
			return 1;
		}
	}
	return 0;
}

int getprefix(char **src) {
	char *p = *src;
	int n = 0;
	skipsp(&p);
	while (ctype(*p) & (CT_PPC))
		++p, ++n;
	if (n) {
		*src = p;
		skipsp(src);
	}
	return n;
}

void _preprocess(ioh_t *out, char *in, int lvl) {
	char *lnstart, *lp, *ip, *argp;
	int wp, r, pps;

	/* strip comments */
	{
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

	/* this very similar to assemble() */
	while (*in) {
		lnstart = in;
		lp = NULL;
		argp = NULL;
		r = 0;
		pps = getprefix(&in);
		wp = getword(&in, &ip);
		if (wp & (WP_LABEL | WP_LOCAL)) { /* we have label and we're sure about it */
			if (!(wp & WP_TSPC) && !(ctype(*in) & (CT_NL | CT_NUL)))
				flerrexit("invalid character in local label"); /* can only happen with local label */
			lp = ip;
			wp = getword(&in, &ip);
		}
		if (ip == NULL) {
			if (pps && !(ctype(*in) & (CT_NL | CT_NUL)))
				flerrexit("invalid preprocessor directive");
			goto endln;
		}
		if (!(ctype(*in) & (CT_NL | CT_NUL))) {
			if (wp & WP_TSPC)
				argp = in;
			else if (pps) flerrexit("invalid preprocessor directive");
		}
		if ((r = ppfind(out, lp, ip, argp)))
			goto endln;
		if (lp == NULL && !(wp & WP_PSPC)) {
			lp = ip;
			wp = getword(&in, &ip);
			if (ip == NULL)
				goto endln;
			if (!(ctype(*in) & (CT_NL | CT_NUL))) {
				if (wp & WP_TSPC)
					argp = in;
				else goto endln;
			}
			r = ppfind(out, lp, ip, argp);
		}
		endln:
		while (!(ctype(*in) & (CT_NUL | CT_NL)))
			++in;
		skipnl(&in);
		if (!r && !ignore)
			ppsub(out, lnstart, 0);
		mfprint(out, "\n");
		if (maclvl < lvl)
			return;
		if (!lvl)
			++line;
	}
}

/* preprocess(in, ret)
 *
 * description
 *   strips comments from in (on the string itself replaces them with space)
 *   evaluates preprocessor directives and builds with them preprocessed data
 *   on a new string, which *ret will point to
 *
 * arguments
 *   char * in: zero terminated string to be preprocessed
 *   char ** ret: *ret is set to pointer with preprocessed data
 *
 * return value
 *   int
 *   length of *ret
 */
void preprocess(ioh_t *out, char *in) {
	file = infile;
	arr_new(&defines, sizeof(define_t));
	arr_new(&macros, sizeof(macro_t));
	line = 1;
	maclvl = level = ignore = 0;
	cmacro = NULL;
	_preprocess(out, in, 0);
	arr_free(&macros);
	arr_free(&defines);
}
