/*
 * freemtap - IOP multitap driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef _FREEMTAP_H_
#define _FREEMTAP_H_

#define MODNAME "freemtap"
#define M_PRINTF(format, args...)	printf(MODNAME ": " format, ## args)

// rpcservers.c
s32 InitRpcServers();

// freemtap.c
s32 mtapPortOpen(u32 port);
s32 mtapPortClose(u32 port);
s32 mtapGetConnection(u32 port);
s32 mtapGetSlotNumber(u32 port);
s32 mtapChangeSlot(u32 port, u32 slot);

#endif


