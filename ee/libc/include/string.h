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
 * String function prototypes
 */

#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>

/* The maximum length of a string within PS2Lib.  */
#define PS2LIB_STR_MAX	4096

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

static inline int strcoll(const char *s1, const char *s2) { return strcmp(s1, s2); }
static inline size_t strxfrm(char *dest, const char *src, size_t n) { strncpy(dest, src, n); return n; }

char *  strerror(int);

/** copies ascii string to sjis string
 * @param sjis_buff dest sjis string buffer
 * @param ascii_buff source ascii string buffer
 * @return length of ascii string copied
 */
int strcpy_sjis(short* sjis_buff, const char* ascii_buff);

/** copies sjis string to ascii string
 *
 * @param ascii_buff dest ascii string buffer
 * @param sjis_buff source sjis string buffer
 * @return length of sjis string copied
 */
int strcpy_ascii(char* ascii_buff, const short* sjis_buff);

/* C String functions by Hiryu (A.Lee) */
#define stricmp strcasecmp
#define strnicmp strncasecmp

int	 strcasecmp(const char *, const char *);
int	 strncasecmp(const char *, const char *, size_t);

char *	strtok(char *, const char *);
char *	strrchr(const char *, int);

char *	strstr(const char *, const char *);

char * strupr(char *);
char * strlwr(char *);

static inline void bzero(void * p, size_t n) { memset(p, 0, n); }
static inline void bcopy(const void * s, void * d, size_t n) { memcpy(d, s, n); }
static inline int bcmp(const void * s1, const void * s2, size_t n) { return memcmp(s1, s2, n); }
static inline char * index(const char * s, int c) { return strchr(s, c); }
static inline char * rindex(const char * s, int c) { return strrchr(s, c); }

#ifdef __cplusplus
}
#endif

#endif /* __STRING_H__ */
