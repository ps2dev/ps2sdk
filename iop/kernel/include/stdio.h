/*
 * stdio.h - Standard C Library I/O.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This file is based off of the work [RO]man, Herben, and any others involved
 * in the "modules" project at http://ps2dev.pgamers.com/.  It is also based
 * off of the work of the many contributors to the ps2lib project at
 * http://ps2dev.livemedia.com.au/.
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef IOP_STDIO_H
#define IOP_STDIO_H

#include "irx.h"

#define stdio_IMPORTS_start DECLARE_IMPORT_TABLE(stdio, 1, 2)
#define stdio_IMPORTS_end END_IMPORT_TABLE

int printf(const char *format, ...);
#define I_printf DECLARE_IMPORT(4, printf)

int puts(const char *s);
#define I_puts DECLARE_IMPORT(7, puts)

#endif /* IOP_STDIO_H */
