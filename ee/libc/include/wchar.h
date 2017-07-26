/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Wide-character type, constant and function declarations for LIBC.
 */

#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <tamtypes.h>
#include <errno.h>

#define __need_size_t
#define __need_wchar_t
#define __need_wint_t
#define __need_NULL
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>

#ifndef NULL
#define NULL	0
#endif

#ifndef WEOF
# define WEOF ((wint_t)-1)
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN 0
#endif

#ifndef WCHAR_MAX
#ifdef __WCHAR_MAX__
#define WCHAR_MAX __WCHAR_MAX__
#else
#define WCHAR_MAX 0x7fffffffu
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

wchar_t *wcsdup(const wchar_t* string);
int wcscasecmp(const wchar_t* wcs1, const wchar_t* wcs2);
int wcsncasecmp(const wchar_t* ws1, const wchar_t* ws2, size_t n);
wchar_t* wcstok(wchar_t* wcs, const wchar_t* delimiters);
wchar_t* wcsrchr(const wchar_t* ws, wint_t wc);
wchar_t* wcswcs(const wchar_t* ws1, const wchar_t* ws2);
wchar_t* wcsupr(wchar_t* wcs);
wchar_t* wcslwr(wchar_t* wcs);
wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);
int iswupper(wint_t wc);
int iswlower(wint_t wc);
int iswalpha(wint_t wc);
int iswdigit(wint_t wc);
int iswalnum(wint_t wc);
int iswcntrl(wint_t wc);
int iswgraph(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswxdigit(wint_t wc);
wchar_t* wcscpy(wchar_t* destination, const wchar_t* source);
wchar_t* wcsncpy(wchar_t* destination, const wchar_t* source, size_t num);
wchar_t* wcspbrk(const wchar_t* wcs1, const wchar_t* wcs2);
size_t wcsspn(const wchar_t* ws1, const wchar_t* ws2);
size_t wcscspn(const wchar_t* wcs1, const wchar_t* wcs2);
int wcscmp(const wchar_t *s1, const wchar_t *s2);
int wcsncmp(const wchar_t *s1, const wchar_t *s2, int len);
int wcslen(const wchar_t *string);
wchar_t *wcschr(const wchar_t *string, wint_t character);
wchar_t* wcscat(wchar_t* destination, const wchar_t* source);
wchar_t* wcsncat(wchar_t* destination, const wchar_t* source, size_t num);

int vxwprintf(void (*func)(wchar_t*,int,void*), void *arg, const wchar_t *format, va_list ap);
void __swout(wchar_t *txt, int amt, void *arg);
int vsnwprintf(wchar_t *buf, size_t n, const wchar_t *fmt, va_list ap);
int snwprintf(wchar_t *str, size_t sz, const wchar_t *format, ...);
int vswprintf(wchar_t *buf, const wchar_t *fmt, va_list ap);
int swprintf(wchar_t *str, size_t n, const wchar_t *format, ...);

double wcstod(const wchar_t *s, wchar_t **eptr);
long int wcstol(const wchar_t *nptr, wchar_t **endptr, int base);
unsigned long int wcstoul(const wchar_t *nptr, wchar_t **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif /* __WCHAR_H__ */
