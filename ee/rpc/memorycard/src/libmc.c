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
 * Function defenitions for mclib.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include "libmc.h"

//#define MC_DEBUG

#ifdef MC_DEBUG
#include <stdio.h>
#endif


/** rpc command function numbers */
enum MC_RPCCMD_NUMBERS{
	MC_RPCCMD_INIT		= 0x00,
	MC_RPCCMD_GET_INFO,
	MC_RPCCMD_OPEN,
	MC_RPCCMD_CLOSE,
	MC_RPCCMD_SEEK,
	MC_RPCCMD_READ,
	MC_RPCCMD_WRITE,
	MC_RPCCMD_FLUSH,
	MC_RPCCMD_CH_DIR,
	MC_RPCCMD_GET_DIR,
	MC_RPCCMD_SET_INFO,
	MC_RPCCMD_DELETE,
	MC_RPCCMD_FORMAT,
	MC_RPCCMD_UNFORMAT,
	MC_RPCCMD_GET_ENT,
	MC_RPCCMD_CHG_PRITY,
	MC_RPCCMD_CHECK_BLOCK,
	MC_RPCCMD_ERASE_BLOCK	= 0x0E,
	MC_RPCCMD_READ_PAGE,
	MC_RPCCMD_WRITE_PAGE
};

/** rpc command function numbers
 * mcRpcCmd[MC_TYPE_??][MC_RPCCMD_???]
 */
static const int mcRpcCmd[2][17] =
{
	// MCMAN/MCSERV values
	{
	0x70,	// MC_RPCCMD_INIT
	0x78,	// MC_RPCCMD_GET_INFO
	0x71,	// MC_RPCCMD_OPEN
	0x72,	// MC_RPCCMD_CLOSE
	0x75,	// MC_RPCCMD_SEEK
	0x73,	// MC_RPCCMD_READ
	0x74,	// MC_RPCCMD_WRITE
	0x7A,	// MC_RPCCMD_FLUSH
	0x7B,	// MC_RPCCMD_CH_DIR
	0x76,	// MC_RPCCMD_GET_DIR
	0x7C,	// MC_RPCCMD_SET_INFO
	0x79,	// MC_RPCCMD_DELETE
	0x77,	// MC_RPCCMD_FORMAT
	0x80,	// MC_RPCCMD_UNFORMAT
	0x7D,	// MC_RPCCMD_ERASE_BLOCK (calls mcman_funcs: 39, 17, 20, 19, 30)
	0x7E,	// MC_RPCCMD_READ_PAGE
	0x7F,	// MC_RPCCMD_WRITE_PAGE (calls mcman_funcs: 20, 19)
	},
	// XMCMAN/XMCSERV values
	{
	0xFE,	// MC_RPCCMD_INIT
	0x01,	// MC_RPCCMD_GET_INFO
	0x02,	// MC_RPCCMD_OPEN
	0x03,	// MC_RPCCMD_CLOSE
	0x04,	// MC_RPCCMD_SEEK
	0x05,	// MC_RPCCMD_READ
	0x06,	// MC_RPCCMD_WRITE
	0x0A,	// MC_RPCCMD_FLUSH
	0x0C,	// MC_RPCCMD_CH_DIR
	0x0D,	// MC_RPCCMD_GET_DIR
	0x0E,	// MC_RPCCMD_SET_INFO
	0x0F,	// MC_RPCCMD_DELETE
	0x10,	// MC_RPCCMD_FORMAT
	0x11,	// MC_RPCCMD_UNFORMAT
	0x12,	// MC_RPCCMD_GET_ENT
	0x14,	// MC_RPCCMD_CHG_PRITY
	0x33,	// MC_RPCCMD_CHECK_BLOCK (calls xmcman_funcs: 45)
	}
};

/** filename related mc command
 * used by: mcOpen, mcGetDir, mcChdir, mcDelete, mcSetFileInfo, mcRename, mcGetEntSpace
 */
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

static mcNameParam_t g_nameParam __attribute__((aligned(64)));

/** file descriptor related mc command
 * used by: mcInit, mcClose, mcSeek, mcRead, mcWrite, mcGetinfo, mcFormat, mcFlush, mcUnformat, mcChangeThreadPriority
 */
static mcDescParam_t g_descParam __attribute__((aligned(64)));

