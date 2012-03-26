#include <stdlib.h> /* NULL */
#include "dis.h" /* dsym_t */
#include "misc.h" /* fawarn(), errexit(), infile, address */
#include "arch.h"
#include "str.h" /* hexnib[] */
#include "io.h"

arr_t dsym = { NULL, 0, 0, 0 }; /* these need to be 0 so cleanup() before disassemble won't fail */

void disassemble(ioh_t *out) {
	dsym_t *sym = (dsym_t *) dsym.data;
	int i = 0, c = dsym.count;
	oc_t *oc;
	unsigned char inop[6];

	while (c > 0) {
		oc = arch->ocs;
		address = sym->addr;
		mfprintf(out, "%8x (", address / arch->align);
		while (oc->name != NULL) {
			if (oc->len > c) /* this is to prevent disaster */
				goto docf;
			for (i = 0; ((sym + arch->ord[i % arch->align])->value & oc->imask[i]) == oc->oc[i] && i < oc->len; ++i);
			if (i == oc->len) {
				for (i = 0; i < oc->len; ++i)
					inop[i] = (sym + arch->ord[i % arch->align])->value;
				for (i = 0; i < oc->len; ++i)
					mfprintf(out, "%2x", inop[i]);
				mfprintf(out, "): %s ", oc->name);
				arch->adis(out, inop, oc->atype);
				sym += oc->len;
				c -= oc->len;
				break;
			}
			docf:
			++oc;
		}
		if (oc->name == NULL) {
			for (i = 0; i < arch->align; ++i)
				mfprintf(out, "%2x", (sym + arch->ord[i])->value);
			mfprint(out, "): data 0x");
			for (i = 0; i < arch->align; ++i)
				mfprintf(out, "%2x", (sym + arch->ord[i])->value);
			sym += arch->align;
			c -= arch->align;
		}
		mfprint(out, "\n");
	}
}
