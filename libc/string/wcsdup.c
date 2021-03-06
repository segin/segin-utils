/*	$OpenBSD: wcsdup.c,v 1.1 2011/07/04 04:37:34 nicm Exp $	*/
/*	$NetBSD: wcsdup.c,v 1.3 2008/05/26 13:17:48 haad Exp $	*/

/*
 * Copyright (C) 2006 Aleksey Cheusov
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted
 * without fee. Permission to modify the code and to distribute modified
 * code is also granted without any restrictions.
 */

#include <stdlib.h>
#include <wchar.h>

wchar_t *wcsdup(const wchar_t *str) {
	wchar_t *copy;
	size_t len;

	len = wcslen(str) + 1;
	copy = malloc(len * sizeof (wchar_t));

	if (!copy)
		return (NULL);

	return (wmemcpy(copy, str, len));
}
