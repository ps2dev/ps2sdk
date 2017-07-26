/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Standard C Library subset.
 */

#ifndef __SYSCLIB_H__
#define __SYSCLIB_H__

#include <types.h>
#include <irx.h>
#include <stdarg.h>
#include <setjmp.h>

typedef void (*print_callback_t)(char **string, int c);

/* These functions are already defined in setjmp.h */

/* These functions are non-standardized (char instead of int) */
char _toupper(char c);
char _tolower(char c);

#ifndef toupper
#define toupper(c) _toupper(c)
#endif

#ifndef tolower
#define tolower(c) _tolower(c)
#endif

unsigned char look_ctype_table(char character);
void *get_ctype_table();
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *p, const void *q, size_t size);
void * memcpy(void *dest, const void *src, size_t size);
void *memmove(void *dest, const void *src, size_t size);
void * memset(void *ptr, int c, size_t size);
int bcmp(const void *, const void *, size_t);
void bcopy(const void *, void *, size_t);
void bzero(void *, size_t);
int prnt(print_callback_t, void * opaque, const char * format, va_list ap);
int sprintf(char *str, const char *format, ...);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
int strcmp(const char *p, const char *q);
char *strcpy(char *dest, const char *src);
size_t strcspn(const char *s, const char *reject);
char *index(const char *s, int c);
char *rindex(const char *s, int c);
size_t strlen(const char *s);
char *strncat(char *dest, const char *src, size_t size);
int strncmp(const char *p, const char *q, size_t size);
char *strncpy(char *dest, const char *src, size_t size);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
size_t strspn(const char *s, const char *accept);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *s, const char *delim);
long strtol(const char *s, char **endptr, int base);
char *atob(char *s, int *i);
unsigned long strtoul(const char *s, char **endptr, int base);
void *wmemcopy(u32 *dest, const u32 *src, size_t size);
void *wmemset(u32 *dest, u32 c, size_t size);
int vsprintf(char *, const char *, va_list);
char *strtok_r(char *s, const char *delim, char **lasts);

#define sysclib_IMPORTS_start DECLARE_IMPORT_TABLE(sysclib, 1, 1)
#define sysclib_IMPORTS_end END_IMPORT_TABLE

#define I_setjmp DECLARE_IMPORT(4, setjmp)
#define I_longjmp DECLARE_IMPORT(5, longjmp)
#define I_toupper DECLARE_IMPORT(6, _toupper)
#define I_tolower DECLARE_IMPORT(7, _tolower)
#define I_look_ctype_table DECLARE_IMPORT(8, look_ctype_table)
#define I_get_ctype_table DECLARE_IMPORT(9, get_ctype_table)
#define I_memchr DECLARE_IMPORT(10, memchr)
#define I_memcmp DECLARE_IMPORT(11, memcmp)
#define I_memcpy DECLARE_IMPORT(12, memcpy)
#define I_memmove DECLARE_IMPORT(13, memmove)
#define I_memset DECLARE_IMPORT(14, memset)
#define I_bcmp DECLARE_IMPORT(15, bcmp);
#define I_bcopy DECLARE_IMPORT(16, bcopy);
#define I_bzero DECLARE_IMPORT(17, bzero);
#define I_prnt DECLARE_IMPORT(18, prnt)
#define I_sprintf DECLARE_IMPORT(19, sprintf)
#define I_strcat DECLARE_IMPORT(20, strcat)
#define I_strchr DECLARE_IMPORT(21, strchr)
#define I_strcmp DECLARE_IMPORT(22, strcmp)
#define I_strcpy DECLARE_IMPORT(23, strcpy)
#define I_strcspn DECLARE_IMPORT(24, strcspn)
#define I_index DECLARE_IMPORT(25, index)
#define I_rindex DECLARE_IMPORT(26, rindex)
#define I_strlen DECLARE_IMPORT(27, strlen)
#define I_strncat DECLARE_IMPORT(28, strncat)
#define I_strncmp DECLARE_IMPORT(29, strncmp)
#define I_strncpy DECLARE_IMPORT(30, strncpy)
#define I_strpbrk DECLARE_IMPORT(31, strpbrk)
#define I_strrchr DECLARE_IMPORT(32, strrchr)
#define I_strspn DECLARE_IMPORT(33, strspn)
#define I_strstr DECLARE_IMPORT(34, strstr)
#define I_strtok DECLARE_IMPORT(35, strtok)
#define I_strtol DECLARE_IMPORT(36, strtol)
#define I_atob DECLARE_IMPORT(37, atob)
#define I_strtoul DECLARE_IMPORT(38, strtoul)
#define I_wmemcopy DECLARE_IMPORT(40, wmemcopy)
#define I_wmemset DECLARE_IMPORT(41, wmemset)
#define I_vsprintf DECLARE_IMPORT(42, vsprintf)
#define I_strtok_r DECLARE_IMPORT(43, strtok_r)

#endif /* __SYSCLIB_H__ */
