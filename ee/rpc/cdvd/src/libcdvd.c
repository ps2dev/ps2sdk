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
*/

/**
 * @file
 * Function definitions for libsceCdvd (EE side calls to the iop module sceCdvdfsv).
 *
 * NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
 * modules stored in rom0.
 *		
 * NOTE: not all functions work with each set of modules!
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
/** XCDVDFSV only */
#define CD_SERVER_POFF			0x80000596

/** allows access to gp reg value from linker script */
extern void *_gp;

// prototypes
void _CdSemaExit(void);

/** searchfile structure */
typedef struct {
	u8 padding[32];
	char name[256];
	void *dest;
} SearchFilePkt;

#ifdef F__libcdvd_internals
// bind variables
int bindInit = -1;
int bindDiskReady = -1;
int bindSearchFile = -1;
// rpc binded client data
/** for sceCdInit() */
SifRpcClientData_t clientInit __attribute__ ((aligned(64)));
/** for sceCdDiskReady() (s-cmd) */
SifRpcClientData_t clientDiskReady __attribute__ ((aligned(64)));
/** for sceCdSearchFile() (n-cmd) */
SifRpcClientData_t clientSearchFile __attribute__ ((aligned(64)));

/** set this to 1 or 2 to print sceCdvd debug messages */
int CdDebug = 0;

// semaphore ids
/** callback semaphore id */
int callbackSemaId = -1;
/** callback semaphore variable (not a real semaphore) */
volatile int cbSema = 0;

// callbacks
volatile int CdCallbackNum;
volatile sceCdCBFunc sceCdCallbackFunc;

// threads
int CdThreadId = 0;
ee_thread_status_t CdThreadParam;
int callbackThreadId = 0;
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
extern int bindInit;
extern int bindDiskReady;
extern int bindSearchFile;
extern SifRpcClientData_t clientInit;
extern SifRpcClientData_t clientDiskReady;
extern SifRpcClientData_t clientSearchFile;
extern int CdDebug;
extern int callbackSemaId;
extern volatile int cbSema;
extern volatile int CdCallbackNum;
extern volatile sceCdCBFunc sceCdCallbackFunc;
extern int CdThreadId;
extern ee_thread_status_t CdThreadParam;
extern int callbackThreadId;
extern ee_thread_t callbackThreadParam;
extern s32 diskReadyMode;
extern s32 trayReqData;
extern u32 initMode;
extern SearchFilePkt searchFileSendBuff;
extern u32 searchFileRecvBuff;

/* Other Functions */

