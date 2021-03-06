/*
 * PkgNames
 * 
 * Extract package names from RPM database or suse zypper cache.
 * Because it's faster than the others.
 * Note: This is kindof a hack, and might not work under all conditions.
 * The kind of files I'm talking about are like:
 * /var/cache/zypp/raw/openSUSE-13.2-0/suse/setup/descr/packages.gz
 * /var/cache/zypp/raw/repo-update/...-primary.xml.gz
 * 
 * Compiling:
 *   CFLAGS="-lz -O3" make pkgnames
 * 
 * Running:
 *   pkgnames <filename>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <errno.h>
#include <string.h>

static const char *filename = "text.gz";

#define BLOCKSIZE 0x1000

int ispkg(char *str) {
	if (*(str++) != '=')
		return 0;
	if (*(str++) != 'P')
		return 0;
	if (*(str++) != 'k')
		return 0;
	if (*(str++) != 'g')
		return 0;
	if (*(str++) != ':')
		return 0;
	return 1;
}

int isxname(char *str) {
	if (*(str++) != '<')
		return 0;
	if (*(str++) != 'n')
		return 0;
	if (*(str++) != 'a')
		return 0;
	if (*(str++) != 'm')
		return 0;
	if (*(str++) != 'e')
		return 0;
	if (*(str++) != '>')
		return 0;
	return 1;
}

int main(int argc, char *argv[]) {
	char *buffer = NULL;
	int buffer_size = 0;
	int file_size = 0;
	gzFile file;
	
	if (argc < 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	filename = argv[1];
	
	file = gzopen(filename, "r");
	if (! file) {
		fprintf(stderr, "gzopen of '%s' failed: %s.\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while (1) {
		int err, bytes_read;
		
		buffer_size += BLOCKSIZE;
		buffer = realloc(buffer, buffer_size);
		if (buffer == NULL) {
			fprintf(stderr, "realloc failed: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		file_size += bytes_read = gzread(file, buffer + file_size, BLOCKSIZE);
		if (bytes_read < BLOCKSIZE) {
			if (gzeof(file)) {
				break;
			} else {
				const char *error_string;
				error_string = gzerror(file, &err);
				if (err) {
					fprintf(stderr, "Error: %s.\n", error_string);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	gzclose(file);
	buffer[file_size] = 0;
	
	{
		char *d = buffer;
		while (*d != 0) {
			while (*d == ' ' || *d == '\t') ++d;
			if (ispkg(d)) {
				char *p;
				d += 5; /* =Pkg: */
				while (*d == ' ' || *d == '\t') ++d;
				p = d;
				while (*d != 0 && *d != ' ' && *d != '\t' && *d != '\n')
					++d;
				if (*d == 0)
					break;
				*d = 0;
				printf("%s ", p);
				++d;
			}
			if (isxname(d)) {
				char *p;
				d += 6; /* <name> */
				p = d;
				while (*d != 0 && *d != '<')
					++d;
				if (*d == 0)
					break;
				*d = 0;
				printf("%s ", p);
				++d;	
			}
			while (*d != 0 && *d != '\n')
				++d;
			while (*d == '\n')
				++d;
		}
	}
	
	free(buffer);
	return 0;
}
