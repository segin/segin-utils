#ifndef __PPC_H__
#define __PPC_H__

#include "mem.h"
#include "io.h"

typedef struct {
	char *name, *val;
	int active, argc;
	char *argv[ARG_MAX];
} define_t;

typedef struct {
	char *name, *val;
	int active, argc;
	char *argv[ARG_MAX];
} macro_t;

typedef struct {
	char *name;
	char *nextln;
	int line;
} file_t;

typedef struct {
	char *start;
	int count, repno, line;
} rep_t;

typedef struct {
	macro_t *macro;
	char *argv[ARG_MAX], *nextln;
	int argc, line;
} amacro_t;

extern void preprocess(ioh_t *out, char *in);

#endif /* __PPC_H__ */
