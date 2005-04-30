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
# IOP-side SIO remote service.
*/

#ifndef __SIOR_H__
#define __SIOR_H__

#include "irx.h"
#include <stdarg.h>

#define	SIOR_IRX              0xC001510

enum {
    SIOR_INIT = 1,
    SIOR_PUTC,
    SIOR_GETC,
    SIOR_GETCBLOCK,
    SIOR_WRITE,
    SIOR_READ,
    SIOR_PUTS,
    SIOR_PUTSN,
    SIOR_GETS,
    SIOR_FLUSH
};

#define sior_IMPORTS_start DECLARE_IMPORT_TABLE(sior, 1, 1)
#define sior_IMPORTS_end END_IMPORT_TABLE

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode);
#define I_sio_init DECLARE_IMPORT(4, sio_init)
int sio_putc(int c);
#define I_sio_putc DECLARE_IMPORT(5, sio_putc)
int sio_getc(void);
#define I_sio_getc DECLARE_IMPORT(6, sio_getc)
int sio_getc_block(void);
#define I_sio_getc_block DECLARE_IMPORT(7, sio_getc_block)

size_t sio_write(void *buf, size_t size);
#define I_sio_write DECLARE_IMPORT(8, sio_write)
size_t sio_read(void *buf, size_t size);
#define I_sio_read DECLARE_IMPORT(9, sio_read)

int sio_puts(const char *str);
#define I_sio_puts DECLARE_IMPORT(10, sio_puts)
int sio_putsn(const char *str); // no newline for this one
#define I_sio_putsn DECLARE_IMPORT(11, sio_putsn)
char *sio_gets(char *str);
#define I_sio_gets DECLARE_IMPORT(12, sio_gets)

void sio_flush(void);
#define I_sio_flush DECLARE_IMPORT(13, sio_flush)

int sio_printf(const char *format, ...);
#define I_sio_printf DECLARE_IMPORT(14, sio_printf)
int sio_vprintf(const char *format, va_list);
#define I_sio_vprintf DECLARE_IMPORT(15, sio_vprintf)

#endif
