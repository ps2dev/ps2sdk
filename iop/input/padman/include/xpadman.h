/**
 * @file
 * Extended pad functions
 */

#ifndef __XPADMAN_H__
#define __XPADMAN_H__

#include <types.h>
#include <defs.h>

#ifdef __cplusplus
extern "C" {
#endif

extern s32 padInit(void * ee_addr);
extern s32 padEnd();
extern s32 padPortClose(s32 port, s32 slot, s32 wait);
extern s32 padPortOpen(s32 port, s32 slot, s32 pad_area_ee_addr, u32 *buf);
extern u32 padGetInBuffer(u32 port, u32 slot, u8 *buf);
extern u32 padSetupEEButtonData(u32 port, u32 slot, void *pstate);
extern u32 padGetModeConfig(u32 port, u32 slot);
extern u32 padSetMainMode(u32 port, u32 slot, u32 mode, u32 lock);
extern u32 padSetActDirect(u32 port, u32 slot, u8 *actData);
extern u32 padSetActAlign(u32 port, u32 slot, const u8 *actData);
extern u32 padGetButtonMask(u32 port, u32 slot);
extern u32 padSetButtonInfo(u32 port, u32 slot, u32 info);
extern s32 padInfoAct(u32 port, u32 slot, s32 act, u32 val);
extern s32 padInfoComb(u32 port, u32 slot, s32 val1, u32 val2);
extern s32 padInfoMode(u32 port, u32 slot, s32 val1, u32 val2);

#define xpadman_IMPORTS_start DECLARE_IMPORT_TABLE(padman, 1, 2)
#define xpadman_IMPORTS_end END_IMPORT_TABLE

#define I_padInit DECLARE_IMPORT(4, padInit)
#define I_padEnd DECLARE_IMPORT(5, padEnd)
#define I_padPortOpen DECLARE_IMPORT(6, padPortOpen)
#define I_padPortClose DECLARE_IMPORT(7, padPortClose)
#define I_padGetInBuffer DECLARE_IMPORT(8, padGetInBuffer)
#define I_padSetupEEButtonData DECLARE_IMPORT(9, padSetupEEButtonData)
#define I_padGetModeConfig DECLARE_IMPORT(10, padGetModeConfig)
#define I_padInfoAct DECLARE_IMPORT(11, padInfoAct)
#define I_padInfoComb DECLARE_IMPORT(12, padInfoComb)
#define I_padInfoMode DECLARE_IMPORT(13, padInfoMode)
#define I_padSetMainMode DECLARE_IMPORT(14, padSetMainMode)
#define I_padSetActDirect DECLARE_IMPORT(15, padSetActDirect)
#define I_padSetActAlign DECLARE_IMPORT(16, padSetActAlign)
#define I_padGetButtonMask DECLARE_IMPORT(17, padGetButtonMask)
#define I_padGetButtonInfo DECLARE_IMPORT(18, padGetButtonInfo)

#ifdef __cplusplus
}
#endif

#endif /* __XPADMAN_H__ */
