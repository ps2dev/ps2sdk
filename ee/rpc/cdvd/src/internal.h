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
 * Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
 *
 * NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
 * modules stored in rom0.
 *		
 * NOTE: not all functions work with each set of modules!
 */

#ifndef _LIBCDVD_INTERNAL_H_
#define _LIBCDVD_INTERNAL_H_

extern int CdDebug;

extern volatile int CdCallbackNum;
extern volatile int cbSema;

extern int CdThreadId;
extern ee_thread_status_t CdThreadParam;

extern int bindNCmd;
extern int bindSCmd;

extern int nCmdSemaId;
extern int sCmdSemaId;

extern int nCmdNum;

extern u8 sCmdRecvBuff[];
extern u8 nCmdRecvBuff[];

#ifdef __cplusplus
extern "C" {
#endif

void _CdSemaInit(void);
void _CdGenericCallbackFunction(void *funcNum);
int  _CdSyncS(int mode);

#ifdef __cplusplus
}
#endif

#endif	// _LIBCDVD_INTERNAL_H_

