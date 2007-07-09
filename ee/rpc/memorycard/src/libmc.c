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
# Function defenitions for mclib.
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


// rpc command function numbers
#define MC_RPCCMD_INIT		0x00
#define MC_RPCCMD_GET_INFO	0x01
#define MC_RPCCMD_OPEN		0x02
#define MC_RPCCMD_CLOSE		0x03
#define MC_RPCCMD_SEEK		0x04
#define MC_RPCCMD_READ		0x05
#define MC_RPCCMD_WRITE		0x06
#define MC_RPCCMD_FLUSH		0x07
#define MC_RPCCMD_CH_DIR	0x08
#define MC_RPCCMD_GET_DIR	0x09
#define MC_RPCCMD_SET_INFO	0x0A
#define MC_RPCCMD_DELETE	0x0B
#define MC_RPCCMD_FORMAT	0x0C
#define MC_RPCCMD_UNFORMAT	0x0D
#define MC_RPCCMD_GET_ENT	0x0E
#define MC_RPCCMD_CHG_PRITY	0x0F
#define MC_RPCCMD_UNKNOWN_1	0x10

// rpc command function numbers
// mcRpcCmd[MC_TYPE_??][MC_RPCCMD_???]
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
	0x7E,	// MC_RPCCMD_GET_ENT
	0x7D,	// MC_RPCCMD_UNKNOWN_1 (calls mcman_funcs: 39, 17, 20, 19, 30)
	0x7F,	// MC_RPCCMD_UNKNOWN_2 (calls mcman_funcs: 20, 19)
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
	0x33,	// MC_RPCCMD_UNKNOWN_1 (calls xmcman_funcs: 45)
	}
};

// filename related mc command
// used by: mcOpen, mcGetDir, mcChdir, mcDelete, mcSetFileInfo, mcRename, mcGetEntSpace
static struct {		// size = 1044
	int port;		// 0
	int slot;		// 4
	int flags;		// 8
	int maxent;		// 12
	int table;		// 16
	char name[1024];// 20
} g_nameParam __attribute__((aligned(64)));

// file descriptor related mc command
// used by: mcInit, mcClose, mcSeek, mcRead, mcWrite, mcGetinfo, mcFormat, mcFlush, mcUnformat, mcChangeThreadPriority
static struct {		// size = 48
	int fd;			// 0
	int port;		// 4
	int slot;		// 8
	int size;		// 12
	int offset;		// 16
	int origin;		// 20
	int buffer;		// 24
	int param;		// 28
	char data[16];	// 32
} g_descParam __attribute__((aligned(64)));

// params sent with rpc commands
//static McRpcNameParam g_nameParam  __attribute__((aligned(64)));
//static McRpcDescParam g_descParam  __attribute__((aligned(64)));
//static unsigned int mcInfoCmd[12] __attribute__((aligned(64)));

// rpc client data
static SifRpcClientData_t g_cdata __attribute__((aligned(64)));

// rpc receive data buffer
#define RSIZE 2048
static u8 g_rdata[RSIZE] __attribute__((aligned(64)));

static int* g_pType = NULL;
static int* g_pFree = NULL;
static int* g_pFormat = NULL;

static int  endParameter[48];
static char curDir[1024];
static mcTable g_fileInfoBuff;	// used by mcSetFileInfo and mcRename


// whether or not mc lib has been inited
static int g_mclibInited = 0;

// stores command currently being executed on the iop
static unsigned int g_currentCmd = 0;

// specifies whether using MCSERV or XMCSERV modules
static int g_mcType = MC_TYPE_MC;


