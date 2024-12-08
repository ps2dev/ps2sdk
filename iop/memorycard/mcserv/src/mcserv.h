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

//#define DEBUG

#define MODNAME "mcserv"
#define MODVER  0x208

#ifdef SIO_DEBUG
	#include <sior.h>
	#define DEBUG
	#define DPRINTF(format, args...) \
		sio_printf(MODNAME ": " format, ##args)
#else
	#ifdef DEBUG
		#define DPRINTF(format, args...) \
			printf(MODNAME ": " format, ##args)
    #else
		#define DPRINTF(format, args...)
    #endif
#endif


// internal function prototypes
extern void *cb_rpc_S_0400(u32 fno, void *buf, int size);
extern void thread_rpc_S_0400(void* param);
extern int sceMcInit(void);
extern int sceMcOpen(void);
extern int sceMcClose(void);
extern int sceMcRead(void);
extern int sceMcWrite(void);
extern int sceMcSeek(void);
extern int sceMcGetDir(void);
extern int sceMcFormat(void);
extern int sceMcGetInfo(void);
extern int sceMcDelete(void);
extern int sceMcFlush(void);
extern int sceMcChDir(void);
extern int sceMcSetFileInfo(void);
extern int sceMcEraseBlock(void);
extern int sceMcReadPage(void);
extern int sceMcWritePage(void);
extern int sceMcUnformat(void);
extern int sceMcRead2(void);
extern int sceMcGetInfo2(void);
extern int sceMcGetEntSpace(void);
extern int sceMcCheckBlock(void);

extern int _McInit(void *rpc_buf);
extern int _McOpen(void *rpc_buf);
extern int _McClose(void *rpc_buf);
extern int _McRead(void *rpc_buf);
extern int _McWrite(void *rpc_buf);
extern int _McSeek(void *rpc_buf);
extern int _McGetDir(void *rpc_buf);
extern int _McFormat(void *rpc_buf);
extern int _McGetInfo(void *rpc_buf);
extern int _McDelete(void *rpc_buf);
extern int _McFlush(void *rpc_buf);
extern int _McChDir(void *rpc_buf);
extern int _McSetFileInfo(void *rpc_buf);
extern int _McEraseBlock(void *rpc_buf);
extern int _McReadPage(void *rpc_buf);
extern int _McWritePage(void *rpc_buf);
extern int _McUnformat(void *rpc_buf);
extern int _McRead2(void *rpc_buf);
extern int _McGetInfo2(void *rpc_buf);
extern int _McGetEntSpace(void *rpc_buf);
extern int _McCheckBlock(void *rpc_buf);

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
