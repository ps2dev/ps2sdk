/*
  _____     ___ ____
   ____|   |    ____|      PS2 OpenSource Project
  |     ___|   |____       (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
                               2003 loser (loser@internalreality.com)
			   (c) 2004 Marcus R. Brown <mrbrown@0xd6.org>
  ------------------------------------------------------------------------
  scmd.c
  		Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
		
		NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
		modules stored in rom0.
		
		NOTE: not all functions work with each set of modules!
*/

#include "kernel.h"
#include "sifrpc.h"
#include "libcdvd.h"
#include "string.h"

#include "internal.h"

#define CD_SERVER_SCMD			0x80000593	// blocking commands (Synchronous)

#define CD_SCMD_READCLOCK		0x01
#define CD_SCMD_WRITECLOCK		0x02
#define CD_SCMD_GETDISCTYPE		0x03
#define CD_SCMD_GETERROR		0x04
#define CD_SCMD_TRAYREQ			0x05
#define CD_SCMD_READILINKID		0x06
#define CD_SCMD_WRITEILINKID	0x07
#define CD_SCMD_READNVM			0x08
#define CD_SCMD_WRITENVM		0x09
#define CD_SCMD_DECSET			0x0A
#define CD_SCMD_SCMD			0x0B
#define CD_SCMD_STATUS			0x0C
#define CD_SCMD_SETHDMODE		0x0D
#define CD_SCMD_OPENCONFIG		0x0E
#define CD_SCMD_CLOSECONFIG		0x0F
#define CD_SCMD_READCONFIG		0x10
#define CD_SCMD_WRITECONFIG		0x11
#define CD_SCMD_READCONSOLEID	0x12
#define CD_SCMD_WRITECONSOLEID	0x13
#define CD_SCMD_GETMECHAVERSION	0x14
#define CD_SCMD_CTRLAUDIOOUT	0x15
#define CD_SCMD_BREAK			0x16
#define CD_SCMD_READSUBQ		0x17
#define CD_SCMD_FORBIDDVDP		0x18
#define CD_SCMD_AUTOADUJSTCTRL	0x19
#define CD_SCMD_RM				0x1A	// XCDVDFSV only
#define CD_SCMD_WM				0x1B	// XCDVDFSV only
#define CD_SCMD_FORBIDREAD		0x1C	// XCDVDFSV only
#define CD_SCMD_SC				0x1D	// XCDVDFSV only
#define CD_SCMD_BOOTCERTIFY		0x1E	// XCDVDFSV only
#define CD_SCMD_CANCELPOWEROFF	0x1F	// XCDVDFSV only
#define CD_SCMD_BLUELEDCTRL		0x20	// XCDVDFSV only
#define CD_SCMD_POWEROFF		0x21	// XCDVDFSV only
#define CD_SCMD_MMODE			0x22	// XCDVDFSV only
#define CD_SCMD_SETTHREADPRI	0x23	// XCDVDFSV only

s32 bindSCmd = -1;

SifRpcClientData_t clientSCmd __attribute__ ((aligned(64)));	// for s-cmds

s32 sCmdSemaId = -1;		// s-cmd semaphore id

u8 sCmdRecvBuff[48] __attribute__ ((aligned(64)));
u8 sCmdSendBuff[48] __attribute__ ((aligned(64)));

u32 configSize = 0;
u8 configBuff[1032] __attribute__ ((aligned(64)));

s32 sCmdNum = 0;

s32 cdCheckSCmd(s32 cmd);

// **** S-Command Functions ****


