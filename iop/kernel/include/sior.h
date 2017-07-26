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
 * IOP-side SIO remote service.
 */

#ifndef __SIOR_H__
#define __SIOR_H__

#include <stdarg.h>
#include <irx.h>

#include <sior-common.h>

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode);
int sio_putc(int c);
int sio_getc(void);
int sio_getc_block(void);

size_t sio_write(const char *buf, size_t size);
size_t sio_read(char *buf, size_t size);

int sio_puts(const char *str);
int sio_putsn(const char *str); // no newline for this one
char *sio_gets(char *str);

void sio_flush(void);

int sio_printf(const char *format, ...);
int sio_vprintf(const char *format, va_list);

#define sior_IMPORTS_start DECLARE_IMPORT_TABLE(sior, 1, 1)
#define sior_IMPORTS_end END_IMPORT_TABLE

#define I_sio_init DECLARE_IMPORT(4, sio_init)
#define I_sio_putc DECLARE_IMPORT(5, sio_putc)
#define I_sio_getc DECLARE_IMPORT(6, sio_getc)
#define I_sio_getc_block DECLARE_IMPORT(7, sio_getc_block)
#define I_sio_write DECLARE_IMPORT(8, sio_write)
#define I_sio_read DECLARE_IMPORT(9, sio_read)
#define I_sio_puts DECLARE_IMPORT(10, sio_puts)
#define I_sio_putsn DECLARE_IMPORT(11, sio_putsn)
#define I_sio_gets DECLARE_IMPORT(12, sio_gets)
#define I_sio_flush DECLARE_IMPORT(13, sio_flush)
#define I_sio_printf DECLARE_IMPORT(14, sio_printf)
#define I_sio_vprintf DECLARE_IMPORT(15, sio_vprintf)

#endif /* __SIOR_H__ */
