
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
