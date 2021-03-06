/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Kazuho Oku
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kazutils.h"

static int _grow_strbuf(kstrbuf *sbuf, size_t additional)
{
	size_t newlen = sbuf->len + additional, newcapacity;
	char *newptr;

	// calculate new capacity
	for (newcapacity = sbuf->capacity == 0 ? 4096 : sbuf->capacity;
		newcapacity <= newlen;
		newcapacity *= 2)
		;
	// just return if no need to realloc
	if (sbuf->capacity == newcapacity)
		return 0;
	// allocate
	if ((newptr = realloc(sbuf->str, newcapacity)) == NULL)
		return -1;
	// update
	newptr[sbuf->len] = '\0';
	sbuf->str = newptr;
	sbuf->capacity = newcapacity;

	return 0;
}

void kstrbuf_clear(kstrbuf *sbuf)
{
	free(sbuf->str);
	memset(sbuf, 0, sizeof(*sbuf));
}

char *kstrbuf_append_str(kstrbuf *sbuf, const char *str)
{
	size_t slen = strlen(str);

	if (_grow_strbuf(sbuf, slen) != 0)
		return NULL;
	memcpy(sbuf->str + sbuf->len, str, slen + 1);

	return sbuf->str;
}

char *kstrbuf_append_char(kstrbuf *sbuf, int ch)
{
	if (_grow_strbuf(sbuf, 1) != 0)
		return NULL;
	sbuf->str[sbuf->len++] = ch;
	sbuf->str[sbuf->len] = '\0';

	return sbuf->str;
}

char *ksprintf(const char *fmt, ...)
{
	char smallbuf[256], *ret;
	va_list arg;
	int len;

	// determine the length (as well as fill-in the small buf)
	va_start(arg, fmt);
	len = vsnprintf(smallbuf, sizeof(smallbuf), fmt, arg);
	va_end(arg);
	if (len == -1)
		return NULL;

	// allocate
	if ((ret = malloc(len + 1)) == NULL)
		return NULL;

	// copy from small buf or reprint
	if (len < sizeof(smallbuf)) {
		memcpy(ret, smallbuf, len + 1);
	} else {
		va_start(arg, fmt);
		vsnprintf(ret, len + 1, fmt, arg);
		va_end(arg);
	}

	return ret;
}

char *kshellquote(const char *raw)
{
	char *quoted;
	int raw_idx, quoted_idx;

	// empty string => ''
	if (raw[0] == '\0')
		return strdup("''");

	if ((quoted = (char *)malloc(strlen(raw) * 2 + 1)) == NULL) {
		perror("");
		return NULL;
	}
	quoted_idx = 0;
	for (raw_idx = 0; raw[raw_idx] != '\0'; ++raw_idx) {
		if (isalnum(raw[raw_idx])) {
			// ok
		} else if (strchr("!%+,-./:@^", raw[raw_idx]) != NULL) {
			// ok
		} else {
			// needs backslash
			quoted[quoted_idx++] = '\\';
		}
		quoted[quoted_idx++] = raw[raw_idx];
	}
	quoted[quoted_idx++] = '\0';

	return quoted;
}

char *kdirname(const char *path)
{
	char *lastslash, *ret;
	size_t len;

	if ((lastslash = strrchr(path, '/')) == NULL)
		return strdup(".");
	len = lastslash - path;
	if ((ret = malloc(len + 1)) == NULL)
		return NULL;
	memcpy(ret, path, len);
	ret[len] = '\0';
	return ret;
}
