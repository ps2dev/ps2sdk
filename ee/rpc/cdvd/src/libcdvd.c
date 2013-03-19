/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
#     2003 loser (loser@internalreality.com)
# (c) 2004 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
#
# Function definitions for libsceCdvd (EE side calls to the iop module sceCdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#		
# NOTE: not all functions work with each set of modules!
*/

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <libcdvd.h>
#include <stdarg.h>

#include "internal.h"

// rpc bind values
#define CD_SERVER_INIT			0x80000592
#define CD_SERVER_SEARCHFILE		0x80000597
#define CD_SERVER_DISKREADY		0x8000059A
#define CD_SERVER_EXTRA			0x80000596	// XCDVDFSV only

// allows access to gp reg value from linker script
extern void *_gp;

// prototypes
void sceCdSemaExit(void);

// searchfile structure
typedef struct {
	u8 padding[32];
	char name[256];
	void *dest;
} SearchFilePkt;

#ifdef F__libcdvd_internals
// bind variables
s32 bindInit = -1;
s32 bindDiskReady = -1;
s32 bindSearchFile = -1;
// rpc binded client data
SifRpcClientData_t clientInit __attribute__ ((aligned(64)));	// for sceCdInit()
SifRpcClientData_t clientDiskReady __attribute__ ((aligned(64)));	// for sceCdDiskReady() (s-cmd)
SifRpcClientData_t clientSearchFile __attribute__ ((aligned(64)));	// for sceCdSearchFile() (n-cmd)

// set this to 1 or 2 to print sceCdvd debug messages
s32 sceCdDebug = 0;

// semaphore ids
s32 callbackSemaId = -1;	// callback semaphore id
volatile s32 cbSema = 0;	// callback semaphore variable (not a real semaphore)

// callbacks
volatile s32 sceCdCallbackNum __attribute__ ((aligned(64)));
volatile CdCBFunc sceCdCallbackFunc __attribute__ ((aligned(64)));

// threads
s32 sceCdThreadId = 0;
ee_thread_status_t sceCdThreadParam;
s32 callbackThreadId = 0;
ee_thread_t callbackThreadParam;

// current command variables

s32 diskReadyMode __attribute__ ((aligned(64)));
s32 trayReqData __attribute__ ((aligned(64)));
u32 initMode __attribute__ ((aligned(64)));

// searchfile stuff
SearchFilePkt searchFileSendBuff __attribute__ ((aligned(64)));
u32 searchFileRecvBuff __attribute__ ((aligned(64)));
#endif

// Prototypes for multimodule
extern s32 bindInit;
extern s32 bindDiskReady;
extern s32 bindSearchFile;
extern SifRpcClientData_t clientInit;
extern SifRpcClientData_t clientDiskReady;
extern SifRpcClientData_t clientSearchFile;
extern s32 sceCdDebug;
extern s32 callbackSemaId;
extern volatile s32 cbSema;
extern volatile s32 sceCdCallbackNum;
extern volatile CdCBFunc sceCdCallbackFunc;
extern s32 sceCdThreadId;
extern ee_thread_status_t sceCdThreadParam;
extern s32 callbackThreadId;
extern ee_thread_t callbackThreadParam;
extern s32 diskReadyMode;
extern s32 trayReqData;
extern u32 initMode;
extern SearchFilePkt searchFileSendBuff;
extern u32 searchFileRecvBuff;




// **** Other Functions ****


// init sceCdvd system
// 
// args:        init mode (CDVD_INIT_???)
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdInit
s32 sceCdInit(s32 mode)
{
	s32 i;

	if (sceCdSyncS(1))
		return 0;
	SifInitRpc(0);
	sceCdThreadId = GetThreadId();
	bindSearchFile = -1;
	bindNCmd = -1;
	bindSCmd = -1;
	bindDiskReady = -1;
	bindInit = -1;

	while (1) {
		if (SifBindRpc(&clientInit, CD_SERVER_INIT, 0) < 0) {
			if (sceCdDebug > 0)
				printf("LibsceCdvd bind err CD_Init\n");
		} else if (clientInit.server != 0)
			break;

		i = 0x10000;
		while (i--);
	}

	bindInit = 0;
	initMode = mode;
	if (SifCallRpc(&clientInit, 0, 0, &initMode, 4, 0, 0, 0, 0) < 0)
		return 0;
	if (mode == SCECdEXIT) {
		if (sceCdDebug > 0)
			printf("LibsceCdvd Exit\n");
		sceCdSemaExit();
		nCmdSemaId = -1;
		sCmdSemaId = -1;
		callbackSemaId = -1;
	} else {
		sceCdSemaInit();
	}

	return 1;
}
#endif

