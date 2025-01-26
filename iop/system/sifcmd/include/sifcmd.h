/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SIF CMD and RPC.
 */

#ifndef __SIFCMD_H__
#define __SIFCMD_H__

#include <types.h>
#include <irx.h>

#include <sifcmd-common.h>
#include <sifrpc-common.h>

#define sifcmd_IMPORTS_start DECLARE_IMPORT_TABLE(sifcmd, 1, 1)
#define sifcmd_IMPORTS_end END_IMPORT_TABLE

#define I_sceSifInitCmd DECLARE_IMPORT(4, sceSifInitCmd)
#define I_sceSifExitCmd DECLARE_IMPORT(5, sceSifExitCmd)
#define I_sceSifGetSreg DECLARE_IMPORT(6, sceSifGetSreg)
#define I_sceSifSetSreg DECLARE_IMPORT(7, sceSifSetSreg)
#define I_sceSifSetCmdBuffer DECLARE_IMPORT(8, sceSifSetCmdBuffer)
#define I_sceSifSetSysCmdBuffer DECLARE_IMPORT(9, sceSifSetSysCmdBuffer)
#define I_sceSifAddCmdHandler DECLARE_IMPORT(10, sceSifAddCmdHandler)
#define I_sceSifRemoveCmdHandler DECLARE_IMPORT(11, sceSifRemoveCmdHandler)
#define I_sceSifSendCmd DECLARE_IMPORT(12, sceSifSendCmd)
#define I_isceSifSendCmd DECLARE_IMPORT(13, isceSifSendCmd)
#define I_sceSifInitRpc DECLARE_IMPORT(14, sceSifInitRpc)
#define I_sceSifBindRpc DECLARE_IMPORT(15, sceSifBindRpc)
#define I_sceSifCallRpc DECLARE_IMPORT(16, sceSifCallRpc)
#define I_sceSifRegisterRpc DECLARE_IMPORT(17, sceSifRegisterRpc)
#define I_sceSifCheckStatRpc DECLARE_IMPORT(18, sceSifCheckStatRpc)
#define I_sceSifSetRpcQueue DECLARE_IMPORT(19, sceSifSetRpcQueue)
#define I_sceSifGetNextRequest DECLARE_IMPORT(20, sceSifGetNextRequest)
#define I_sceSifExecRequest DECLARE_IMPORT(21, sceSifExecRequest)
#define I_sceSifRpcLoop DECLARE_IMPORT(22, sceSifRpcLoop)
#define I_sceSifGetOtherData DECLARE_IMPORT(23, sceSifGetOtherData)
#define I_sceSifRemoveRpc DECLARE_IMPORT(24, sceSifRemoveRpc)
#define I_sceSifRemoveRpcQueue DECLARE_IMPORT(25, sceSifRemoveRpcQueue)
#define I_sceSifSetSif1CB DECLARE_IMPORT(26, sceSifSetSif1CB)
#define I_sceSifClearSif1CB DECLARE_IMPORT(27, sceSifClearSif1CB)
#define I_sceSifSendCmdIntr DECLARE_IMPORT(28, sceSifSendCmdIntr)
#define I_isceSifSendCmdIntr DECLARE_IMPORT(29, isceSifSendCmdIntr)

#endif	/* __SIFCMD_H__ */
