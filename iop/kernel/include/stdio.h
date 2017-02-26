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

#ifndef IOP_STDIO_H
#define IOP_STDIO_H

#include "irx.h"
#include <stdarg.h>

#define stdio_IMPORTS_start DECLARE_IMPORT_TABLE(stdio, 1, 2)
#define stdio_IMPORTS_end END_IMPORT_TABLE

int printf(const char *format, ...);
#define I_printf DECLARE_IMPORT(4, printf)

int getchar(void);
#define I_getchar DECLARE_IMPORT(5, getchar)

int putchar(int c);
#define I_putchar DECLARE_IMPORT(6, putchar)

int puts(const char *s);
#define I_puts DECLARE_IMPORT(7, puts)

char *gets(char *s);
#define I_gets DECLARE_IMPORT(8, gets)

int fdprintf(int fd, const char *format, ...);
#define I_fdprintf DECLARE_IMPORT(9, fdprintf)

int fdgetc(int fd);
#define I_fdgetc DECLARE_IMPORT(10, fdgetc)

char *fdgets(char *buf, int fd);
#define I_fdgets DECLARE_IMPORT(11, fdgets)

int fdputc(int c, int fd);
#define I_fdputc DECLARE_IMPORT(12, fdputc)

int fdputs(const char *s, int fd);
#define I_fdputs DECLARE_IMPORT(13, fdputs)

int vfdprintf(int fd, const char *format, va_list ap);
#define I_vfdprintf DECLARE_IMPORT(14, vfdprintf)

#endif /* IOP_STDIO_H */
