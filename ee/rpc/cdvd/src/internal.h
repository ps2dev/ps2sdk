/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
                               2003 loser (loser@internalreality.com)
  ------------------------------------------------------------------------
  libcdvd.c
  		Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
		
		NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
		modules stored in rom0.
		
		NOTE: not all functions work with each set of modules!
*/

#ifndef _LIBCDVD_INTERNAL_H_
#define _LIBCDVD_INTERNAL_H_

extern s32 cdDebug;

extern volatile s32 cdCallbackNum;
extern volatile s32 cbSema;

extern s32 cdThreadId;
extern ee_thread_t cdThreadParam;

extern s32 bindNCmd;
extern s32 bindSCmd;

extern s32 nCmdSemaId;
extern s32 sCmdSemaId;

extern s32 nCmdNum;

extern u8 sCmdRecvBuff[];
extern u8 nCmdRecvBuff[];

void cdSemaInit(void);

void cdCallback(void *funcNum);

s32  cdSyncS(s32 mode);


#endif	// _LIBCDVD_INTERNAL_H_

