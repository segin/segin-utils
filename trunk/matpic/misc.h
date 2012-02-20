#ifndef __MISC_H__
#define __MISC_H__

extern char *infile;
extern unsigned int address, line;
extern int dosnl;

void cleanup(void);
void errexit(char *msg);
void flerrexit(char *file, int line, char *msg);
void flwarn(char *file, int line, char *msg);
void fawarn(char *file, int addr, char *msg);
unsigned int getval(char **src);
unsigned int numarg(char **src);
int getargs(char **src, int *args);
int getword(char **src, char **word);

/* getword return value is a bit map with following properties */
#define WP_LOCAL 1  /* identifier preceded by "." */
#define WP_LABEL 2  /* we're certain it's a label, it ends with ':' */
#define WP_PSPC  4  /* the word is preceded by space */
#define WP_TSPC  8  /* there are trailing spaces */

#endif /* __MISC_H__ */

