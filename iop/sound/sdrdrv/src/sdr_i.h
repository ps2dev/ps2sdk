/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SDR_I_H
#define _SDR_I_H

#include <irx_imports.h>
#include <sdrdrv.h>

#ifndef SDRDRV_OBSOLETE_FUNCS
#define SDRDRV_OBSOLETE_FUNCS 0
#endif
#ifndef SDRDRV_EECB_COMPAT
#define SDRDRV_EECB_COMPAT 0
#endif
#ifndef SDRDRV_IMPLEMENT_AUTODMA
#define SDRDRV_IMPLEMENT_AUTODMA 0
#endif
#ifndef SDRDRV_IMPLEMENT_LIBOSDS
#define SDRDRV_IMPLEMENT_LIBOSDS 0
#endif

typedef struct SdrEECBData_
{
	// cppcheck-suppress unusedStructMember
	int mode;
	// cppcheck-suppress unusedStructMember
	int voice_bit;
	// cppcheck-suppress unusedStructMember
	int status;
	// cppcheck-suppress unusedStructMember
	int opt;
	// cppcheck-suppress unusedStructMember
	int pad[12];
} SdrEECBData;

typedef struct SdrEECBInfo_
{
	SdrEECBData m_eeCBData;
	int m_thid_cb;
	int m_initial_priority_cb;
} SdrEECBInfo;

typedef struct SdrInfo_
{
	int m_thid_main;
	sceSdrUserCommandFunction m_sceSdr_vUserCommandFunction[16];
	SifRpcDataQueue_t *m_rpc_qd;
	SifRpcServerData_t *m_rpc_sd;
	int m_procbat_returns[384];
	sceSdEffectAttr m_e_attr;
} SdrInfo;

extern void sce_sdr_loop(void *arg);
#if SDRDRV_OBSOLETE_FUNCS
extern int _sce_sdrDMA0CallBackProc(void *data);
extern int _sce_sdrDMA1CallBackProc(void *data);
extern int _sce_sdrIRQCallBackProc(void *data);
#endif
extern int _sce_sdrDMA0IntrHandler(int core, void *common);
extern int _sce_sdrDMA1IntrHandler(int core, void *common);
extern int _sce_sdrSpu2IntrHandler(int core_bit, void *common);
extern void sce_sdrcb_loop(void *arg);

extern SdrEECBInfo g_eeCBInfo;
extern SdrInfo g_sdrInfo;

#endif
