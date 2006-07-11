/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Standard C Library subset.
*/

#ifndef IOP_SYSCLIB_H
#define IOP_SYSCLIB_H

#include <stdarg.h>

#include "types.h"
#include "irx.h"
#include "setjmp.h"

typedef void (*print_callback_t)(void * opaque, u8 char_to_print);

#define sysclib_IMPORTS_start DECLARE_IMPORT_TABLE(sysclib, 1, 1)
#define sysclib_IMPORTS_end END_IMPORT_TABLE

/* These functions are already defined in setjmp.h */
#define I_setjmp DECLARE_IMPORT(4, setjmp)
#define I_longjmp DECLARE_IMPORT(5, longjmp)

char toupper(char c);
#define I_toupper DECLARE_IMPORT(6, toupper)
char tolower(char c);
#define I_tolower DECLARE_IMPORT(7, tolower)
unsigned char look_ctype_table(char character);
#define I_look_ctype_table DECLARE_IMPORT(8, look_ctype_table)
void *memchr(const void *s, int c, size_t n);
#define I_memchr DECLARE_IMPORT(10, memchr)
int memcmp(const void *p, const void *q, size_t size);
#define I_memcmp DECLARE_IMPORT(11, memcmp)
void * memcpy(void *dest, const void *src, size_t size);
#define I_memcpy DECLARE_IMPORT(12, memcpy)
void *memmove(void *dest, const void *src, size_t size);
#define I_memmove DECLARE_IMPORT(13, memmove)
void * memset(void *ptr, int c, size_t size);
#define I_memset DECLARE_IMPORT(14, memset)
int bcmp(const void *, const void *, size_t);
#define I_bcmp DECLARE_IMPORT(15, bcmp);
void bcopy(const void *, void *, size_t);
#define I_bcopy DECLARE_IMPORT(16, bcopy);
void bzero(void *, size_t);
#define I_bzero DECLARE_IMPORT(17, bzero);
int prnt(print_callback_t, void * opaque, const char * format, va_list ap);
#define I_prnt DECLARE_IMPORT(18, prnt)
int sprintf(char *str, const char *format, ...);
#define I_sprintf DECLARE_IMPORT(19, sprintf)
char *strcat(char *dest, const char *src);
#define I_strcat DECLARE_IMPORT(20, strcat)
char *strchr(const char *s, int c);
#define I_strchr DECLARE_IMPORT(21, strchr)
int strcmp(const char *p, const char *q);
#define I_strcmp DECLARE_IMPORT(22, strcmp)
char *strcpy(char *dest, const char *src);
#define I_strcpy DECLARE_IMPORT(23, strcpy)
size_t strcspn(const char *s, const char *reject);
#define I_strcspn DECLARE_IMPORT(24, strcspn)
char *index(const char *s, int c);
#define I_index DECLARE_IMPORT(25, index)
char *rindex(const char *s, int c);
#define I_rindex DECLARE_IMPORT(26, rindex)
size_t strlen(const char *s);
#define I_strlen DECLARE_IMPORT(27, strlen)
char *strncat(char *dest, const char *src, size_t size);
#define I_strncat DECLARE_IMPORT(28, strncat)
int strncmp(const char *p, const char *q, size_t size);
#define I_strncmp DECLARE_IMPORT(29, strncmp)
char *strncpy(char *dest, const char *src, size_t size);
#define I_strncpy DECLARE_IMPORT(30, strncpy)
char *strpbrk(const char *s, const char *accept);
#define I_strpbrk DECLARE_IMPORT(31, strpbrk)
char *strrchr(const char *s, int c);
#define I_strrchr DECLARE_IMPORT(32, strrchr)
size_t strspn(const char *s, const char *accept);
#define I_strspn DECLARE_IMPORT(33, strspn)
char *strstr(const char *haystack, const char *needle);
#define I_strstr DECLARE_IMPORT(34, strstr)
char *strtok(char *s, const char *delim);
#define I_strtok DECLARE_IMPORT(35, strtok)
long strtol(const char *s, char **endptr, int base);
#define I_strtol DECLARE_IMPORT(36, strtol)
unsigned long strtoul(const char *s, char **endptr, int base);
#define I_strtoul DECLARE_IMPORT(38, strtoul)

int vsprintf(char *, const char *, va_list);
#define I_vsprintf DECLARE_IMPORT(42, vsprintf)

#define sysclib_IMPORTS \
	sysclib_IMPORTS_start \
 \
 	I_setjmp \
	I_longjmp \
 \
	I_toupper \
	I_tolower \
	I_look_ctype_table \
	I_memchr \
	I_memcmp \
	I_memcpy \
	I_memmove \
	I_memset \
 \
	I_sprintf \
 \
	I_strcat \
	I_strchr \
	I_strcmp \
	I_strcpy \
	I_strcspn \
	I_index \
	I_rindex \
	I_strlen \
	I_strncat \
	I_strncmp \
	I_strncpy \
	I_strpbrk \
	I_strrchr \
	I_strspn \
	I_strstr \
	I_strtok \
	I_strtol \
	I_strtoul \
 \
	sysclib_IMPORTS_end

#endif /* IOP_SYSCLIB_H */
