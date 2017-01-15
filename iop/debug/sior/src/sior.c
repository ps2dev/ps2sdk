/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# IOP exception handling.
*/

#include <stdarg.h>

#include "types.h"
#include "defs.h"
#include "irx.h"

#include "sior.h"
#include "xprintf.h"

#include "irx_imports.h"
IRX_ID("sioremote_driver", 1, 1);

extern struct irx_export_table _exp_sior;

static struct t_SifRpcClientData cd0;
static union siorCommsData buffer;
static char tbuf[2048];

void sio_init(u32 baudrate, u8 lcr_ueps, u8 lcr_upen, u8 lcr_usbl, u8 lcr_umode) {
    struct siorInitArgs * i = &buffer.init;
    i->baudrate = baudrate;
    i->lcr_ueps = lcr_ueps;
    i->lcr_upen = lcr_upen;
    i->lcr_usbl = lcr_usbl;
    i->lcr_umode = lcr_umode;
    SifCallRpc(&cd0, SIOR_INIT, 0, &buffer, sizeof(struct siorInitArgs), NULL, 0, NULL, NULL);
}

int sio_putc(int c) {
    buffer.result = c;
    SifCallRpc(&cd0, SIOR_PUTC, 0, &buffer, sizeof(int), &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

int sio_getc(void) {
    SifCallRpc(&cd0, SIOR_GETC, 0, 0, 0, &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

int sio_getc_block(void) {
    SifCallRpc(&cd0, SIOR_GETCBLOCK, 0, 0, 0, &buffer, sizeof(int), NULL, NULL);
    return buffer.result;
}

size_t sio_write(const char *buf, size_t size) {
    buffer.write.buf = buf;
    buffer.write.len = size;
    SifCallRpc(&cd0, SIOR_WRITE, 0, &buffer, 8, &buffer, 4, NULL, NULL);
    return buffer.result;
}

size_t sio_read(char *buf, size_t size) {
    buffer.read.buf = buf;
    buffer.read.len = size;
    SifCallRpc(&cd0, SIOR_READ, 0, &buffer, 8, &buffer, 4, NULL, NULL);
    return buffer.result;
}

int sio_puts(const char * str) {
    buffer.cstr = str;
    SifCallRpc(&cd0, SIOR_PUTS, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.result;
}

int sio_putsn(const char * str) {
    buffer.cstr = str;
    SifCallRpc(&cd0, SIOR_PUTSN, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.result;
}

char *sio_gets(char * str) {
    buffer.str = str;
    SifCallRpc(&cd0, SIOR_GETS, 0, &buffer, 4, &buffer, 4, NULL, NULL);
    return buffer.str;
}

void sio_flush(void) {
    SifCallRpc(&cd0, SIOR_FLUSH, 0, NULL, 0, NULL, 0, NULL, NULL);
}

int sio_vprintf(const char * str, va_list args) {
    int res;

    res = vsnprintf(tbuf, sizeof(tbuf), str, args);
    printf("%s", tbuf);
    sio_putsn(tbuf);

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
    int retries;

    memset(&cd0, 0, sizeof(cd0));

    for (retries = 0; retries < 15; retries++) {
	printf("Binding RPC.\n");
	if (SifBindRpc(&cd0, SIOR_IRX, 0) < 0) {
	    printf("Failed!\n");
	    return MODULE_NO_RESIDENT_END;
	}
	if (cd0.server != 0) {
	    printf("Registering library.\n");
	    if (RegisterLibraryEntries(&_exp_sior)) {
		printf("Couldn't register.\n");
		return MODULE_NO_RESIDENT_END;
	    }
	    return MODULE_RESIDENT_END;
	}
	printf("Huh, not available, retrying... ?\n");
	DelayThread(1000);
    }

    printf("Giving up.\n");

    return MODULE_NO_RESIDENT_END;
}
