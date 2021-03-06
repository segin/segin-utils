/* FreeBASIC FBSTRING implementation in C++ 
 * Copyright (c) 2008-2011, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string>

/* from yetifoot's tinybasic */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct FBSTRING {
public:
	FBSTRING();
	FBSTRING(const char *);
	FBSTRING(const char *, const size_t);
	FBSTRING(const char *, const size_t, const size_t);
	FBSTRING(const std::string &);
	FBSTRING(const std::string &, const size_t);
	FBSTRING(const std::string &, const size_t, const size_t);
	FBSTRING(const FBSTRING &);
	FBSTRING(const FBSTRING &, const size_t);
	FBSTRING(const FBSTRING &, const size_t, const size_t);
	~FBSTRING();
	int operator=(const int);
	int operator=(const short);
	int operator=(const float);
	int operator=(const double);
	int operator=(const char);	
	int operator=(const char *);
	int operator=(const std::string &);
	int operator=(const FBSTRING &);
	int operator==(const FBSTRING &);
	int operator!=(const FBSTRING &);
	int operator+=(const FBSTRING &);
	static int asc(const FBSTRING &);
	static int val(const FBSTRING &);
private:
	void *data;
	int len;
	int size;
	void init_FBSTRING(void *, int); /* common constructor bits */
	void fini_FBSTRING(); /* common destructor bits */
};

typedef int integer;
typedef char zstring;

void print(const FBSTRING&, integer nl);

FBSTRING& concat(const FBSTRING&, const FBSTRING&);
FBSTRING& lcase(const FBSTRING&);
FBSTRING& int_to_str(integer);
FBSTRING& chr(integer);
integer asc(const FBSTRING&);
integer val(const FBSTRING&);

#define exit_ exit
#define EOF_ EOF
#define callocate(n) calloc(n, 1 )

/* perform allocations in 4k chunks; as FreeBASIC only runs on x86, this
 * allows for (hopefully) efficent memory allocation since we're allocating
 * an entire page at once.
 */

void FBSTRING::init_FBSTRING(void *str, int length);
{
	int m;
	m = length / 4096;
	if((length % 4096) > 0) 
		m++;
	size = m * 4096;	
	data = ::malloc(size);
	len = length;
	::memcpy(data, str, length);
	(char *) data[length] = 0;
}	

void FBSTRING::fini_FBSTRING()
{
	if(data) free(data);
	data = 0;
	len = 0;
	size = 0;
}

/* initialize an empty string */
FBSTRING::FBSTRING()
{
	data = 0;
	len = 0;
	size = 0;
}

/* initialize a string from a character array */
FBSTRING::FBSTRING(const char *s)
{
	data = 0;
	len = 0;
	size = 0;
	if((int) s == 0) return;
	int s_len = strlen(s);
	init_FBSTRING((void *) s, s_len);
}

/* initialize a string from a character array, with a given starting offset. */
FBSTRING::FBSTRING(const char *s, const size_t l)
{
	data = 0;
	len = 0;
	size = 0;
	if((int) s == 0) return;
	int s_len = strlen(s);
	if (s_len < (int) l) return;
	int m = s_len - (int) l;
	init_FBSTRING((void *) s + (void *) l, m);
}

/* initialize a string from a character array, with a given starting offset
 * plus max length */
FBSTRING::FBSTRING(const char *s, const size_t l, const size_t x)
{
	data = 0;
	len = 0;
	size = 0;
	if(((int) s == 0) || ((int) x == 0)) return;
	int s_len = strlen(s);
	if (s_len < (int) l) return;
	int m = s_len - (int) l;
	init_FBSTRING((void *) ((int) s + (int) l, (int) (m < x ? m : x));
}

FBSTRING::FBSTRING(const FBSTRING& s)
{
	data = 0;
	len = 0;
	size = 0;
	if(s.data == 0) return;
	init_FBSTRING(s, s_len);
}

FBSTRING::~FBSTRING()
{
	fini_FBSTRING();
}

int FBSTRING::operator=(const char *s)
{
	if(data) free(data);
	data = 0;
	len = 0;
	size = 0;
	if(s == 0) return 0;
	int s_len = strlen(s);
	data = malloc(s_len);
	len = s_len;
	size = s_len;
	::memcpy(data, s, s_len);
	return 0;
}

int FBSTRING::operator=(const FBSTRING& s)
{
	data = 0;
	len = 0;
	size = 0;
	if(s.data == 0) return 0;
	data = malloc(s.len);
	len = s.len;
	size = s.len;
	memcpy(data, s.data, s.len);
	return 0;
}

int FBSTRING::operator==(const FBSTRING& s)
{
	if(len != s.len) return 0;
	if((data == 0) && (s.data == 0)) return 1;
	if((data == 0) || (s.data == 0)) return 0;
	if(memcmp(data, s.data, len) != 0) return 0;
	return 1;
}

int FBSTRING::operator!=(const FBSTRING& s)
{
	if(len != s.len) return 1;
	if((data == 0) && (s.data == 0)) return 0;
	if((data == 0) || (s.data == 0)) return 1;
	if(memcmp(data, s.data, len) != 0) return 1;
	return 0;
}

int FBSTRING::operator+=(const FBSTRING& s)
{
	FBSTRING result;
	result.data = data;
	result.len = len;
	result.size = size;
	result = concat(result, s);
	free(data);
	data = result.data;
	len = result.len;
	size = result.size;
	return 0;
}

/*

void print(const FBSTRING& s, integer nl)
{
	int pos;
	for(pos = 0; pos < s.len; pos++) {
		fputc(((char *)s.data)[pos], stdout);
	}
	if(nl) fputc(10, stdout);
}

FBSTRING& concat(const FBSTRING& s1, const FBSTRING& s2)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	result->data = malloc(s1.len + s2.len);
	result->len = s1.len + s2.len;
	result->size = s1.len + s2.len;
	memcpy(result->data, s1.data, s1.len);
	memcpy(&(((char *)result->data)[s1.len]), s2.data, s2.len);
	return *result;
}

FBSTRING& lcase(const FBSTRING& s)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	*result = s;
	int pos;
	for(pos = 0; pos < s.len; pos++) {
		((char *)result->data)[pos] = tolower(((char *)s.data)[pos]);
	}
	return *result;
}

FBSTRING& str_temp(const FBSTRING& s)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	*result = s;
	return *result;
}

FBSTRING& int_to_str(integer i)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	char *tmp = (char *)calloc(32, 1);
	sprintf(tmp, "%i", i);
	int tmp_len = strlen(tmp);
	result->data = tmp;
	result->len = tmp_len;
	result->size = 32;
	return *result;
}

FBSTRING& chr(integer i)
{
	FBSTRING *result = (FBSTRING *)calloc(sizeof(FBSTRING), 1);
	result->data = malloc(1);
	result->len = 1;
	result->size = 1;
	((char *)result->data)[0] = i;
	return *result;
}
*/
static int FBSTRING::val(const FBSTRING& s)
{
	int n;
	char *tmp = (char *)calloc(s.len + 1, 1);
	memcpy(tmp, s.data, s.len);
	n = atoi(tmp);
	free(tmp);
	return n;
}

static int FBSTRING::asc(const FBSTRING& s)
{
	if(s.data == 0) return -1;
	return ((char *)s.data)[0];
}

