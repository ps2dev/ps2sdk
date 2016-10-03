/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# String function prototypes
*/

#ifndef _STRING_H
#define _STRING_H

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ASM String functions by Jeff Johnston of Cygnus Solutions */
void *	memchr(const void *, int, size_t);
void *	memcpy(void *, const void *, size_t);
void *	memmove(void *, const void *, size_t);
void *	memset(void *, int, size_t);

int	memcmp(const void *, const void *, size_t);

int	strcmp(const char *, const char *);
int	strncmp(const char *, const char *, size_t);

unsigned int strlen(const char *);

char *  strdup(const char *s);

char *	strcat(char *, const char *);
char *	strchr(const char *, int);
char *	strcpy(char *, const char *);
char *	strncat(char *, const char *, size_t);
char *	strncpy(char *, const char *, size_t);

char *  strpbrk(const char *s, const char *accept);
size_t  strspn(const char *s, const char *accept);
size_t  strcspn(const char *s, const char *reject);

/* Try to make gnu89, C89, C99, C11, gnu99 compatible inline. */
#ifndef INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#  define INLINE extern __inline__
# else
#  define INLINE inline
# endif
#endif
INLINE int strcoll(const char *s1, const char *s2) { return strcmp(s1, s2); }

char *  strerror(int);

INLINE size_t strxfrm(char *dest, const char *src, size_t n) { strncpy(dest, src, n); return n; }
#undef INLINE

char *	strtok(char *, const char *);
char *	strrchr(const char *, int);

char *	strstr(const char *, const char *);

char * strupr(char *);
char * strlwr(char *);

/* Backward compatibility... */
#include <ctype.h>

#ifdef __cplusplus
}
#endif

#endif	// _STRING_H
