#ifndef IOP_XPADMAN_H
#define IOP_XPADMAN_H

#include "irx.h"

s32 padInit(void * ee_addr);
s32 padEnd();
s32 padPortClose(s32 port, s32 slot, s32 wait);
s32 padPortOpen(s32 port, s32 slot, s32 pad_area_ee_addr, u32 *buf);
u32 padGetInBuffer(u32 port, u32 slot, u8 *buf);
u32 padSetupEEButtonData(u32 port, u32 slot, void *pstate)
u32 padGetModeConfig(u32 port, u32 slot);
u32 padSetMainMode(u32 port, u32 slot, u32 mode, u32 lock);
u32 padSetActDirect(u32 port, u32 slot, u8 *actData);
u32 padSetActAlign(u32 port, u32 slot, u8 *actData);
u32 padGetButtonMask(u32 port, u32 slot);
u32 padSetButtonInfo(u32 port, u32 slot, u32 info);
s32 padInfoAct(u32 port, u32 slot, s32 act, u32 val);
s32 padInfoComb(u32 port, u32 slot, s32 val1, u32 val2);
s32 padInfoMode(u32 port, u32 slot, s32 val1, u32 val2);

#define xpadman_IMPORTS_start DECLARE_IMPORT_TABLE(padman, 1, 2)
#define xpadman_IMPORTS_end END_IMPORT_TABLE

#define I_padInit				DECLARE_IMPORT(4, padInit)
#define I_padEnd				DECLARE_IMPORT(5, padEnd)
#define I_padPortOpen			DECLARE_IMPORT(6, padPortOpen)
#define I_padPortClose			DECLARE_IMPORT(7, padPortClose)
#define I_padGetInBuffer		DECLARE_IMPORT(8, padGetInBuffer)
#define I_padSetupEEButtonData	DECLARE_IMPORT(9, padSetupEEButtonData)
#define I_padGetModeConfig		DECLARE_IMPORT(10, padGetModeConfig)
#define I_padInfoAct			DECLARE_IMPORT(11, padInfoAct)
#define I_padInfoComb			DECLARE_IMPORT(12, padInfoComb)
#define I_padInfoMode			DECLARE_IMPORT(13, padInfoMode)
#define I_padSetMainMode		DECLARE_IMPORT(14, padSetMainMode)
#define I_padSetActDirect		DECLARE_IMPORT(15, padSetActDirect)
#define I_padSetActAlign		DECLARE_IMPORT(16, padSetActAlign)
#define I_padGetButtonMask		DECLARE_IMPORT(17, padGetButtonMask)
#define I_PadGetButtonInfo		DECLARE_IMPORT(18, padGetButtonInfo);

#endif

