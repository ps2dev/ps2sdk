/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __MCSERV_H__
#define __MCSERV_H__

#include <loadcore.h>
#include <intrman.h>
#include <sysclib.h>
#include <sifman.h>
#include <sifcmd.h>
#include <thbase.h>
#include <stdio.h>
#include <mcman.h>
#include "mcman_imports.h"

//#define DEBUG

#define MODNAME "mcserv"
#define MODVER  0x208

// internal function prototypes
void *cb_rpc_S_0400(u32 fno, void *buf, int size);
void thread_rpc_S_0400(void* param);
int sceMcInit(void);
int sceMcOpen(void);
int sceMcClose(void);
int sceMcRead(void);
int sceMcWrite(void);
int sceMcSeek(void);
int sceMcGetDir(void);
int sceMcFormat(void);
int sceMcGetInfo(void);
int sceMcDelete(void);
int sceMcFlush(void);
int sceMcChDir(void);
int sceMcSetFileInfo(void);
int sceMcEraseBlock(void);
int sceMcReadPage(void);
int sceMcWritePage(void);
int sceMcUnformat(void);
int sceMcRead2(void);
int sceMcGetInfo2(void);
int sceMcGetEntSpace(void);
int sceMcCheckBlock(void);

int _McInit(void *rpc_buf);
int _McOpen(void *rpc_buf);
int _McClose(void *rpc_buf);
int _McRead(void *rpc_buf);
int _McWrite(void *rpc_buf);
int _McSeek(void *rpc_buf);
int _McGetDir(void *rpc_buf);
int _McFormat(void *rpc_buf);
int _McGetInfo(void *rpc_buf);
int _McDelete(void *rpc_buf);
int _McFlush(void *rpc_buf);
int _McChDir(void *rpc_buf);
int _McSetFileInfo(void *rpc_buf);
int _McEraseBlock(void *rpc_buf);
int _McReadPage(void *rpc_buf);
int _McWritePage(void *rpc_buf);
int _McUnformat(void *rpc_buf);
int _McRead2(void *rpc_buf);
int _McGetInfo2(void *rpc_buf);
int _McGetEntSpace(void *rpc_buf);
int _McCheckBlock(void *rpc_buf);

// filename related mc command
// used by: mcOpen, mcGetDir, mcChdir, mcDelete, mcSetFileInfo, mcRename, mcGetEntSpace
typedef struct {			// size = 1044
	int port;			// 0
	int slot;			// 4
	int flags;			// 8
	int maxent;			// 12
	union {
		sceMcTblGetDir *mcT;	// 16
		char *curdir;
	};
	char name[1024];		// 20
} mcNameParam_t;

#endif
