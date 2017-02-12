/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# LIBSD definitions and imports.
*/

#ifndef IOP_LIBSD_H
#define IOP_LIBSD_H

#include "types.h"
#include "irx.h"
#include <libsd-common.h>

#define libsd_IMPORTS_start DECLARE_IMPORT_TABLE(libsd, 1, 4)
#define libsd_IMPORTS_end END_IMPORT_TABLE

int sceSdQuit();
#define I_sceSdQuit DECLARE_IMPORT(2, sceSdQuit);
int sceSdInit(int flag);
#define I_sceSdInit DECLARE_IMPORT(4, sceSdInit);
SdIntrCallback sceSdSetIRQCallback(SdIntrCallback cb);
#define I_sceSdSetIRQCallback DECLARE_IMPORT(22, sceSdSetIRQCallback);
SdIntrCallback sceSdSetTransCallback(s32 core, SdIntrCallback cb);
#define I_sceSdSetTransCallback DECLARE_IMPORT(21, sceSdSetTransCallback);

void sceSdSetParam(u16 entry, u16 value);
#define I_sceSdSetParam DECLARE_IMPORT(5, sceSdSetParam);
u16 sceSdGetParam(u16 entry);
#define I_sceSdGetParam DECLARE_IMPORT(6, sceSdGetParam);

void sceSdSetCoreAttr(u16 entry, u16 value);
#define I_sceSdSetCoreAttr DECLARE_IMPORT(11, sceSdSetCoreAttr);
u16 sceSdGetCoreAttr(u16 entry);
#define I_sceSdGetCoreAttr DECLARE_IMPORT(12, sceSdGetCoreAttr);
int sceSdClearEffectWorkArea(int core, int channel, int effect_mode);
#define I_sceSdClearEffectWorkArea DECLARE_IMPORT(25, sceSdClearEffectWorkArea);

void sceSdSetAddr(u16 entry, u32 value);
#define I_sceSdSetAddr DECLARE_IMPORT(9, sceSdSetAddr);
u32 sceSdGetAddr(u16 entry);
#define I_sceSdGetAddr DECLARE_IMPORT(10, sceSdGetAddr);

void sceSdSetSwitch(u16 entry, u32 value);
#define I_sceSdSetSwitch DECLARE_IMPORT(7, sceSdSetSwitch);
u32 sceSdGetSwitch(u16 entry);
#define I_sceSdGetSwitch DECLARE_IMPORT(8, sceSdGetSwitch);

u16 sceSdNote2Pitch(u16 center_note, u16 center_fine, u16 note, s16 fine);
#define I_sceSdNote2Pitch DECLARE_IMPORT(13, sceSdNote2Pitch);
u16 sceSdPitch2Note(u16 center_note, u16 center_fine, u16 pitch);
#define I_sceSdPitch2Note DECLARE_IMPORT(14, sceSdPitch2Note);

int sceSdSetEffectAttr(int core, sceSdEffectAttr *attr);
#define I_sceSdSetEffectAttr DECLARE_IMPORT(23, sceSdSetEffectAttr);
void sceSdGetEffectAttr(int core, sceSdEffectAttr *attr);
#define I_sceSdGetEffectAttr DECLARE_IMPORT(24, sceSdGetEffectAttr);

int sceSdProcBatch(sceSdBatch *batch, u32 *rets, u32 num);
#define I_sceSdProcBatch DECLARE_IMPORT(15, sceSdProcBatch);
int sceSdProcBatchEx(sceSdBatch *batch, u32 *rets, u32 num, u32 voice);
#define I_sceSdProcBatchEx DECLARE_IMPORT(16, sceSdProcBatchEx);