// function that gets called when mcGetInfo ends
// and interrupts are disabled
static void mcGetInfoApdx(volatile int* info)
{
	volatile int* u_info = UNCACHED_SEG(info);
	
	if(g_pType	!= NULL)
	{
		*g_pType	= u_info[0];
	}
	
	if(g_pFree	!= NULL)
	{
		*g_pFree	= u_info[1];
	}
	
	if(g_pFormat!= NULL)
	{
		// older mcman doesnt support retrieving whether card is formatted
		// so if a card is present, we will always say its formatted
		if(g_mcType == MC_TYPE_MC)
		{
			if(u_info[0] == 0)
				*g_pFormat	= 0;
			else
				*g_pFormat	= 1;
		}
		else if(g_mcType == MC_TYPE_XMC)
		{
			*g_pFormat	= u_info[36];
		}
	}
}

// function that gets called when mcRead ends
// and interrupts are disabled
static void mcReadFixAlign(volatile int* data_raw)
{
	int	*ptr = (int*)UNCACHED_SEG(data_raw);
	u8	*src, *dest;
	int  i;
	
	dest = (u8*)ptr[2];
	src  = (u8*)(ptr + 4);
	for(i=0; i<ptr[0]; i++)
	{
		dest[i] = src[i];
	}
	
	dest = (u8*)ptr[3];
	if(g_mcType == MC_TYPE_MC)
		src  = (u8*)(ptr +  8);
	else
		src  = (u8*)(ptr + 20);
	for(i=0; i<ptr[1]; i++)
	{
		dest[i] = src[i];
	}
}

// function that gets called when mcChDir ends
// and interrupts are disabled
static void mcStoreDir(volatile int* arg)
{
	int len;
	int currentDir = (int)curDir | 0x20000000;
	len = strlen((char*)currentDir);
	if(len >= 1024)
		len = strlen((char*)(currentDir+1023));
	memcpy((void*)arg, (void*)currentDir, len);
	*(u8*)(currentDir+len) = 0;
}


// init memcard lib
// 
// args:	MC_TYPE_MC  = use MCSERV/MCMAN
//			MC_TYPE_XMC = use XMCSERV/XMCMAN
// returns:	0   = successful
//			< 0 = error
int mcInit(int type)
{
	int ret=0;

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
	}
	else if(g_mcType == MC_TYPE_XMC)
	{
#ifdef MC_DEBUG
		printf("libmc: using XMCMAN & XMCSERV\n");
#endif		
		
		// call init function
		if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_INIT], 0, &g_descParam, sizeof(g_descParam), g_rdata, 12, 0, 0)) < 0)
		{
			// init error
#ifdef MC_DEBUG
			printf("libmc: initialisation error\n");
#endif
			g_mclibInited = 0;
			return ret - 100;
		}
		
		// check if old version of mcserv loaded
		if(*(s32*)UNCACHED_SEG(g_rdata+4) < 0x205)
		{
#ifdef MC_DEBUG
			printf("libmc: mcserv is too old (%x)\n", *(s32*)UNCACHED_SEG(g_rdata+4));
#endif
			g_mclibInited = 0;
			return -120;
		}
		
		// check if old version of mcman loaded
		if(*(s32*)UNCACHED_SEG(g_rdata+8) < 0x206)
		{
#ifdef MC_DEBUG
			printf("libmc: mcman is too old (%x)\n", *(s32*)UNCACHED_SEG(g_rdata+8));
#endif
			g_mclibInited = 0;
			return -121;
		}
		ret = *(s32*)UNCACHED_SEG(g_rdata+0);
	}
	
	// successfully inited
	g_mclibInited = 1;
	g_currentCmd = 0;
	return ret;
}

// get memcard state
// mcSync result:	 0 = same card as last getInfo call
//					-1 = formatted card inserted since last getInfo call
//					-2 = unformatted card inserted since last getInfo call
//					< -2 = memcard access error (could be due to accessing psx memcard)
// 
// args:    port number
//          slot number
//          pointer to get memcard type
//          pointer to get number of free clusters
//          pointer to get whether or not the card is formatted
// returns:	0   = successful
//			< 0 = error
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
		g_descParam.param	= (int)endParameter;
	}
	else
	{
		g_descParam.port	= port;
		g_descParam.slot	= slot;
		g_descParam.size	= (format)	? 1 : 0;
		g_descParam.offset	= (free)	? 1 : 0;
		g_descParam.origin	= (type)	? 1 : 0;
		g_descParam.param	= (int)endParameter;
	}
	g_pType		= type;
	g_pFree		= free;
	g_pFormat	= format;
	SifWriteBackDCache(endParameter, 192);
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_GET_INFO], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, (SifRpcEndFunc_t)mcGetInfoApdx, endParameter)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_INFO;
	return ret;
}

