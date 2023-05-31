/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __SIM_H__
#define __SIM_H__

extern void hdskSimGetFreeSectors(s32 device, struct hdskStat *stat, apa_device_t *deviceinfo);
extern void hdskSimMovePartition(struct hdskBitmap *dest, struct hdskBitmap *start);
extern struct hdskBitmap *hdskSimFindEmptyPartition(u32 size);
extern struct hdskBitmap *hdskSimFindLastUsedPartition(u32 size, u32 start, int mode);
extern struct hdskBitmap *hdskSimFindLastUsedBlock(u32 size, u32 start, int mode);
extern void hdskSimMovePartitionsBlock(struct hdskBitmap *dest, struct hdskBitmap *src);

#endif
