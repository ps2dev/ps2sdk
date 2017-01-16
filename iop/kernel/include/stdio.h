/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Standard C Library I/O.
*/

#ifndef IOP_STDIO_H
#define IOP_STDIO_H

#include "irx.h"

#define stdio_IMPORTS_start DECLARE_IMPORT_TABLE(stdio, 1, 2)
#define stdio_IMPORTS_end END_IMPORT_TABLE

int printf(const char *format, ...);
#define I_printf DECLARE_IMPORT(4, printf)

int putchar(int ch);
#define I_putchar DECLARE_IMPORT(6, putchar)

int puts(const char *s);
#define I_puts DECLARE_IMPORT(7, puts)

char *gets(char *s);
#define I_gets DECLARE_IMPORT(8, gets)

#endif /* IOP_STDIO_H */
