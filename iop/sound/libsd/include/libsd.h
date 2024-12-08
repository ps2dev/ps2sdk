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
 * LIBSD definitions and imports.
 */

#ifndef __LIBSD_H__
#define __LIBSD_H__

#include <types.h>
#include <irx.h>
#include <libsd-common.h>

#define libsd_IMPORTS_start DECLARE_IMPORT_TABLE(libsd, 1, 4)
#define libsd_IMPORTS_end END_IMPORT_TABLE

#define I_sceSdQuit DECLARE_IMPORT(2, sceSdQuit);
#define I_sceSdInit DECLARE_IMPORT(4, sceSdInit);
#define I_sceSdSetIRQCallback DECLARE_IMPORT(22, sceSdSetIRQCallback);
#define I_sceSdSetTransCallback DECLARE_IMPORT(21, sceSdSetTransCallback);
#define I_sceSdSetParam DECLARE_IMPORT(5, sceSdSetParam);
#define I_sceSdGetParam DECLARE_IMPORT(6, sceSdGetParam);
#define I_sceSdSetCoreAttr DECLARE_IMPORT(11, sceSdSetCoreAttr);
#define I_sceSdGetCoreAttr DECLARE_IMPORT(12, sceSdGetCoreAttr);
#define I_sceSdClearEffectWorkArea DECLARE_IMPORT(25, sceSdClearEffectWorkArea);
#define I_sceSdSetAddr DECLARE_IMPORT(9, sceSdSetAddr);
#define I_sceSdGetAddr DECLARE_IMPORT(10, sceSdGetAddr);
#define I_sceSdSetSwitch DECLARE_IMPORT(7, sceSdSetSwitch);
#define I_sceSdGetSwitch DECLARE_IMPORT(8, sceSdGetSwitch);
#define I_sceSdNote2Pitch DECLARE_IMPORT(13, sceSdNote2Pitch);
#define I_sceSdPitch2Note DECLARE_IMPORT(14, sceSdPitch2Note);
#define I_sceSdSetEffectAttr DECLARE_IMPORT(23, sceSdSetEffectAttr);
#define I_sceSdGetEffectAttr DECLARE_IMPORT(24, sceSdGetEffectAttr);
#define I_sceSdProcBatch DECLARE_IMPORT(15, sceSdProcBatch);
#define I_sceSdProcBatchEx DECLARE_IMPORT(16, sceSdProcBatchEx);
#define I_sceSdVoiceTrans DECLARE_IMPORT(17, sceSdVoiceTrans);
#define I_sceSdBlockTrans DECLARE_IMPORT(18, sceSdBlockTrans);
#define I_sceSdVoiceTransStatus DECLARE_IMPORT(19, sceSdVoiceTransStatus);
#define I_sceSdBlockTransStatus DECLARE_IMPORT(20, sceSdBlockTransStatus);
#define I_sceSdSetTransIntrHandler DECLARE_IMPORT(26, sceSdSetTransIntrHandler);
#define I_sceSdSetSpu2IntrHandler DECLARE_IMPORT(27, sceSdSetSpu2IntrHandler);
#define I_sceSdGetTransIntrHandlerArgument DECLARE_IMPORT(28, sceSdGetTransIntrHandlerArgument);
#define I_sceSdGetSpu2IntrHandlerArgument DECLARE_IMPORT(29, sceSdGetSpu2IntrHandlerArgument);
#define I_sceSdStopTrans DECLARE_IMPORT(30, sceSdStopTrans);
#define I_sceSdCleanEffectWorkArea DECLARE_IMPORT(31, sceSdCleanEffectWorkArea);
#define I_sceSdSetEffectMode DECLARE_IMPORT(32, sceSdSetEffectMode);
#define I_sceSdSetEffectModeParams DECLARE_IMPORT(33, sceSdSetEffectModeParams);

#endif /* __LIBSD_H__ */
