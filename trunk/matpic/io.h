#ifndef __IO_H__
#define __IO_H__

#include <stdarg.h>

typedef struct ioh_t ioh_t;

struct ioh_t {
	int (*write)(ioh_t *, char *, int);
	int (*read)(ioh_t *, char *, int);
	void (*close)(ioh_t *);
	void *data;
	char buf[2048];
	int pos;
};

extern int dosnl;

extern int mfread(ioh_t *h, char *data, int len);
extern int mfwrite(ioh_t *h, char *data, int len);
extern int mfflush(ioh_t *h);
extern void mfclose(ioh_t *h);
extern int mfprint(ioh_t *h, char *data);
extern int mprintnum(ioh_t *h, unsigned int n, int b, int p);
extern int mfprintf(ioh_t *h, char *fmt, ...);
extern int mvafprintf(ioh_t *h, char *fmt, va_list l);
extern ioh_t *mfdopen(int fd);

#endif /* __IO_H__ */

