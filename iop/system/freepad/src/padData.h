/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef __FREEPAD_PADDATA_H__
#define __FREEPAD_PADDATA_H__


void pdReset(); 
void pdTransfer();

u32 pdSetCtrl1(u32 port, u32 slot, u32 ctrl);
u32 pdSetCtrl2(u32 port, u32 slot, u32 ctrl);

u32 pdSetInBuffer(u32 port, u32 slot, u32 size, u8 *buf);
u32 pdGetOutBuffer(u32 port, u32 slot, u32 size, u8 *buf);

u32 pdSetInSize(u32 port, u32 slot, u32 size);
u32 pdSetOutSize(u32 port, u32 slot, u32 size);

u32 pdGetInSize(u8 id);
u32 pdGetOutSize(u8 id);

u32 pdSetRegData(u32 port, u32 slot, u32 reg_data);
u32 pdGetRegData(u32 id);

u32 pdIsActive(u32 port, u32 slot);
u32 pdSetActive(u32 port, u32 slot, u32 active);

u32 pdGetStat70bit(u32 port, u32 slot);
u32 pdSetStat70bit(u32 port, u32 slot, u32 val);

s32 pdGetError(u32 port, u32 slot);

u32 pdCheckConnection(u32 port, u32 slot);




#endif

