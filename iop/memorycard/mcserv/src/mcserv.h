/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: mcserv.h 1410 2009-01-18 15:24:54Z jimmikaelkael $
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
int (*rpc_func)(void);

static const u8 XMCSERV_RpcCmd[2][16] =
{	// libmc rpc cmd values for XMCMAN/XMCSERV
	{	0xFE,	// CMD_INIT
		0x01,	// CMD_GETINFO
		0x02,	// CMD_OPEN
		0x03,	// CMD_CLOSE
		0x04,	// CMD_SEEK
		0x05,	// CMD_READ
		0x06,	// CMD_WRITE
		0x0A,	// CMD_FLUSH
		0x0C,	// CMD_CHDIR
		0x0D,	// CMD_GETDIR
		0x0E,	// CMD_SETFILEINFO
		0x0F,	// CMD_DELETE
		0x10,	// CMD_FORMAT
		0x11,	// CMD_UNFORMAT
		0x12,	// CMD_GETENTSPACE
		0x33	// CMD_CHECKBLOCK (calls xmcman_funcs: 45)
	},
	{ // corresponding internal rpc function
		0x00,	// sceMcInit
		0x12,	// sceMcGetInfo2
		0x01,	// sceMcOpen
		0x02,	// sceMcClose
		0x05,	// sceMcSeek
		0x11,	// sceMcRead2
		0x04,	// sceMcWrite
		0x0A,	// sceMcFlush
		0x0B,	// sceMcChDir
		0x06,	// sceMcGetDir
		0x0C,	// sceMcSetFileInfo
		0x09,	// sceMcDelete
		0x07,	// sceMcFormat
		0x10,	// sceMcUnformat
		0x13,	// sceMcGetEntSpace
		0x14	// sceMcCheckBlock (calls xmcman_funcs: 45)
	}
};

// rpc command handling array
void *rpc_funcs_array[21] = {
    (void *)sceMcInit,
    (void *)sceMcOpen,
    (void *)sceMcClose,
    (void *)sceMcRead,
    (void *)sceMcWrite,
    (void *)sceMcSeek,
    (void *)sceMcGetDir,
    (void *)sceMcFormat,
    (void *)sceMcGetInfo,
    (void *)sceMcDelete,
    (void *)sceMcFlush,
    (void *)sceMcChDir,
    (void *)sceMcSetFileInfo,
    (void *)sceMcEraseBlock,
    (void *)sceMcReadPage,
    (void *)sceMcWritePage,
    (void *)sceMcUnformat,
    (void *)sceMcRead2,
    (void *)sceMcGetInfo2,
    (void *)sceMcGetEntSpace,
    (void *)sceMcCheckBlock
};

// filename related mc command
// used by: McOpen, McGetDir, McChdir, McDelete, McSetFileInfo
typedef struct g_nameParam { // size = 1044
	int port;		// 0
	int slot;		// 4
	int flags;		// 8
	int maxent;		// 12
	mcTable_t *mcT;	// 16
	char name[1024];// 20
} g_nameParam_t;

// file descriptor related mc command
// used by: McInit, McClose, McSeek, McRead, McWrite, McGetinfo, McFormat, McFlush, McUnformat
typedef struct g_descParam { // size = 48
	int fd;			// 0
	int port;		// 4
	int slot;		// 8
	int size;		// 12
	int offset;		// 16
	int origin;		// 20
	void *buffer;	// 24
	void *param;	// 28
	u8  data[16];	// 32
} g_descParam_t;

// endParamenter struct
// used by: McRead, McGetInfo, McReadPage
typedef struct g_endParam { // size = 64
	int   fastIO1_size; 	// 0
	int   fastIO2_size; 	// 4
	void *fastIO1_eeaddr;	// 8
	void *fastIO2_eeaddr;	// 12
	u8    fastIO1_data[16];	// 16
	u8    fastIO2_data[32];	// 32
} g_endParam_t;

// endParamenter2 struct
// used by: McRead2, McGetInfo2
typedef struct g_endParam2 {// size = 192
	int   fastIO1_size; 	// 0
	int   fastIO2_size; 	// 4
	void *fastIO1_eeaddr;	// 8
	void *fastIO2_eeaddr;	// 12
	u8    fastIO1_data[64]; // 16
	u8    fastIO2_data[64];	// 80
	int   flag;				// 144
	u8    unused[44];		// 148
} g_endParam2_t;


#define TH_C 0x02000000
static int mcserv_tidS_0400;

static SifRpcDataQueue_t mcserv_qdS_0400 __attribute__((aligned(64)));
static SifRpcServerData_t mcserv_sdS_0400 __attribute__((aligned(64)));

static u8 mcserv_rpc_buf[2048] __attribute__((aligned(64)));

static struct {		
	int rpc_func_ret;
	int mcserv_version;
	int mcman_version;
} rpc_stat __attribute__((aligned(64)));

#define MCSERV_BUFSIZE 8192
static u8 mcserv_buf[MCSERV_BUFSIZE] __attribute__((aligned(64)));


#endif
