/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
                               2003 loser (loser@internalreality.com)
			   (c) 2004 Marcus R. Brown <mrbrown@0xd6.org>
  ------------------------------------------------------------------------
  libcdvd.c
  		Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
		
		NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
		modules stored in rom0.
		
		NOTE: not all functions work with each set of modules!
*/

#include "tamtypes.h"
#include "kernel.h"
#include "sifrpc.h"
#include "string.h"
#include "libcdvd.h"
#include <stdarg.h>

#include "internal.h"

// rpc bind values
#define CD_SERVER_INIT			0x80000592
#define CD_SERVER_SEARCHFILE	0x80000597
#define CD_SERVER_DISKREADY		0x8000059A
#define CD_SERVER_EXTRA			0x80000596	// XCDVDFSV only


// set this to 1 or 2 to print cdvd debug messages
s32 cdDebug = 0;

// allows access to gp reg value from linker script
extern void *_gp;

// prototypes
void cdSemaExit(void);
static void cdCallbackLoop(void);

// bind variables
s32 bindInit = -1;
s32 bindDiskReady = -1;
s32 bindSearchFile = -1;
// rpc binded client data
SifRpcClientData_t clientInit __attribute__ ((aligned(64)));	// for cdInit()
SifRpcClientData_t clientDiskReady __attribute__ ((aligned(64)));	// for cdDiskReady() (s-cmd)
SifRpcClientData_t clientSearchFile __attribute__ ((aligned(64)));	// for cdSearchFile() (n-cmd)

// semaphore ids
s32 callbackSemaId = -1;	// callback semaphore id
volatile s32 cbSema = 0;	// callback semaphore variable (not a real semaphore)

// callbacks
volatile s32 cdCallbackNum __attribute__ ((aligned(64)));
volatile CdCBFunc cdCallbackFunc __attribute__ ((aligned(64)));

// threads
s32 cdThreadId = 0;
ee_thread_t cdThreadParam;
s32 callbackThreadId = 0;
ee_thread_t callbackThreadParam;

// current command variables

s32 diskReadyMode __attribute__ ((aligned(64)));
s32 trayReqData __attribute__ ((aligned(64)));
u32 initMode __attribute__ ((aligned(64)));

// searchfile stuff
typedef struct {
	u8 padding[32];
	char name[256];
	void *dest;
} SearchFilePkt;
SearchFilePkt searchFileSendBuff __attribute__ ((aligned(64)));
u32 searchFileRecvBuff __attribute__ ((aligned(64)));





// **** Other Functions ****


// init cdvd system
// 
// args:        init mode (CDVD_INIT_???)
// returns:     1 if successful
//                      0 if error
s32 cdInit(s32 mode)
{
	s32 i;

	if (cdSyncS(1))
		return 0;
	SifInitRpc(0);
	cdThreadId = GetThreadId();
	bindSearchFile = -1;
	bindNCmd = -1;
	bindSCmd = -1;
	bindDiskReady = -1;
	bindInit = -1;

	while (1) {
		if (SifBindRpc(&clientInit, CD_SERVER_INIT, 0) < 0) {
			if (cdDebug > 0)
				printf("Libcdvd bind err CD_Init\n");
		} else if (clientInit.server != 0)
			break;

		i = 0x10000;
		while (i--);
	}

	bindInit = 0;
	initMode = mode;
	SifWriteBackDCache(&initMode, 4);
	if (SifCallRpc(&clientInit, 0, 0, &initMode, 4, 0, 0, 0, 0) < 0)
		return 0;
	if (mode == CDVD_INIT_EXIT) {
		if (cdDebug > 0)
			printf("Libcdvd Exit\n");
		cdSemaExit();
		nCmdSemaId = -1;
		sCmdSemaId = -1;
		callbackSemaId = -1;
	} else {
		cdSemaInit();
	}

	return 1;
}


// convert from sector number to minute:second:frame
CdvdLocation_t *cdIntToPos(s32 i, CdvdLocation_t * p)
{
	p->minute = (((((i + 150) / 75) / 60) / 10) * 16) + ((((i + 150) / 75) / 60) % 10);
	p->second = (((((i + 150) / 75) % 60) / 10) * 16) + ((((i + 150) / 75) % 60) % 10);
	p->sector = ((((i + 150) % 75) / 10) * 16) + (((i + 150) % 75) % 10);
	return p;
}

// convert from minute:second:frame to sector number
s32 cdPosToInt(CdvdLocation_t * p)
{
	return ((((p->minute / 16) * 10) + (p->minute & 0xF)) * 60 + ((p->second / 16) * 10) + (p->second & 0xF)
	    ) * 75 + (p->sector / 16) * 10 + (p->sector & 0xF) - 150;
}


