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
# LIBSD definitions and imports.
*/

#ifndef IOP_LIBSD_H
#define IOP_LIBSD_H

#include "types.h"
#include "irx.h"

#define libsd_IMPORTS_start DECLARE_IMPORT_TABLE(libsd, 1, 4)
#define libsd_IMPORTS_end END_IMPORT_TABLE

typedef struct {
	u16 func;
	u16 entry;
	u32  value;
} SdBatch;

typedef struct {
     int     core;
     int     mode;
     short   depth_L;
     short   depth_R;
     int     delay;
     int     feedback;
} SdEffectAttr;

typedef int (*SdIntrHandler)(int, void *);

int sceSdInit(int flag);
#define I_sceSdInit DECLARE_IMPORT(4, sceSdInit);
void* sceSdSetIRQCallback ( void SD_IRQ_CBProc(void *) );
#define I_sceSdSetIRQCallback DECLARE_IMPORT(22, sceSdSetIRQCallback);
void* sceSdSetTransCallback (u16 channel, void SD_TRANS_CBProc(void *) );
#define I_sceSdSetTransCallback DECLARE_IMPORT(21, sceSdSetTransCallback);

void sceSdSetParam(u16 entry, u16 value);
#define I_sceSdSetParam DECLARE_IMPORT(5, sceSdSetParam);
u16 sceSdGetParam(u16 entry);
#define I_sceSdGetParam DECLARE_IMPORT(6, sceSdGetParam);

void sceSdSetCoreAttr(u16 entry, u16 value );
#define I_sceSdSetCoreAttr DECLARE_IMPORT(11, sceSdSetCoreAttr);
u16 sceSdGetCoreAttr(u16 entry );
#define I_sceSdGetCoreAttr DECLARE_IMPORT(12, sceSdGetCoreAttr);
int sceSdClearEffectWorkArea (int core, int channel, int effect_mode );
#define I_sceSdClearEffectWorkArea DECLARE_IMPORT(25, sceSdClearEffectWorkArea);

void sceSdSetAddr(u16 entry, u32 value );
#define I_sceSdSetAddr DECLARE_IMPORT(9, sceSdSetAddr);
u32 sceSdGetAddr(u16 entry );
#define I_sceSdGetAddr DECLARE_IMPORT(10, sceSdGetAddr);

void sceSdSetSwitch(u16 entry, u32 value );
#define I_sceSdSetSwitch DECLARE_IMPORT(7, sceSdSetSwitch);
u32 sceSdGetSwitch(u16 entry );
#define I_sceSdGetSwitch DECLARE_IMPORT(8, sceSdGetSwitch);

u16 sceSdNote2Pitch (u16 center_note, u16 center_fine, u16 note, short fine);
#define I_sceSdNote2Pitch DECLARE_IMPORT(13, sceSdNote2Pitch);
u16 sceSdPitch2Note (u16 center_note, u16 center_fine, u16 pitch);
#define I_sceSdPitch2Note DECLARE_IMPORT(14, sceSdPitch2Note);

int sceSdSetEffectAttr (int core, SdEffectAttr *attr );
#define I_sceSdSetEffectAttr DECLARE_IMPORT(23, sceSdSetEffectAttr);
void sceSdGetEffectAttr (int core, SdEffectAttr *attr );
#define I_sceSdGetEffectAttr DECLARE_IMPORT(24, sceSdGetEffectAttr);

int sceSdProcBatch(SdBatch* batch, u32 returns[], u32 num  );
#define I_sceSdProcBatch DECLARE_IMPORT(15, sceSdProcBatch);
int sceSdProcBatchEx(SdBatch* batch, u32 returns[], u32 num, u32 voice  );
#define I_sceSdProcBatchEx DECLARE_IMPORT(16, sceSdProcBatchEx);

int sceSdVoiceTrans(short channel, u16 mode, u8 *m_addr, u8 *s_addr, u32 size );
#define I_sceSdVoiceTrans DECLARE_IMPORT(17, sceSdVoiceTrans);
int sceSdBlockTrans(short channel, u16 mode, u8 *m_addr, u32 size, ... );
#define I_sceSdBlockTrans DECLARE_IMPORT(18, sceSdBlockTrans);
u32 sceSdVoiceTransStatus (short channel, short flag);
#define I_sceSdVoiceTransStatus DECLARE_IMPORT(19, sceSdVoiceTransStatus);
u32 sceSdBlockTransStatus (short channel, short flag);
#define I_sceSdBlockTransStatus DECLARE_IMPORT(20, sceSdBlockTransStatus);

SdIntrHandler sceSdSetTransIntrHandler(int channel, SdIntrHandler func, void *arg);
#define I_sceSdSetTransIntrHandler DECLARE_IMPORT(26, sceSdSetTransIntrHandler);
SdIntrHandler sceSdSetSpu2IntrHandler(SdIntrHandler func, void *arg);
#define I_sceSdSetSpu2IntrHandler DECLARE_IMPORT(27, sceSdSetSpu2IntrHandler);

#endif
