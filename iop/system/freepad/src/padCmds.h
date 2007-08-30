/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef __FREEPAD_PADCMDS_H__
#define __FREEPAD_PADCMDS_H__

u32 PadIsSupported(padState_t *pstate);
u32 ReadData(padState_t *pstate);
u32 QueryModel(padState_t *pstate);
u32 SetMainMode(padState_t *pstate);
u32 QueryAct(u32 actuator, padState_t *pstate);
u32 QueryComb(u32 val, padState_t *pstate);
u32 QueryMode(u32 val, padState_t *pstate);
u32 EnterConfigMode(u8 val, padState_t *pstate);
u32 ExitConfigMode(padState_t *pstate); 
u32 SetActAlign(padState_t *pstate);
u32 QueryButtonMask(padState_t *pstate);
u32 VrefParam(u32 val, padState_t *pstate);
u32 SetButtonInfo(padState_t *pstate);

#endif