// params sent with rpc commands
//static McRpcNameParam g_nameParam  __attribute__((aligned(64)));
//static McRpcDescParam g_descParam  __attribute__((aligned(64)));
//static unsigned int mcInfoCmd[12] __attribute__((aligned(64)));

/** external IOP reboot count */
extern int _iop_reboot_count;

/** rpc client data */
static SifRpcClientData_t g_cdata __attribute__((aligned(64)));

/** rpc receive data buffer */
#define RSIZE 2048
static union{
	s32 result;
	mcRpcStat_t rpcStat;
	u8 buffer[RSIZE];
} g_rdata __attribute__((aligned(64)));

static int* g_pType = NULL;
static int* g_pFree = NULL;
static int* g_pFormat = NULL;

static int  endParameter[48];
static char curDir[1024];
static sceMcTblGetDir g_fileInfoBuff __attribute__((aligned(64)));	// used by mcSetFileInfo and mcRename


/** whether or not mc lib has been inited */
static int g_mclibInited = 0;

/** stores command currently being executed on the iop */
static unsigned int g_currentCmd = 0;

/** specifies whether using MCSERV or XMCSERV modules */
static int g_mcType = MC_TYPE_MC;


/** function that gets called when mcGetInfo ends
 * and interrupts are disabled
 */
static void mcGetInfoApdx(void* info)
{
	mcEndParam_t	*ptr	= (mcEndParam_t*)UNCACHED_SEG(info);
	mcEndParam2_t	*ptrNew	= (mcEndParam2_t*)UNCACHED_SEG(info);

	// older MCSERV doesnt support retrieving whether card is formatted
	// so if a card is present, determine whether its formatted based on the return value from MCSERV
	if(g_mcType == MC_TYPE_MC)
	{
		if(g_pType != NULL)
			*g_pType = ptr->type;

		if(g_pFree != NULL)
			*g_pFree = ptr->free;

		if(g_pFormat != NULL)
			*g_pFormat = (ptr->type == MC_TYPE_NONE || g_rdata.result == -2) ? 0 : 1;
	} else {
		if(g_pType != NULL)
			*g_pType = ptrNew->type;

		if(g_pFree != NULL)
			*g_pFree = ptrNew->free;

		if(g_pFormat != NULL)
			*g_pFormat = ptrNew->formatted;
	}
}

/** function that gets called when mcRead ends
 * and interrupts are disabled
 */
static void mcReadFixAlign(void* data_raw)
{
	mcEndParam_t	*ptr	= (mcEndParam_t*)UNCACHED_SEG(data_raw);
	mcEndParam2_t	*ptrNew	= (mcEndParam2_t*)UNCACHED_SEG(data_raw);
	u8 *dest;
	int i;

	if(g_mcType == MC_TYPE_MC)
	{
		for(i = 0,dest = (u8*)ptr->dest1; i < ptr->size1; i++)
			dest[i] = ptr->src1[i];
		for(i = 0,dest = (u8*)ptr->dest2; i < ptr->size2; i++)
			dest[i] = ptr->src2[i];
	} else {
		for(i = 0,dest = (u8*)ptrNew->dest1; i < ptrNew->size1; i++)
			dest[i] = ptrNew->src1[i];
		for(i = 0,dest = (u8*)ptrNew->dest2; i < ptrNew->size2; i++)
			dest[i] = ptrNew->src2[i];
	}
}

/** function that gets called when mcChDir ends
 * and interrupts are disabled
 */
static void mcStoreDir(void* arg)
{
	int len;
	char *currentDir = UNCACHED_SEG(curDir);
	len = strlen(currentDir);
	if(len >= 1024)
		len = strlen(currentDir+1023);
	memcpy(arg, currentDir, len);
	*(currentDir+len) = 0;
}