// convert from sector number to minute:second:frame
#ifdef F_sceCdIntToPos
sceCdlLOCCD *sceCdIntToPos(u32 i, sceCdlLOCCD * p)
{
	p->minute = (((((i + 150) / 75) / 60) / 10) * 16) + ((((i + 150) / 75) / 60) % 10);
	p->second = (((((i + 150) / 75) % 60) / 10) * 16) + ((((i + 150) / 75) % 60) % 10);
	p->sector = ((((i + 150) % 75) / 10) * 16) + (((i + 150) % 75) % 10);
	return p;
}
#endif

// convert from minute:second:frame to sector number
#ifdef F_sceCdPosToInt
u32 sceCdPosToInt(sceCdlLOCCD * p)
{
	return ((((p->minute / 16) * 10) + (p->minute & 0xF)) * 60 + ((p->second / 16) * 10) + (p->second & 0xF)
	    ) * 75 + (p->sector / 16) * 10 + (p->sector & 0xF) - 150;
}
#endif


// search for a file on disc
// 
// args:        file structure to get file info in
//                      name of file to search for (no wildcard characters)
//                              (should be in the form '\\SYSTEM.CNF;1')
// returns:     1 if successful
//                      0 if error (or no file found)
#ifdef F_sceCdSearchFile
s32 sceCdSearchFile(sceCdlFILE * file, const char *name)
{
	s32 i;

	sceCdSemaInit();
	if (PollSema(nCmdSemaId) != nCmdSemaId)
		return 0;
	nCmdNum = CD_SERVER_SEARCHFILE;
	ReferThreadStatus(sceCdThreadId, &sceCdThreadParam);
	if (sceCdSync(1)) {
		SignalSema(nCmdSemaId);
		return 0;
	}
	SifInitRpc(0);
	if (bindSearchFile < 0) {
		while (1) {
			if (SifBindRpc(&clientSearchFile, CD_SERVER_SEARCHFILE, 0) < 0) {
				if (sceCdDebug > 0)
					printf("libsceCdvd bind err sceCdSearchFile\n");
			}
			if (clientSearchFile.server != 0)
				break;

			i = 0x10000;
			while (i--);
		}
		bindSearchFile = 0;
	}

	strncpy(searchFileSendBuff.name, name, 255);
	searchFileSendBuff.name[255] = '\0';
	searchFileSendBuff.dest = &searchFileSendBuff;

	if (sceCdDebug > 0)
		printf("ee call cmd search %s\n", searchFileSendBuff.name);
	if (SifCallRpc(&clientSearchFile, 0, 0, &searchFileSendBuff, sizeof(SearchFilePkt), nCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	memcpy(file, UNCACHED_SEG(&searchFileSendBuff), 32);

	if (sceCdDebug > 0) {
		printf("search name %s\n", file->name);
		printf("search size %d\n", file->size);
		printf("search loc lnn %d\n", file->lsn);
		printf("search loc date %02X %02X %02X %02X %02X %02X %02X %02X\n",
		       file->date[0], file->date[1], file->date[2], file->date[3],
		       file->date[4], file->date[5], file->date[6], file->date[7]);
		printf("search loc date %02d %02d %02d %02d %02d %02d %02d %02d\n",
		       file->date[0], file->date[1], file->date[2], file->date[3],
		       file->date[4], file->date[5], file->date[6], file->date[7]);
	}

	SignalSema(nCmdSemaId);
//	return 1;
	return *(s32*)UNCACHED_SEG(nCmdRecvBuff);
}
#endif

// checks if drive is ready
// 
// args:         mode
// returns:     CDVD_READY_READY if ready
//                      SCECdNotReady if busy
#ifdef F_sceCdDiskReady
s32 sceCdDiskReady(s32 mode)
{
	s32 i;

	if (sceCdDebug > 0)
		printf("DiskReady 0\n");

	sceCdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId)
		return SCECdNotReady;
	if (sceCdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return SCECdNotReady;
	}

	SifInitRpc(0);
	if (bindDiskReady < 0) {
		while (1) {
			if (SifBindRpc(&clientDiskReady, CD_SERVER_DISKREADY, 0) < 0) {
				if (sceCdDebug > 0)
					printf("LibsceCdvd bind err CdDiskReady\n");
			}
			if (clientDiskReady.server != 0)
				break;

			i = 0x10000;
			while (i--);
		}
	}
	bindDiskReady = 0;
	diskReadyMode = mode;

	if (SifCallRpc(&clientDiskReady, 0, 0, &diskReadyMode, 4, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 6;
	}
	if (sceCdDebug > 0)
		printf("DiskReady ended\n");

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

#ifdef F_sceCdSemaInit
void sceCdSemaInit(void)
{
	struct t_ee_sema semaParam;

	// return if both semaphores are already inited
	if (nCmdSemaId != -1 && sCmdSemaId != -1)
		return;

	semaParam.init_count = 1;
	semaParam.max_count = 1;
	semaParam.option = 0;
	nCmdSemaId = CreateSema(&semaParam);
	sCmdSemaId = CreateSema(&semaParam);

	semaParam.init_count = 0;
	callbackSemaId = CreateSema(&semaParam);

	cbSema = 0;
}
#endif

#ifdef F_sceCdSemaExit
void sceCdSemaExit(void)
{
	if (callbackThreadId) {
		sceCdCallbackNum = -1;
		SignalSema(callbackSemaId);
	}
	DeleteSema(nCmdSemaId);
	DeleteSema(sCmdSemaId);
	DeleteSema(callbackSemaId);
}
#endif

// initialise callback thread
// 
// args:        callback thread priority
//                      callback thread stack address (16 byte aligned)
//                      callback thread stack size
// returns:     1 if initialised callback
//                      0 if only priority was changed
#ifdef F_sceCdInitCallbackThread
static void sceCdCallbackLoop(void);
s32 sceCdInitCallbackThread(s32 priority, void *stackAddr, s32 stackSize)
{
	// if callback thread has already been initialised, just change its priority
	if (callbackThreadId != 0) {
		ChangeThreadPriority(callbackThreadId, priority);
		return 0;
	}
	// initialise callback thread
	sceCdThreadId = GetThreadId();
	ReferThreadStatus(sceCdThreadId, &sceCdThreadParam);
	callbackThreadParam.stack_size = stackSize;
	callbackThreadParam.gp_reg = &_gp;
	callbackThreadParam.func = sceCdCallbackLoop;
	callbackThreadParam.stack = stackAddr;
	callbackThreadParam.initial_priority = priority;
	callbackThreadId = CreateThread(&callbackThreadParam);
	StartThread(callbackThreadId, 0);

	return 1;
}
#endif

// sets the sceCd callback function
// gets called when the following functions complete:
//    sceCdSeek, sceCdStandby, sceCdStop, sceCdPause, sceCdRead
// 
// args:        pointer to new callback function (or null)
// returns:     pointer to old function
#ifdef F_sceCdCallback
CdCBFunc sceCdCallback(CdCBFunc newFunc)
{
	CdCBFunc oldFunc = sceCdCallbackFunc;

	if (sceCdSync(1))
		return 0;

	sceCdCallbackFunc = newFunc;
	return oldFunc;
}
#endif

// **** Util Functions ****


// callback loop thread
// once callbacks have been inited using sceCdInitCallbackThread()
// this function continually loops until a callback with function
// number '-1' is generated
#ifdef F_sceCdInitCallbackThread
static void sceCdCallbackLoop(void)
{
	while (1) {
		WaitSema(callbackSemaId);

		// if callback number if -1, stop callbck thread loop
		if (sceCdCallbackNum == -1)
			ExitThread();

		if (sceCdDebug > 0)
			printf("sceCdCallbackFunc = %08X   sceCdCallbackNum = %d\n", (u32) sceCdCallbackFunc, sceCdCallbackNum);

		// if callback function number and 'custom callback function' pointer are valid, do callback
		if (sceCdCallbackFunc && sceCdCallbackNum)
			sceCdCallbackFunc(sceCdCallbackNum);

		cbSema = 0;
	}
}
#endif

// generic sceCd callback function
#ifdef F_sceCdGenericCallbackFunction
void sceCdGenericCallbackFunction(void *funcNum)
{
	// set the currently executing function num
	sceCdCallbackNum = *(s32*) funcNum;
	iSignalSema(nCmdSemaId);
	
	// check if user callback is registered
	if (callbackThreadId)
	{
		if (sceCdCallbackFunc)
		{
			iSignalSema(callbackSemaId);
			return;
		}
	}
	
	// clear the currently executing function num
	cbSema = 0;
	sceCdCallbackNum = 0;

}
#endif