#ifdef F_sceCdInit
s32 sceCdInit(s32 mode)
{
	int i;

	if (_CdSyncS(1))
		return 0;
	SifInitRpc(0);
	CdThreadId = GetThreadId();
	bindSearchFile = -1;
	bindNCmd = -1;
	bindSCmd = -1;
	bindDiskReady = -1;
	bindInit = -1;

	while (1) {
		if (SifBindRpc(&clientInit, CD_SERVER_INIT, 0) < 0) {
			if (CdDebug > 0)
				printf("Libcdvd bind err CD_Init\n");
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
		if (CdDebug > 0)
			printf("Libcdvd Exit\n");
		_CdSemaExit();
		nCmdSemaId = -1;
		sCmdSemaId = -1;
		callbackSemaId = -1;
	} else {
		_CdSemaInit();
	}

	return 1;
}
#endif

#ifdef F_sceCdIntToPos
sceCdlLOCCD *sceCdIntToPos(u32 i, sceCdlLOCCD * p)
{
	p->minute = (((((i + 150) / 75) / 60) / 10) * 16) + ((((i + 150) / 75) / 60) % 10);
	p->second = (((((i + 150) / 75) % 60) / 10) * 16) + ((((i + 150) / 75) % 60) % 10);
	p->sector = ((((i + 150) % 75) / 10) * 16) + (((i + 150) % 75) % 10);
	return p;
}
#endif

#ifdef F_sceCdPosToInt
u32 sceCdPosToInt(sceCdlLOCCD * p)
{
	return ((((p->minute / 16) * 10) + (p->minute & 0xF)) * 60 + ((p->second / 16) * 10) + (p->second & 0xF)
	    ) * 75 + (p->sector / 16) * 10 + (p->sector & 0xF) - 150;
}
#endif

#ifdef F_sceCdSearchFile
s32 sceCdSearchFile(sceCdlFILE * file, const char *name)
{
	int i;

	_CdSemaInit();
	if (PollSema(nCmdSemaId) != nCmdSemaId)
		return 0;
	nCmdNum = CD_SERVER_SEARCHFILE;
	ReferThreadStatus(CdThreadId, &CdThreadParam);
	if (sceCdSync(1)) {
		SignalSema(nCmdSemaId);
		return 0;
	}
	SifInitRpc(0);
	if (bindSearchFile < 0) {
		while (1) {
			if (SifBindRpc(&clientSearchFile, CD_SERVER_SEARCHFILE, 0) < 0) {
				if (CdDebug > 0)
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

	if (CdDebug > 0)
		printf("ee call cmd search %s\n", searchFileSendBuff.name);
	if (SifCallRpc(&clientSearchFile, 0, 0, &searchFileSendBuff, sizeof(SearchFilePkt), nCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	memcpy(file, UNCACHED_SEG(&searchFileSendBuff), 32);

	if (CdDebug > 0) {
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
	return *(s32*)UNCACHED_SEG(nCmdRecvBuff);
}
#endif

#ifdef F_sceCdDiskReady
s32 sceCdDiskReady(s32 mode)
{
	int i;

	if (CdDebug > 0)
		printf("DiskReady 0\n");

	_CdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId)
		return SCECdNotReady;
	if (_CdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return SCECdNotReady;
	}

	SifInitRpc(0);
	if (bindDiskReady < 0) {
		while (1) {
			if (SifBindRpc(&clientDiskReady, CD_SERVER_DISKREADY, 0) < 0) {
				if (CdDebug > 0)
					printf("Libcdvd bind err CdDiskReady\n");
			}
			if (clientDiskReady.server != 0)
				break;

			i = 0x10000;
			while (i--);
		}
	}
	bindDiskReady = 0;
	diskReadyMode = mode;

	if (SifCallRpc(&clientDiskReady, 0, 0, &diskReadyMode, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 6;
	}
	if (CdDebug > 0)
		printf("DiskReady ended\n");

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

#ifdef F__CdSemaInit
void _CdSemaInit(void)
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

#ifdef F__CdSemaExit
void _CdSemaExit(void)
{
	if (callbackThreadId) {
		CdCallbackNum = -1;
		SignalSema(callbackSemaId);
	}
	DeleteSema(nCmdSemaId);
	DeleteSema(sCmdSemaId);
	DeleteSema(callbackSemaId);
}
#endif

#ifdef F_sceCdInitEeCB
static void _CdCallbackLoop(void);
s32 sceCdInitEeCB(s32 priority, void *stackAddr, s32 stackSize)
{
	// if callback thread has already been initialised, just change its priority
	if (callbackThreadId != 0) {
		ChangeThreadPriority(callbackThreadId, priority);
		return 0;
	}
	// initialise callback thread
	CdThreadId = GetThreadId();
	ReferThreadStatus(CdThreadId, &CdThreadParam);
	callbackThreadParam.stack_size = stackSize;
	callbackThreadParam.gp_reg = &_gp;
	callbackThreadParam.func = &_CdCallbackLoop;
	callbackThreadParam.stack = stackAddr;
	callbackThreadParam.initial_priority = priority;
	callbackThreadId = CreateThread(&callbackThreadParam);
	StartThread(callbackThreadId, NULL);

	return 1;
}
#endif

#ifdef F_sceCdCallback
sceCdCBFunc sceCdCallback(sceCdCBFunc newFunc)
{
	sceCdCBFunc oldFunc = sceCdCallbackFunc;

	if (sceCdSync(1))
		return 0;

	sceCdCallbackFunc = newFunc;
	return oldFunc;
}
#endif

/* Util Functions */


/** callback loop thread
 * once callbacks have been inited using sceCdInitCallbackThread()
 * this function continually loops until a callback with function
 * number '-1' is generated
 */
#ifdef F_sceCdInitEeCB
static void _CdCallbackLoop(void)
{
	while (1) {
		WaitSema(callbackSemaId);

		// if callback number if -1, stop callbck thread loop
		if (CdCallbackNum == -1)
			ExitThread();

		if (CdDebug > 0)
			printf("sceCdCallbackFunc = %08X   CdCallbackNum = %d\n", (u32) sceCdCallbackFunc, CdCallbackNum);

		// if callback function number and 'custom callback function' pointer are valid, do callback
		if (sceCdCallbackFunc && CdCallbackNum)
			sceCdCallbackFunc(CdCallbackNum);

		cbSema = 0;
	}
}
#endif

/** generic callback function */
#ifdef F__CdGenericCallbackFunction
void _CdGenericCallbackFunction(void *funcNum)
{
	// set the currently executing function num
	CdCallbackNum = *(s32*) funcNum;
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
	CdCallbackNum = 0;
}
#endif