// open a file on memcard
// mcSync returns:	0 or more = file descriptor (success)
//					< 0 = error
// 
// args:	port number
//			slot number
//			filename to open
//			open file mode (O_RDWR, O_CREAT, etc)
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_OPEN], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_OPEN;
	return ret;
}

// close an open file on memcard
// mcSync returns:	0 if closed successfully
//					< 0 = error
// 
// args:	file descriptor of open file
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_CLOSE], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CLOSE;
	return ret;
}

// move memcard file pointer
// mcSync returns:	0 or more = offset of file pointer from start of file
//					< 0 = error
// 
// args:	file descriptor
//			number of bytes from origin
//			initial position for offset
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_SEEK], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_SEEK;
	return ret;
}

// read from file on memcard
// mcSync returns:	0 or more = number of bytes read from memcard
//					< 0 = error
// 
// args:	file descriptor
//			buffer to read to
//			number of bytes to read
// returns:	0   = successful
//			< 0 = error
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
	g_descParam.buffer	= (int)buffer;
	g_descParam.param	= (int)endParameter;
	SifWriteBackDCache(buffer, size);
	SifWriteBackDCache(endParameter, 192);
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_READ], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, (SifRpcEndFunc_t)mcReadFixAlign, endParameter)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_READ;
	return ret;
}

// write to file on memcard
// mcSync returns:	0 or more = number of bytes written to memcard
//					< 0 = error
// 
// args:	file descriptor
//			buffer to write from write
// returns:	0   = successful
//			< 0 = error
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
		g_descParam.buffer	= (int)buffer + ( ((int)(buffer-1) & 0xFFFFFFF0) - (int)(buffer-16) );
	}
	for(i=0; i<g_descParam.origin; i++)
	{
		g_descParam.data[i] = *(char*)(buffer+i);
	}
	FlushCache(0);
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_WRITE], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_WRITE;
	return ret;
}

// flush file cache to memcard
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	file descriptor
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_FLUSH], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_FLUSH;
	return ret;
}

// create a dir
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	port number
//			slot number
//			directory name
// returns:	0   = successful
//			< 0 = error
int mcMkDir(int port, int slot, const char* name)
{
	int ret = mcOpen(port, slot, name, 0x40);
	
	if(ret != 0)
		g_currentCmd = MC_FUNC_MK_DIR;
	return ret;
}

// change current dir
// (can also get current dir)
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	port number
//			slot number
//			new dir to change to
//			buffer to get current dir (use 0 if not needed)
// returns:	0   = successful
//			< 0 = error
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
	g_nameParam.table		= (int)curDir;
	strncpy(g_nameParam.name, newDir, 1023);
	g_nameParam.name[1023] = 0;
	SifWriteBackDCache(curDir, 1024);
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_CH_DIR], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, (SifRpcEndFunc_t)mcStoreDir, currentDir)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CH_DIR;
	return ret;
}

// get memcard filelist
// mcSync result:	 0 or more = number of file entries obtained (success)
//					-2 = unformatted card
//					-4 = dirname error
// 
// args:    port number of memcard
//          slot number of memcard
//          filename to search for (can use wildcard and relative dirs)
//          mode: 0 = first call, otherwise = followup call
//          maximum number of entries to be written to filetable in 1 call
//          mc table array
// returns:	0   = successful
//			< 0 = error
int mcGetDir(int port, int slot, const char *name, unsigned mode, int maxent, mcTable* table)
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
	g_nameParam.table	= (int)table;
	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;
	SifWriteBackDCache(table, maxent * sizeof(mcTable));
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_GET_DIR], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_DIR;
   	return ret;
}

