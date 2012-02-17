#include "host.h" /* exit(), EXIT_FAILURE, fprintf(), stderr */

void errexit(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

void flerrexit(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void flwarn(char *file, int line, char *msg) {
	fprintf(stderr, "%s: line %d: %s\n", file, line, msg);
	exit(EXIT_FAILURE);
}

void fawarn(char *file, int addr, char *msg) {
	fprintf(stderr, "%s: %X: %s\n", file, addr, msg);
}
