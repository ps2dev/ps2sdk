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
 * Additional thbase functions only found in newer IOPRP images
 */

#ifndef IOP_XTHBASE_H
#define IOP_XTHBASE_H

#include "thbase.h"

int GetThreadCurrentPriority(void);
#define I_GetThreadCurrentPriority DECLARE_IMPORT(42, GetThreadCurrentPriority)
unsigned int GetSystemTimeLow(void);
#define I_GetSystemTimeLow DECLARE_IMPORT(43, GetSystemTimeLow)
int ReferSystemStatus(iop_sys_status_t *info, size_t size);
#define I_ReferSystemStatus DECLARE_IMPORT(44, ReferSystemStatus)
int ReferThreadRunStatus(int thid, iop_thread_run_status_t *stat, size_t size);
#define I_ReferThreadRunStatus DECLARE_IMPORT(45, ReferThreadRunStatus)
int GetThreadStackFreeSize(int thid);
#define I_GetThreadStackFreeSize DECLARE_IMPORT(46, GetThreadStackFreeSize)
int GetThreadmanIdList(int type, int *readbuf, int readbufsize, int *objectcount);
#define I_GetThreadmanIdList DECLARE_IMPORT(47, GetThreadmanIdList)

#endif /* IOP_XTHBASE_H */