int mcInit(int type)
{
	int ret=0;
	mcRpcStat_t *rpcStat = (mcRpcStat_t*)UNCACHED_SEG(&g_rdata.rpcStat);
	static int _rb_count = 0;

	if(_rb_count != _iop_reboot_count)
	{
		_rb_count = _iop_reboot_count;
		mcReset();
	}

	if(g_mclibInited)
		return -1;

	SifInitRpc(0);

	// set which modules to use
	g_mcType = type;

	// bind to mc rpc on iop
	do
	{
		if((ret=SifBindRpc(&g_cdata, 0x80000400, 0)) < 0)
		{
			#ifdef MC_DEBUG
				printf("libmc: bind error\n");
			#endif

			return ret;
		}
		if(g_cdata.server == NULL)
			nopdelay();
	}
	while (g_cdata.server == NULL);

	// for some reason calling this init sif function with 'mcserv' makes all other
	// functions not work properly. although NOT calling it means that detecting
	// whether or not cards are formatted doesnt seem to work :P
	if(g_mcType == MC_TYPE_MC)
	{
#ifdef MC_DEBUG
		printf("libmc: using MCMAN & MCSERV\n");

#endif
		g_descParam.offset = -217;

		// call init function
		if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_INIT], 0, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL))>=0)
		{
			ret = g_rdata.result;
		}
		else{
			// init error
#ifdef MC_DEBUG
			printf("libmc: initialisation error\n");
#endif
			g_mclibInited = 0;
			return g_rdata.result - 100;
		}
	}
	else if(g_mcType == MC_TYPE_XMC)
	{
#ifdef MC_DEBUG
		printf("libmc: using XMCMAN & XMCSERV\n");
#endif

		// call init function
		if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_INIT], 0, &g_descParam, sizeof(g_descParam), &g_rdata, 12, NULL, NULL)) < 0)
		{
			// init error
#ifdef MC_DEBUG
			printf("libmc: initialisation error\n");
#endif
			g_mclibInited = 0;
			return ret - 100;
		}

		// check if old version of mcserv loaded
		if(rpcStat->mcserv_version < 0x205)
		{
#ifdef MC_DEBUG
			printf("libmc: mcserv is too old (%x)\n", rpcStat->mcserv_version);
#endif
			g_mclibInited = 0;
			return -120;
		}

		// check if old version of mcman loaded
		if(rpcStat->mcman_version < 0x206)
		{
#ifdef MC_DEBUG
			printf("libmc: mcman is too old (%x)\n", rpcStat->mcman_version);
#endif
			g_mclibInited = 0;
			return -121;
		}
		ret = rpcStat->result;
	}

	// successfully inited
	g_mclibInited = 1;
	g_currentCmd = 0;
	return ret;
}

int mcGetInfo(int port, int slot, int* type, int* free, int* format)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	if(g_mcType == MC_TYPE_MC)
	{
		g_descParam.port	= port;
		g_descParam.slot	= slot;
		g_descParam.size	= (type)	? 1 : 0;
		g_descParam.offset	= (free)	? 1 : 0;
		g_descParam.origin	= (format)	? 1 : 0;
		g_descParam.param	= endParameter;
	}
	else
	{
		g_descParam.port	= port;
		g_descParam.slot	= slot;
		g_descParam.size	= (format)	? 1 : 0;
		g_descParam.offset	= (free)	? 1 : 0;
		g_descParam.origin	= (type)	? 1 : 0;
		g_descParam.param	= endParameter;
	}
	g_pType		= type;
	g_pFree		= free;
	g_pFormat	= format;
	SifWriteBackDCache(endParameter, 192);

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_GET_INFO], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, (SifRpcEndFunc_t)mcGetInfoApdx, endParameter)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_INFO;
	return ret;
}

int mcOpen(int port, int slot, const char *name, int mode)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port		= port;
	g_nameParam.slot		= slot;
	g_nameParam.flags		= mode;
	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_OPEN], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_OPEN;
	return ret;
}

int mcClose(int fd)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.fd	= fd;

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_CLOSE], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CLOSE;
	return ret;
}

int mcSeek(int fd, int offset, int origin)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.fd		= fd;
	g_descParam.offset	= offset;
	g_descParam.origin	= origin;

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_SEEK], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_SEEK;
	return ret;
}

int mcRead(int fd, void *buffer, int size)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.fd		= fd;
	g_descParam.size	= size;
	g_descParam.buffer	= buffer;
	g_descParam.param	= endParameter;
	SifWriteBackDCache(buffer, size);
	SifWriteBackDCache(endParameter, 192);

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_READ], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, (SifRpcEndFunc_t)mcReadFixAlign, endParameter)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_READ;
	return ret;
}

