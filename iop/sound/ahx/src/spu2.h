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
*/

#ifndef _SPU2_H_
#define _SPU2_H_

#include "freesd.h"

s32 SdInit(s32 flag);
void SdSetParam(u16 reg, u16 val);
void SdSetCoreAttr(u16 entry, u16 val);
IntrCallback SdSetTransCallback(int core, IntrCallback cb);
int SdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, u8 *startaddr);
u32 SdBlockTransStatus(s16 chan, s16 flag);

#endif
