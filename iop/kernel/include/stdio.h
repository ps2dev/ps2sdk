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
 * Standard C Library I/O.
 */

#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *format, ...);
int getchar(void);
int putchar(int c);
int puts(const char *s);
char *gets(char *s);
int fdprintf(int fd, const char *format, ...);
int fdgetc(int fd);
int fdputc(int c, int fd);
int fdputs(const char *s, int fd);
char *fdgets(char *buf, int fd);
int vfdprintf(int fd, const char *format, va_list ap);

#define stdio_IMPORTS_start DECLARE_IMPORT_TABLE(stdio, 1, 2)
#define stdio_IMPORTS_end END_IMPORT_TABLE

#define I_printf DECLARE_IMPORT(4, printf)
#define I_getchar DECLARE_IMPORT(5, getchar)
#define I_putchar DECLARE_IMPORT(6, putchar)
#define I_puts DECLARE_IMPORT(7, puts)
#define I_gets DECLARE_IMPORT(8, gets)
#define I_fdprintf DECLARE_IMPORT(9, fdprintf)
#define I_fdgetc DECLARE_IMPORT(10, fdgetc)
#define I_fdputc DECLARE_IMPORT(11, fdputc)
#define I_fdputs DECLARE_IMPORT(12, fdputs)
#define I_fdgets DECLARE_IMPORT(13, fdgets)
#define I_vfdprintf DECLARE_IMPORT(14, vfdprintf)

#ifdef __cplusplus
}
#endif

#endif /* __STDIO_H__ */
