#ifndef __MEM_H__
#define __MEM_H__

/************************
 * generic memory stuff *
 ************************/

#include <stdlib.h> /* NULL */
#include <string.h> /* strlen() */
#include "config.h" /* BLOCK */

typedef struct {
	void *data;
	int size, count, space;
} arr_t;

extern void arr_new(arr_t *a, int size);
extern void arr_add(arr_t *a, void *data);
extern void arr_free(arr_t *a);

#define arr_item(a, type, i) ((type *) (a).data + (i))
#define arr_top(a, type) (a.count ? ((type *) (a).data + (a).count - 1) : NULL)
#define arr_pop(a, type) (a.count ? ((type *) (a).data + --((a).count)) : NULL)

typedef struct {
	char *data;
	unsigned long len, res;
} string_t;

#define vstr_add(s, str) (vstr_addl(s, str, strlen(str)))

extern void vstr_new(string_t *s);
extern void vstr_skip(string_t *s, unsigned long len);
extern void vstr_fill(string_t *s, unsigned long len, char what);
extern void vstr_addl(string_t *s, char *str, unsigned long len);
extern void vstr_free(string_t *s);

#endif /* __MEM_H__ */
