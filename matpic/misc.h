#ifndef __MISC_H__
#define __MISC_H__

void cleanup(void);
void errexit(char *msg);
void flerrexit(char *file, int line, char *msg);
void flwarn(char *file, int line, char *msg);
void fawarn(char *file, int addr, char *msg);
int getargs(char **src, int *args);

#endif /* __MISC_H__ */

