/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SPEED ATAPI device driver definitions and imports.
 */

#ifndef _XATAPI_H
#define _XATAPI_H

#include <tamtypes.h>

extern int xatapi_2_terminate(int with_quit);
extern int xatapi_4_sceAtaSoftReset(void);
extern int xatapi_5_sceAtaExecCmd(
	void *buf,
	u32 blkcount,
	u16 feature,
	u16 nsector,
	u16 sector,
	u16 lcyl,
	u16 hcyl,
	u16 select,
	u16 command,
	u32 unk10);
extern int xatapi_6_sceAtaWaitResult(void);
extern int xatapi_7_sceCdAtapiExecCmd(s16 n, void *buf, int nsec, int secsize, void *pkt, int pkt_len, int proto);
extern int xatapi_8_sceCdAtapiWaitResult(void);
extern void xatapi_9_sceCdSpdAtaDmaStart(int dir);
extern void xatapi_10_sceCdSpdAtaDmaEnd(void);
extern int xatapi_11_sceAtaGetError(void);
extern int xatapi_12_get_ata_control(void);
extern int xatapi_13_get_speed_reg(int regaddr);
extern int xatapi_14_set_speed_reg(int regaddr, u16 regval);
extern int xatapi_15_exec_f6_f9_scsi(void);

#define xatapi_IMPORTS_start DECLARE_IMPORT_TABLE(xatapi, 1, 1)
#define xatapi_IMPORTS_end END_IMPORT_TABLE

#define I_xatapi_2_terminate DECLARE_IMPORT(2, xatapi_2_terminate)
#define I_xatapi_4_sceAtaSoftReset DECLARE_IMPORT(4, xatapi_4_sceAtaSoftReset)
#define I_xatapi_5_sceAtaExecCmd DECLARE_IMPORT(5, xatapi_5_sceAtaExecCmd)
#define I_xatapi_6_sceAtaWaitResult DECLARE_IMPORT(6, xatapi_6_sceAtaWaitResult)
#define I_xatapi_7_sceCdAtapiExecCmd DECLARE_IMPORT(7, xatapi_7_sceCdAtapiExecCmd)
#define I_xatapi_8_sceCdAtapiWaitResult DECLARE_IMPORT(8, xatapi_8_sceCdAtapiWaitResult)
#define I_xatapi_9_sceCdSpdAtaDmaStart DECLARE_IMPORT(9, xatapi_9_sceCdSpdAtaDmaStart)
#define I_xatapi_10_sceCdSpdAtaDmaEnd DECLARE_IMPORT(10, xatapi_10_sceCdSpdAtaDmaEnd)
#define I_xatapi_11_sceAtaGetError DECLARE_IMPORT(11, xatapi_11_sceAtaGetError)
#define I_xatapi_12_get_ata_control DECLARE_IMPORT(12, xatapi_12_get_ata_control)
#define I_xatapi_13_get_speed_reg DECLARE_IMPORT(13, xatapi_13_get_speed_reg)
#define I_xatapi_14_set_speed_reg DECLARE_IMPORT(14, xatapi_14_set_speed_reg)
#define I_xatapi_15_exec_f6_f9_scsi DECLARE_IMPORT(15, xatapi_15_exec_f6_f9_scsi)

#endif
