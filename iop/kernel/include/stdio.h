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
# Standard C Library I/O.
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

#define stdio_IMPORTS \
	stdio_IMPORTS_start \
 \
 	I_printf \
	I_puts \
 \
	stdio_IMPORTS_end

#endif /* IOP_STDIO_H */
