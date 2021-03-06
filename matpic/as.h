#ifndef __AS_H__
#define __AS_H__

#include "arch.h"
#include "mem.h"
#include "lineno.h"

typedef struct label_t label_t;

struct label_t {
	char *name;
	int address;
	int local;
	int parent; /* tried to keep this as pointer first, big mistake (realloc) */
};

typedef union {
	struct head {
		unsigned int type;
		unsigned int line;
	} head;
	struct ins {
		unsigned int type, line;
		char *args;
		oc_t *oc;
	} ins;
	struct org {
		unsigned int type, line;
		unsigned long address;
		char *end;
	} org;
	struct data {
		unsigned int type, line;
		char *args;
		int size;
	} data;
	struct lbl {
		unsigned int type, line;
		unsigned int lbl;
	} lbl;
	struct ctx {
		unsigned int type, line;
		lineno_t *ctx;
	} ctx;
} ins_t;

typedef struct {
	unsigned long start;
	unsigned long end;
} map_t;

typedef struct {
	char *str;
} strarg_t;

enum itype {
	IT_END, /* end of data */
	IT_ORG, /* org directive */
	IT_PAD, /* padding (used by string stuff) */
	IT_DAT, /* data directive */
	IT_INS, /* an actual instruction */
	IT_CTX, /* change of filename/macro */
	IT_CTX_END,
	IT_LBL  /* last label encountered */
};

extern arr_t inss;
extern arr_t labels;
extern int llbl; /* last label */

extern char *lp, *ip, *argp, *nextln;
extern int prefix, run;

extern int parseln(char *in);
extern void assemble(char *code);

#endif /* __AS_H__ */