int mcWrite(int fd, const void *buffer, int size)
{
	int i, ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.fd	= fd;
	if(size < 17)
	{
		g_descParam.size	= 0;
		g_descParam.origin	= size;
		g_descParam.buffer	= 0;
	}
	else
	{
		g_descParam.size	= size        - ( ((int)(buffer-1) & 0xFFFFFFF0) - (int)(buffer-16) );
		g_descParam.origin	=               ( ((int)(buffer-1) & 0xFFFFFFF0) - (int)(buffer-16) );
		g_descParam.buffer	= (void*)((int)buffer + ( ((int)(buffer-1) & 0xFFFFFFF0) - (int)(buffer-16) ));
	}
	for(i=0; i<g_descParam.origin; i++)
	{
		g_descParam.data[i] = *(char*)(buffer+i);
	}
	FlushCache(0);

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_WRITE], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_WRITE;
	return ret;
}

int mcFlush(int fd)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.fd	= fd;

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_FLUSH], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_FLUSH;
	return ret;
}

int mcMkDir(int port, int slot, const char* name)
{
	int ret = mcOpen(port, slot, name, 0x40);

	if(ret != 0)
		g_currentCmd = MC_FUNC_MK_DIR;
	return ret;
}

int mcChdir(int port, int slot, const char* newDir, char* currentDir)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port		= port;
	g_nameParam.slot		= slot;
	g_nameParam.curdir		= curDir;
	strncpy(g_nameParam.name, newDir, 1023);
	g_nameParam.name[1023] = 0;
	SifWriteBackDCache(curDir, 1024);

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_CH_DIR], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, (SifRpcEndFunc_t)mcStoreDir, currentDir)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CH_DIR;
	return ret;
}

int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, sceMcTblGetDir* table)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port	= port;
	g_nameParam.slot	= slot;
	g_nameParam.flags	= mode;
	g_nameParam.maxent	= maxent;
	g_nameParam.mcT		= table;
	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;
	SifWriteBackDCache(table, maxent * sizeof(sceMcTblGetDir));

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_GET_DIR], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_DIR;
   	return ret;
}

int mcSetFileInfo(int port, int slot, const char* name, const sceMcTblGetDir* info, unsigned flags)
{
	int ret;

	// check mc lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port	= port;
	g_nameParam.slot	= slot;
	g_nameParam.flags	= flags;	// NOTE: this was ANDed with 7 so that u cant turn off copy protect! :)
	g_nameParam.mcT		= &g_fileInfoBuff;
	memcpy(&g_fileInfoBuff, info, sizeof(sceMcTblGetDir));

	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;
	FlushCache(0);

	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_SET_INFO], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_SET_INFO;
	return ret;
}

int mcDelete(int port, int slot, const char *name)
{
	int ret;

	// check lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port = port;
	g_nameParam.slot = slot;
	g_nameParam.flags = 0;
	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;

	// call delete function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_DELETE], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_DELETE;
   	return ret;
}

int mcFormat(int port, int slot)
{
	int ret;

	// check lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.port = port;
	g_descParam.slot = slot;

	// call format function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_FORMAT], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_FORMAT;
	return ret;
}

int mcUnformat(int port, int slot)
{
	int ret;

	// check lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_descParam.port = port;
	g_descParam.slot = slot;

	// call unformat function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_UNFORMAT], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_UNFORMAT;
	return ret;
}

int mcGetEntSpace(int port, int slot, const char* path)
{
	int ret;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_MC))
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port = port;
	g_nameParam.slot = slot;
	strncpy(g_nameParam.name, path, 1023);
	g_nameParam.name[1023] = 0;

	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_GET_ENT], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_ENT;
	return ret;
}

int mcRename(int port, int slot, const char* oldName, const char* newName)
{
	int ret;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_MC))	//I don't think that the old MCSERV module supports this because the v1.00 and v1.01 OSDSYS doesn't seem to have the sceMcRename function at all and the sceMcRename function was only introduced with SCE PS2SDK v1.50. I see that it doesn't work with rom0:MCSERV either way...
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
	g_nameParam.port = port;
	g_nameParam.slot = slot;
	g_nameParam.flags = 0x10;
	g_nameParam.mcT = &g_fileInfoBuff;
	strncpy(g_nameParam.name, oldName, 1023);
	g_nameParam.name[1023] = 0;
	strncpy((char*)g_fileInfoBuff.EntryName, newName, 31);
	g_fileInfoBuff.EntryName[31] = 0;
	FlushCache(0);

	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_SET_INFO], SIF_RPC_M_NOWAIT, &g_nameParam, sizeof(g_nameParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_RENAME;
	return ret;
}

