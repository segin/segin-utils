/* matnum
 *
 * Big number library for integers of fixed size.
 *
 * Originally intended to have 64bit numbers with sign (and not losing
 * a bit), and also to make possible the use of 64bit integers without
 * C99 compiler or 64bit CPU. As well as to be able to convert those
 * numbers to various formats (referring to the storage of negative
 * numbers mostly) without knowledge of the host architecture.
 *
 * The code assumes 'unsigned short' is at least 16 bits, and
 * 'unsigned long' at least 32bits.
 *
 * Copyright (c) 2012 Mattis Michel.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "num.h"

#include <stdlib.h>

void num_set32(num_t *lval, unsigned long rval) {
	int i;
	lval->value[0] = rval & 0xFFFF;
	lval->value[1] = (rval >> 16) & 0xFFFF;
	for (i = 2; i < NUM_SIZE; ++i)
		lval->value[i] = 0;
	lval->flags = ((rval == 0) ? NUM_FLAGS_ZERO : 0);
}

unsigned long num_get32(num_t *rval, int opts) {
	unsigned long ret = 0;
	ret |= rval->value[0];
	ret |= rval->value[1] << 16;
	return ret;
}

int num_iseq(num_t *lval, num_t *rval) {
	int i;
	for (i = 0; i < NUM_SIZE; ++i)
		if (lval->value[i] != rval->value[i])
			return 0;
	return 1;
}

int num_isgt(num_t *lval, num_t *rval) {
	int i;
	for (i = NUM_SIZE - 1; i >= 0; --i) {
		if (lval->value[i] < rval->value[i])
			return 0;
		if (lval->value[i] > rval->value[i])
			return 1;
	}
	return 0;
}

int num_isgte(num_t *lval, num_t *rval) {
	int i;
	for (i = NUM_SIZE - 1; i >= 0; --i) {
		if (lval->value[i] < rval->value[i])
			return 0;
		if (lval->value[i] > rval->value[i])
			return 1;
	}
	return 1;
}

void num_add(num_t *lval, num_t *rval) {
	unsigned long res = 0;
	int i;
	lval->flags &= ~(NUM_FLAGS_ZERO|NUM_FLAGS_OVFL);
	if ((lval->flags ^ rval->flags) & NUM_FLAGS_SIGN) {
		lval->flags &= ~(rval->flags & NUM_FLAGS_ZERO);
		num_sub(lval, rval);
		return;
	}
	for (i = 0; i < NUM_SIZE; ++i) {
		res += (unsigned long) lval->value[i] + rval->value[i];
		lval->value[i] = res & 0xFFFF;
		res >>= 16;
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
	if (res > 0)
		lval->flags |= NUM_FLAGS_OVFL;
}

void num_sub(num_t *lval, num_t *rval) {
	unsigned long res = 0;
	int i;
	lval->flags &= ~NUM_FLAGS_ZERO;
	for (i = 0; i < NUM_SIZE; ++i) {
		
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
}

void num_mul(num_t *lval, num_t *rval) {
	
}

void num_div(num_t *lval, num_t *rval) {
	
}

void num_mod(num_t *lval, num_t *rval) {
	
}

void num_and(num_t *lval, num_t *rval) {
	int i;
	lval->flags &= ~NUM_FLAGS_ZERO;
	for (i = 0; i < NUM_SIZE; ++i) {
		lval->value[i] &= rval->value[i];
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
}

void num_ior(num_t *lval, num_t *rval) {
	int i;
	lval->flags &= ~NUM_FLAGS_ZERO;
	for (i = 0; i < NUM_SIZE; ++i) {
		lval->value[i] |= rval->value[i];
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
}

void num_eor(num_t *lval, num_t *rval) {
	int i;
	lval->flags &= ~NUM_FLAGS_ZERO;
	for (i = 0; i < NUM_SIZE; ++i) {
		lval->value[i] ^= rval->value[i];
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
}

void num_shl(num_t *lval, num_t *rval) {
	
}

void num_shr(num_t *lval, num_t *rval) {
	
}

void num_not(num_t *lval) {
	int i;
	lval->flags &= ~NUM_FLAGS_ZERO;
	for (i = 0; i < NUM_SIZE; ++i) {
		lval->value[i] = ~lval->value[i];
		if (lval->value[i] != 0) lval->flags |= NUM_FLAGS_ZERO;
	}
}

void num_neg(num_t *lval) {
	lval->flags ^= NUM_FLAGS_SIGN;
}