int sceSdVoiceTrans(s16 chan, u16 mode, u8 *iopaddr, u32 *spuaddr, u32 size);
#define I_sceSdVoiceTrans DECLARE_IMPORT(17, sceSdVoiceTrans);
int sceSdBlockTrans(s16 chan, u16 mode, u8 *iopaddr, u32 size, u8 *startaddr);
#define I_sceSdBlockTrans DECLARE_IMPORT(18, sceSdBlockTrans);
int sceSdVoiceTransStatus(s16 channel, s16 flag);
#define I_sceSdVoiceTransStatus DECLARE_IMPORT(19, sceSdVoiceTransStatus);
u32 sceSdBlockTransStatus(s16 channel, s16 flag);
#define I_sceSdBlockTransStatus DECLARE_IMPORT(20, sceSdBlockTransStatus);

sceSdTransIntrHandler sceSdSetTransIntrHandler(int channel, sceSdTransIntrHandler func, void *arg);
#define I_sceSdSetTransIntrHandler DECLARE_IMPORT(26, sceSdSetTransIntrHandler);
sceSdSpu2IntrHandler sceSdSetSpu2IntrHandler(sceSdSpu2IntrHandler func, void *arg);
#define I_sceSdSetSpu2IntrHandler DECLARE_IMPORT(27, sceSdSetSpu2IntrHandler);

void *sceSdGetTransIntrHandlerArgument(int arg);
#define I_sceSdGetTransIntrHandlerArgument DECLARE_IMPORT(28, sceSdGetTransIntrHandlerArgument);
void *sceSdGetSpu2IntrHandlerArgument();
#define I_sceSdGetSpu2IntrHandlerArgument DECLARE_IMPORT(29, sceSdGetSpu2IntrHandlerArgument);
int sceSdStopTrans(int channel);
#define I_sceSdStopTrans DECLARE_IMPORT(30, sceSdStopTrans);
int sceSdCleanEffectWorkArea(int core, int channel, int effect_mode);
#define I_sceSdCleanEffectWorkArea DECLARE_IMPORT(31, sceSdCleanEffectWorkArea);
int sceSdSetEffectMode(int core, sceSdEffectAttr *param);
#define I_sceSdSetEffectMode DECLARE_IMPORT(32, sceSdSetEffectMode);
int sceSdSetEffectModeParams(int core, sceSdEffectAttr *attr);
#define I_sceSdSetEffectModeParams DECLARE_IMPORT(33, sceSdSetEffectModeParams);

//Backwards compatibility definitions
#define SdQuit sceSdQuit
#define SdInit sceSdInit
#define SdSetIRQCallback sceSdSetIRQCallback
#define SdSetTransCallback sceSdSetTransCallback
#define SdSetParam sceSdSetParam
#define SdGetParam sceSdGetParam
#define SdSetCoreAttr sceSdSetCoreAttr
#define SdGetCoreAttr sceSdGetCoreAttr
#define SdClearEffectWorkArea sceSdClearEffectWorkArea
#define SdSetAddr sceSdSetAddr
#define SdGetAddr sceSdGetAddr
#define SdSetSwitch sceSdSetSwitch
#define SdGetSwitch sceSdGetSwitch
#define SdNote2Pitch sceSdNote2Pitch
#define SdPitch2Note sceSdPitch2Note
#define SdSetEffectAttr sceSdSetEffectAttr
#define SdGetEffectAttr sceSdGetEffectAttr
#define SdProcBatch sceSdProcBatch
#define SdProcBatchEx sceSdProcBatchEx
#define SdVoiceTrans sceSdVoiceTrans
#define SdBlockTrans sceSdBlockTrans
#define SdVoiceTransStatus sceSdVoiceTransStatus
#define SdBlockTransStatus sceSdBlockTransStatus
#define SdSetTransIntrHandler sceSdSetTransIntrHandler
#define SdSetSpu2IntrHandler sceSdSetSpu2IntrHandler
#define SdGetTransIntrHandlerArgument sceSdGetTransIntrHandlerArgument
#define SdGetSpu2IntrHandlerArgument sceSdGetSpu2IntrHandlerArgument
#define SdStopTrans sceSdStopTrans
#define SdCleanEffectWorkArea sceSdCleanEffectWorkArea
#define SdSetEffectMode sceSdSetEffectMode
#define SdSetEffectModeParams sceSdSetEffectModeParams

#endif