int mcEraseBlock(int port, int slot, int block, int mode){
	int result;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_XMC))
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	g_descParam.port=port;
	g_descParam.slot=slot;
	g_descParam.offset=block;
	g_descParam.origin=mode;

	if((result = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_ERASE_BLOCK], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL))==0){
		g_currentCmd = MC_FUNC_ERASE_BLOCK;
	}

	return result;
}

struct libmc_PageReadAlignData{
	unsigned int size1;
	unsigned int size2;
	void *dest1;
	void *dest2;
	unsigned char data1[16];
	unsigned char data2[16];
	unsigned char padding[16];
};

static struct libmc_PageReadAlignData libmc_ReadPageAlignData;

static void libmc_ReadAlignFunction(struct libmc_PageReadAlignData *data){
	unsigned int misaligned;

	if((misaligned=(unsigned int)data->dest1&0xF)!=0){
		memcpy(UNCACHED_SEG(data->dest1), UNCACHED_SEG(data->data1), 16-misaligned);
		memcpy(UNCACHED_SEG((unsigned int)data->dest1+(16-misaligned)), UNCACHED_SEG((unsigned int)data->data1+(16-misaligned)+0x1F0), misaligned);
	}
}

int mcReadPage(int port, int slot, unsigned int page, void *buffer){
	int result;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_XMC))
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	g_descParam.fd=page;
	g_descParam.port=port;
	g_descParam.slot=slot;
	g_descParam.buffer=buffer;
	g_descParam.param=&libmc_ReadPageAlignData;

	SifWriteBackDCache(buffer, 0x200);

	if((result = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_READ_PAGE], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, (void*)&libmc_ReadAlignFunction, UNCACHED_SEG(&libmc_ReadPageAlignData)))==0){
		g_currentCmd = MC_FUNC_READ_PAGE;
	}

	return result;
}

int mcWritePage(int port, int slot, int page, const void *buffer){
	int result, misaligned;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_XMC))
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	g_descParam.fd=page;
	g_descParam.port=port;
	g_descParam.slot=slot;
	g_descParam.buffer=(void*)buffer;

	SifWriteBackDCache((void*)buffer, 512);

	if((misaligned=(unsigned int)buffer&0xF)!=0){
		memcpy(g_descParam.data, buffer, 16-misaligned);
		memcpy((void*)(g_descParam.data+(16-misaligned)), (void*)(buffer+(16-misaligned)+0x1F0), misaligned);
	}

	if((result = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_WRITE_PAGE], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL))==0){
		g_currentCmd = MC_FUNC_WRITE_PAGE;
	}

	return result;
}

int mcChangeThreadPriority(int level)
{
	int ret;

	// check lib is inited
	if((!g_mclibInited)||(g_mcType==MC_TYPE_MC))
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;

	// set global variables
//	*(u32*)mcCmd.name = level;

	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_CHG_PRITY], SIF_RPC_M_NOWAIT, &g_descParam, sizeof(g_descParam), &g_rdata, 4, NULL, NULL)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CHG_PRITY;
	return ret;
}

int mcSync(int mode, int *cmd, int *result)
{
	int funcIsExecuting, i;

	// check if any functions are registered
	if(g_currentCmd == MC_FUNC_NONE)
		return -1;

	// check if function is still processing
	funcIsExecuting = SifCheckStatRpc(&g_cdata);

	// if mode = 0, wait for function to finish
	if(mode == 0)
	{
		while(SifCheckStatRpc(&g_cdata))
		{
			for(i=0; i<100000; i++)
    			;
		}
		// function has finished
		funcIsExecuting = 0;
	}

	// get the function that just finished
	if(cmd)
		*cmd = g_currentCmd;

	// if function is still processing, return 0
	if(funcIsExecuting == 1)
		return 0;

	// function has finished, so clear last command
	g_currentCmd = 0;

	// get result
	if(result)
		*result = g_rdata.result;

	return 1;
}

int mcReset(void)
{
	g_mclibInited = 0;
	g_cdata.server = NULL;
	return 0;
}

