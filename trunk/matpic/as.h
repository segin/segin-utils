#ifndef __AS_H__
#define __AS_H__

#include "mem.h"

#define FN_MAX 512 /* filename max length */

extern char file[FN_MAX];
extern char *infile;
extern unsigned int address, line;

typedef struct {
	unsigned int type, line;
	union {
		struct ins {
			char *args;
			unsigned char oc[6];
			unsigned int len, atype;
		} ins;
		struct org {
			unsigned int address;
		} org;
		struct data {
			char *args;
			unsigned int value;
		} data;
		struct file {
			char *file;
		} file;
	} d;
} ins_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
	IT_FIL  /* change of filename */
};

extern arr_t inss;

typedef struct {
	char *name;
	int address;
} label_t;

extern arr_t labels;

void aerrexit(char *msg);
unsigned int getval(char **src);
extern unsigned int numarg(char **src);
extern int getargs(char **src, int *args);
extern void assemble(char *code);
extern void cleanup(void);

#endif /* __AS_H__ */