// read clock value from ps2s clock
// 
// args:        time/date struct
// returns:     1 if successful
//                      0 if error
s32 cdReadClock(CdvdClock_t * clock)
{
	if (cdCheckSCmd(CD_SCMD_READCLOCK) == 0)
		return 0;

	if (cdDebug > 0)
		printf("Libcdvd call Clock read 1\n");

	if (SifCallRpc(&clientSCmd, CD_SCMD_READCLOCK, 0, 0, 0, sCmdRecvBuff, 16, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy(clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);

	if (cdDebug > 0)
		printf("Libcdvd call Clock read 2\n");

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// write clock value to ps2s clock
// 
// args:        time/date struct to set clocks time with
// returns:     1 if successful
//                      0 if error
s32 cdWriteClock(const CdvdClock_t * clock)
{
	if (cdCheckSCmd(CD_SCMD_WRITECLOCK) == 0)
		return 0;

	memcpy(sCmdSendBuff, clock, 8);
	SifWriteBackDCache(sCmdSendBuff, 8);

	if (SifCallRpc(&clientSCmd, CD_SCMD_WRITECLOCK, 0, sCmdSendBuff, 8, sCmdRecvBuff, 16, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy((CdvdClock_t *)clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// gets the type of the currently inserted disc
// 
// returns:     disk type (CDVD_TYPE_???)
CdvdDiscType_t cdGetDiscType(void)
{
	if (cdCheckSCmd(CD_SCMD_GETDISCTYPE) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETDISCTYPE, 0, 0, 0, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// gets the last error that occurred
// 
// returns:     error type (CDVD_ERR_???)
s32 cdGetError(void)
{
	if (cdCheckSCmd(CD_SCMD_GETERROR) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETERROR, 0, 0, 0, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// open/close/check disk tray
// 
// args:        param (CDVD_TRAY_???)
//                      address for returning tray state change
// returns:     1 if successful
//                      0 if error
s32 cdTrayReq(s32 param, u32 * traychk)
{
	if (cdCheckSCmd(CD_SCMD_TRAYREQ) == 0)
		return 0;

	memcpy(sCmdSendBuff, &param, 4);
	SifWriteBackDCache(sCmdSendBuff, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_TRAYREQ, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (traychk)
		*traychk = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// send an s-command by function number
// 
// args:        command number
//                      input buffer  (can be null)
//                      size of input buffer  (0 - 16 byte)
//                      output buffer (can be null)
//                      size of output buffer (0 - 16 bytes)
// returns:     1 if successful
//                      0 if error
s32 cdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize)
{
	if (cdCheckSCmd(CD_SCMD_SCMD) == 0)
		return 0;

	*(u16 *) & sCmdSendBuff[0] = cmdNum;
	*(u16 *) & sCmdSendBuff[2] = inBuffSize;
	memset(&sCmdSendBuff[4], 0, 16);
	if (inBuff)
		memcpy(&sCmdSendBuff[4], inBuff, inBuffSize);
	SifWriteBackDCache(sCmdSendBuff, 20);

	if (SifCallRpc(&clientSCmd, CD_SCMD_SCMD, 0, sCmdSendBuff, 20, sCmdRecvBuff, 16, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (outBuff)
		memcpy(outBuff, UNCACHED_SEG(sCmdRecvBuff), outBuffSize);
	SignalSema(sCmdSemaId);
	return 1;
}

// gets the status of the cd system
// 
// returns:     status (CDVD_STAT_???)
s32 cdStatus(void)
{
	if (cdCheckSCmd(CD_SCMD_STATUS) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_STATUS, 0, 0, 0, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	if (cdDebug >= 2)
		printf("status called\n");

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// 'breaks' the currently executing command
// 
// returns:     1 if successful
//                      0 if error
s32 cdBreak(void)
{
	if (cdCheckSCmd(CD_SCMD_BREAK) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_BREAK, 0, 0, 0, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// cancel power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        result
// returns:     1 if successful
//                      0 if error
s32 cdCancelPowerOff(u32 * result)
{
	if (cdCheckSCmd(CD_SCMD_CANCELPOWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_CANCELPOWEROFF, 0, 0, 0, sCmdRecvBuff, 8, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// blue led control
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        control value
//                      result
// returns:     1 if successful
//                      0 if error
s32 cdBlueLedCtrl(u8 control, u32 * result)
{
	if (cdCheckSCmd(CD_SCMD_BLUELEDCTRL) == 0)
		return 0;

	*(u32 *) sCmdSendBuff = control;
	SifWriteBackDCache(sCmdSendBuff, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_BLUELEDCTRL, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        result
// returns:     1 if successful
//                      0 if error
s32 cdPowerOff(u32 * result)
{
	if (cdCheckSCmd(CD_SCMD_POWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_POWEROFF, 0, 0, 0, sCmdRecvBuff, 8, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// set media mode
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        media mode
// returns:     1 if successful
//                      0 if error
s32 cdSetMediaMode(u32 mode)
{
	if (cdCheckSCmd(CD_SCMD_MMODE) == 0)
		return 0;

	memcpy(sCmdSendBuff, &mode, 4);
	SifWriteBackDCache(sCmdSendBuff, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_MMODE, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}

// change cd thread priority
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        media mode
// returns:     1 if successful
//                      0 if error
s32 cdChangeThreadPriority(u32 priority)
{
	if (cdCheckSCmd(CD_SCMD_SETTHREADPRI) == 0)
		return 0;

	memcpy(sCmdSendBuff, &priority, 4);
	SifWriteBackDCache(sCmdSendBuff, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_SETTHREADPRI, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(s32 *) UNCACHED_SEG(sCmdRecvBuff);
}


// check whether ready to send an s-command
// 
// args:        current command
// returns:     1 if ready to send
//                      0 if busy/error
s32 cdCheckSCmd(s32 cur_cmd)
{
	s32 i;
	cdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId) {
		if (cdDebug > 0)
			printf("Scmd fail sema cur_cmd:%d keep_cmd:%d\n", cur_cmd, sCmdNum);
		return 0;
	}
	sCmdNum = cur_cmd;
	ReferThreadStatus(cdThreadId, &cdThreadParam);
	if (cdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SifInitRpc(0);
	if (bindSCmd >= 0)
		return 1;
	while (1) {
		if (SifBindRpc(&clientSCmd, CD_SERVER_SCMD, 0) < 0) {
			if (cdDebug > 0)
				printf("Libcdvd bind err S cmd\n");
		}
		if (clientSCmd.server != 0)
			break;

		i = 0x10000;
		while (i--)
			;
	}

	bindSCmd = 0;
	return 1;
}


// s-command wait
// (shouldnt really need to call this yourself)
// 
// args:        0 = wait for completion of command (blocking)
//                      1 = check current status and return immediately
// returns:     0 = completed
//                      1 = not completed
s32 cdSyncS(s32 mode)
{
	if (mode == 0) {
		if (cdDebug > 0)
			printf("S cmd wait\n");
		while (SifCheckStatRpc(&clientSCmd))
			;
		return 0;
	}
	return SifCheckStatRpc(&clientSCmd);
}
