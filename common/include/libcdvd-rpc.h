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
 * Common definitions for the libcdvd RPC on the EE and IOP
 */

#ifndef __LIBCDVD_RPC_H__
#define __LIBCDVD_RPC_H__

#include <tamtypes.h>

/* S-command parameters */

struct cdvdScmdParam
{
    u16 cmdNum;
    u16 inBuffSize;
    u8 inBuff[16];
};

struct cdvdDecSetParam
{
    u8 arg1;
    u8 arg2;
    u8 shift;
    u8 pad;
};

struct cdvdReadWriteNvmParam
{
    u32 address;
    u16 value;
    u16 pad;
};

/* N-command parameters */

struct cdvdNcmdParam
{
    u16 cmdNum;
    u16 inBuffSize;
    u8 inBuff[16];
};

struct cdvdReadKeyParam
{
    u32 arg1;
    u32 arg2;
    u32 command;
};

#endif /* _LIBCDVD_RPC_H_ */