// change file information
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	port number
//			slot number
//			filename to access
//			data to be changed
//			flags to show which data is valid
// returns:	0   = successful
//			< 0 = error
int mcSetFileInfo(int port, int slot, const char* name, const mcTable* info, unsigned flags)
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
	g_nameParam.table	= (int)&g_fileInfoBuff;
	memcpy(&g_fileInfoBuff, info, sizeof(mcTable));
	
	strncpy(g_nameParam.name, name, 1023);
	g_nameParam.name[1023] = 0;
	FlushCache(0);
	
	// send sif command
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_SET_INFO], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_SET_INFO;
	return ret;
}

// delete file
// mcSync returns:	0 if deleted successfully
//					< 0 if error
// 
// args:	port number to delete from
//			slot number to delete from
//			filename to delete
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_DELETE], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_DELETE;
   	return ret;
}

// format memory card
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:    port number
//          slot number
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_FORMAT], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_FORMAT;
	return ret;
}

// unformat memory card
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:    port number
//          slot number
// returns:	0   = successful
//			< 0 = error
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
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_RPCCMD_UNFORMAT], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_UNFORMAT;
	return ret;
}

// get free space info
// mcSync returns:	0 or more = number of free entries (success)
//					< 0 if error
// 
// args:	port number
//			slot number
//			path to be checked
// returns:	0   = successful
//			< 0 = error
int mcGetEntSpace(int port, int slot, const char* path)
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
	strncpy(g_nameParam.name, path, 1023);
	g_nameParam.name[1023] = 0;
	
	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_FUNC_GET_ENT], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_GET_ENT;
	return ret;
}

// rename file or dir on memcard
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	port number
//			slot number
//			name of file/dir to rename
//			new name to give to file/dir
// returns:	0   = successful
//			< 0 = error
int mcRename(int port, int slot, const char* oldName, const char* newName)
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
	g_nameParam.flags = 0x10;
	g_nameParam.table = (int)&g_fileInfoBuff;
	strncpy(g_nameParam.name, oldName, 1023);
	g_nameParam.name[1023] = 0;
	strncpy(g_fileInfoBuff.name, newName, 31);
	g_fileInfoBuff.name[31] = 0;
	FlushCache(0);
	
	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_FUNC_SET_INFO], 1, &g_nameParam, sizeof(g_nameParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_RENAME;
	return ret;
}

// change mcserv thread priority
// (i dont think this is implemented properly)
// mcSync returns:	0 if ok
//					< 0 if error
// 
// args:	thread priority
// returns:	0   = successful
//			< 0 = error
int mcChangeThreadPriority(int level)
{
	int ret;
	
	// check lib is inited
	if(!g_mclibInited)
		return -1;
	// check nothing else is processing
	if(g_currentCmd != MC_FUNC_NONE)
		return g_currentCmd;
	
	// set global variables
//	*(u32*)mcCmd.name = level;
	
	// call sif function
	if((ret = SifCallRpc(&g_cdata, mcRpcCmd[g_mcType][MC_FUNC_CHG_PRITY], 1, &g_descParam, sizeof(g_descParam), g_rdata, 4, 0, 0)) != 0)
		return ret;
	g_currentCmd = MC_FUNC_CHG_PRITY;
	return ret;
}

// wait for mc functions to finish
// or check if they have finished yet
// 
// args:	mode: 0=wait till function finishes, 1=check function status
//			pointer for getting the number of the currently processing function (can be NULL)
//			pointer for getting result of completed function (if it does complete) (can be NULL)
// returns:	0  = function is still executing (mode=1)
//			1  = function has finished executing
//			-1 = no function registered
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
		*result = *(int*)g_rdata;
	
	return 1;
}

int mcReset()
{
	g_mclibInited = 0;
	g_cdata.server = NULL;
	return 0; 
}
