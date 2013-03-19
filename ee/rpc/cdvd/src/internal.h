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
# Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#		
# NOTE: not all functions work with each set of modules!
*/

#ifndef _LIBCDVD_INTERNAL_H_
#define _LIBCDVD_INTERNAL_H_

extern s32 sceCdDebug;

extern volatile s32 sceCdCallbackNum;
extern volatile s32 cbSema;

extern s32 sceCdThreadId;
extern ee_thread_status_t sceCdThreadParam;

extern s32 bindNCmd;
extern s32 bindSCmd;

extern s32 nCmdSemaId;
extern s32 sCmdSemaId;

extern s32 nCmdNum;

extern u8 sCmdRecvBuff[];
extern u8 nCmdRecvBuff[];

#ifdef __cplusplus
extern "C" {
#endif

void sceCdSemaInit(void);

void sceCdGenericCallbackFunction(void *funcNum);

s32  sceCdSyncS(s32 mode);

#ifdef __cplusplus
}
#endif

#endif	// _LIBCDVD_INTERNAL_H_

