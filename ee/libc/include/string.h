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

#include <stddef.h>
#include <stdarg.h>

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

char *	strcat(char *, const char *);
char *	strchr(const char *, int);
char *	strcpy(char *, const char *);
char *	strncat(char *, const char *, size_t);
char *	strncpy(char *, const char *, size_t);


// copies ascii string to sjis string
// 
// args:    dest sjis string buffer
//          source ascii string buffer
// returns: length of ascii string copied
int strcpy_sjis(short* sjis_buff, const char* ascii_buff);

// copies sjis string to ascii string
// 
// args:    dest ascii string buffer
//          source sjis string buffer
// returns: length of sjis string copied
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

/* Backward compatibility... */
#include <ctype.h>

#ifdef __cplusplus
}
#endif

#endif	// _STRING_H
