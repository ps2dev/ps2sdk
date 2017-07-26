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
 * Common definitions for SIOR on the EE and IOP
 */

#ifndef __SIOR_COMMON_H__
#define __SIOR_COMMON_H__

#include <tamtypes.h>

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

struct siorInitArgs {
    u32 baudrate;
    u8 lcr_ueps;
    u8 lcr_upen;
    u8 lcr_usbl;
    u8 lcr_umode;
};

struct siorReadArgs {
	char *buf;
	s32 len;
};

struct siorWriteArgs {
	const char *buf;
	s32 len;
};

union siorCommsData {
	u8 data[64];
	int result;
	struct siorInitArgs init;
	struct siorReadArgs read;
	struct siorWriteArgs write;
	const char *cstr;
	char *str;
	int c;
};

#endif /* __SIOR_COMMON_H__ */