// search for a file on disc
// 
// args:        file structure to get file info in
//                      name of file to search for (no wildcard characters)
//                              (should be in the form '\\SYSTEM.CNF;1')
// returns:     1 if successful
//                      0 if error (or no file found)
s32 cdSearchFile(CdvdFileSpec_t * file, const char *name)
{
	s32 i;

	cdSemaInit();
	if (PollSema(nCmdSemaId) != nCmdSemaId)
		return 0;
	nCmdNum = CD_SERVER_SEARCHFILE;
	ReferThreadStatus(cdThreadId, &cdThreadParam);
	if (cdSync(1)) {
		SignalSema(nCmdSemaId);
		return 0;
	}
	SifInitRpc(0);
	if (bindSearchFile < 0) {
		while (1) {
			if (SifBindRpc(&clientSearchFile, CD_SERVER_SEARCHFILE, 0) < 0) {
				if (cdDebug > 0)
					printf("libcdvd bind err cdSearchFile\n");
			}
			if (clientSearchFile.server != 0)
				break;

			i = 0x10000;
			while (i--);
		}
		bindSearchFile = 0;
	}

	strncpy(searchFileSendBuff.name, name, 256);
	searchFileSendBuff.name[255] = '\0';
	searchFileSendBuff.dest = &searchFileSendBuff;

	if (cdDebug > 0)
		printf("ee call cmd search %s\n", searchFileSendBuff.name);
	SifWriteBackDCache(&searchFileSendBuff, sizeof(SearchFilePkt));
	if (SifCallRpc(&clientSearchFile, 0, 0, &searchFileSendBuff, sizeof(SearchFilePkt), nCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	memcpy(file, UNCACHED_SEG(&searchFileSendBuff), 32);

	if (cdDebug > 0) {
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

// checks if drive is ready
// 
// args:         mode
// returns:     CDVD_READY_READY if ready
//                      CDVD_READY_NOTREADY if busy
s32 cdDiskReady(s32 mode)
{
	s32 i;

	if (cdDebug > 0)
		printf("DiskReady 0\n");

	cdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId)
		return CDVD_READY_NOTREADY;
	if (cdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return CDVD_READY_NOTREADY;
	}

	SifInitRpc(0);
	if (bindDiskReady < 0) {
		while (1) {
			if (SifBindRpc(&clientDiskReady, CD_SERVER_DISKREADY, 0) < 0) {
				if (cdDebug > 0)
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
	SifWriteBackDCache(&diskReadyMode, 4);

	if (SifCallRpc(&clientDiskReady, 0, 0, &diskReadyMode, 4, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 6;
	}
	if (cdDebug > 0)
		printf("DiskReady ended\n");

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}


void cdSemaInit(void)
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

void cdSemaExit(void)
{
	if (callbackThreadId) {
		cdCallbackNum = -1;
		SignalSema(callbackSemaId);
	}
	DeleteSema(nCmdSemaId);
	DeleteSema(sCmdSemaId);
	DeleteSema(callbackSemaId);
}

// initialise callback thread
// 
// args:        callback thread priority
//                      callback thread stack address (16 byte aligned)
//                      callback thread stack size
// returns:     1 if initialised callback
//                      0 if only priority was changed
s32 cdInitCallbackThread(s32 priority, void *stackAddr, s32 stackSize)
{
	// if callback thread has already been initialised, just change its priority
	if (callbackThreadId != 0) {
		ChangeThreadPriority(callbackThreadId, priority);
		return 0;
	}
	// initialise callback thread
	cdThreadId = GetThreadId();
	ReferThreadStatus(cdThreadId, &cdThreadParam);
	callbackThreadParam.stack_size = stackSize;
	callbackThreadParam.gp_reg = &_gp;
	callbackThreadParam.func = cdCallbackLoop;
	callbackThreadParam.stack = stackAddr;
	callbackThreadParam.initial_priority = priority;
	callbackThreadId = CreateThread(&callbackThreadParam);
	StartThread(callbackThreadId, 0);

	return 1;
}

// sets the cd callback function
// gets called when the following functions complete:
//    cdSeek, cdStandby, cdStop, cdPause, cdRead
// 
// args:        pointer to new callback function (or null)
// returns:     pointer to old function
CdCBFunc cdSetCallback(CdCBFunc newFunc)
{
	CdCBFunc oldFunc = cdCallbackFunc;

	if (cdSync(1))
		return 0;

	cdCallbackFunc = newFunc;
	return oldFunc;
}


// **** Util Functions ****


// callback loop thread
// once callbacks have been inited using cdInitCallbackThread()
// this function continually loops until a callback with function
// number '-1' is generated
static void cdCallbackLoop(void)
{
	while (1) {
		WaitSema(callbackSemaId);

		// if callback number if -1, stop callbck thread loop
		if (cdCallbackNum == -1)
			ExitThread();

		if (cdDebug > 0)
			printf("cdCallbackFunc = %08X   cdCallbackNum = %d\n", (u32) cdCallbackFunc, cdCallbackNum);

		// if callback function number and 'custom callback function' pointer are valid, do callback
		if (cdCallbackFunc && cdCallbackNum)
			cdCallbackFunc(cdCallbackNum);

		cbSema = 0;
	}
}

// generic cd callback function
void cdCallback(void *funcNum)
{
	// set the currently executing function num
	cdCallbackNum = *(s32*) funcNum;
	iSignalSema(nCmdSemaId);
	
	// check if user callback is registered
	if (callbackThreadId)
	{
		if (cdCallbackFunc)
		{
			iSignalSema(callbackSemaId);
			return;
		}
	}
	
	// clear the currently executing function num
	cbSema = 0;
	cdCallbackNum = 0;

}
