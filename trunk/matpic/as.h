#ifndef __AS_H__
#define __AS_H__

#include "mem.h"

#define FN_MAX 512 /* filename max length */

extern int line;
extern char file[FN_MAX];
extern char *infile;
extern int address;

typedef struct {
	int type, line;
	union {
		struct ins {
			char *args;
			int oc;
			int atype;
		};
		struct org {
			int address;
		};
		struct data {
			int value;
		};
		struct file {
			char *file;
		};
	};
} ins_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
	IT_FIL, /* change of filename */
};

extern arr_t inss;

typedef struct {
	char *name;
	int address;
} label_t;

extern arr_t labels;

void errexit(char *msg);
unsigned int getval(char **src);
extern unsigned int numarg(char **src);
extern int getargs(char **src, int *args);
extern void assemble(char **code);
extern void cleanup(void);

#endif /* __AS_H__ */
