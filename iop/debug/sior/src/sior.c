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
# IOP exception handling.
*/

#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "sior.h"

#include "irx_imports.h"
IRX_ID("sioremote_driver", 1, 1);

struct irx_export_table _exp_sior;

static struct t_SifRpcClientData cd0 __attribute__((aligned(64)));
static u32 buffer[32] __attribute__((aligned(64)));

struct init_arguments_t {
    u32 baudrate;
    u8 lcr_ueps;
    u8 lcr_upen;
    u8 lcr_usbl;
    u8 lcr_umode;
};

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode) {
    struct init_arguments_t * i = (struct init_arguments_t *) buffer;
    i->baudrate = baudrate;
    i->lcr_ueps = lcr_ueps;
    i->lcr_upen = lcr_upen;
    i->lcr_usbl = lcr_usbl;
    i->lcr_umode = lcr_umode;
    SifCallRpc(&cd0, SIOR_INIT, 0, buffer, sizeof(*i), NULL, 0, 0, 0);
}

int sio_putc(int c) {
    *((int *)buffer) = c;
    SifCallRpc(&cd0, SIOR_PUTC, 0, buffer, sizeof(int), buffer, sizeof(int), 0, 0);
    FlushDcache();
    return *((int *)buffer);
}

int sio_getc(void) {
    SifCallRpc(&cd0, SIOR_GETC, 0, 0, 0, buffer, sizeof(int), 0, 0);
    FlushDcache();
    return *((int *)buffer);
}

int sio_getc_block(void) {
    SifCallRpc(&cd0, SIOR_GETCBLOCK, 0, 0, 0, buffer, sizeof(int), 0, 0);
    FlushDcache();
    return *((int *)buffer);
}

size_t sio_write(void *buf, size_t size) {
    buffer[0] = (u32) buf;
    buffer[1] = size;
    SifCallRpc(&cd0, SIOR_WRITE, 0, buffer, 8, buffer, 4, 0, 0);
    FlushDcache();
    return buffer[0];
}

size_t sio_read(void *buf, size_t size) {
    buffer[0] = (u32) buf;
    buffer[1] = size;
    SifCallRpc(&cd0, SIOR_READ, 0, buffer, 8, buffer, 4, 0, 0);
    FlushDcache();
    return buffer[0];
}

int sio_puts(const char * str) {
    buffer[0] = (u32) str;
    SifCallRpc(&cd0, SIOR_PUTS, 0, buffer, 4, buffer, 4, 0, 0);
    FlushDcache();
    return *((int *)buffer);
}

int sio_putsn(const char * str) {
    buffer[0] = (u32) str;
    SifCallRpc(&cd0, SIOR_PUTSN, 0, buffer, 4, buffer, 4, 0, 0);
    FlushDcache();
    return *((int *)buffer);
}

char *sio_gets(char * str) {
    buffer[0] = (u32) str;
    SifCallRpc(&cd0, SIOR_GETS, 0, buffer, 4, buffer, 4, 0, 0);
    FlushDcache();
    return *((char *)buffer);
}

void sio_flush(void) {
    SifCallRpc(&cd0, SIOR_FLUSH, 0, 0, 0, 0, 0, 0, 0);
}

int sio_vprintf(const char * str, va_list args) {
    char * tbuf[2048];
    int res;
    
    res = vsprintf((char *)tbuf, str, args);
    sio_putsn((char *)tbuf);
    
    return res;
}

int sio_printf(const char * str, ...) {
    int res;
    va_list args;

    va_start(args, str);
    res = sio_vprintf(str, args);
    va_end(args);
    
    return res;
}

int _start(int argc, char **argv)
{
    int /* rv, */ retries;
    
    memset(&cd0, 0, sizeof(cd0));
    
    for (retries = 0; retries < 15; retries++) {
	printf("Binding RPC.\n");
	if (SifBindRpc(&cd0, SIOR_IRX, 0) < 0) {
	    printf("Failed!\n");
	    return 1;
	}
	if (cd0.server != 0) {
	    printf("Registering library.\n");
	    if (RegisterLibraryEntries(&_exp_sior)) {
		printf("Couldn't register.\n");
		return 1;
	    }
	    FlushDcache();
	    return 0;
	}
	printf("Huh, not available, retrying... ?\n");
	DelayThread(1000);
    }
    
    printf("Giving up.\n");
    
    return 1;
}

int shutdown()
{
    return 0;
}
