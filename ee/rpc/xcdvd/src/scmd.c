/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
## (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
#     2003 loser (loser@internalreality.com)
# (c) 2004 Marcus R. Brown <mrbrown@0xd6.org> Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Function definitions for libsceCdvd (EE side calls to the iop module sceCdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#
# NOTE: not all functions work with each set of modules!
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <libxcdvd.h>
#include <string.h>

#include "internal.h"

#define CD_SERVER_SCMD			0x80000593	// blocking commands (Synchronous)

#define CD_SCMD_READCLOCK		0x01
#define CD_SCMD_WRITECLOCK		0x02
#define CD_SCMD_GETDISKTYPE		0x03
#define CD_SCMD_GETERROR		0x04
#define CD_SCMD_TRAYREQ			0x05
#define CD_SCMD_READ_ILINK_ID		0x06
#define CD_SCMD_WRITE_ILINK_ID		0x07
#define CD_SCMD_READ_NVM		0x08
#define CD_SCMD_WRITE_NVM		0x09
#define CD_SCMD_DEC_SET			0x0A
#define CD_SCMD_SCMD			0x0B
#define CD_SCMD_STATUS			0x0C
#define CD_SCMD_BREAK			0x16
#define CD_SCMD_OPEN_CONFIG		0x0E
#define CD_SCMD_CLOSE_CONFIG		0x0F
#define CD_SCMD_READ_CONFIG		0x10
#define CD_SCMD_WRITE_CONFIG		0x11
#define CD_SCMD_READ_CONSOLE_ID		0x12
#define CD_SCMD_WRITE_CONSOLE_ID	0x13
#define CD_SCMD_READ_MECHACON_VERSION	0x14
#define CD_SCMD_FORBID_DVDP		0x18
#define CD_SCMD_READ_MODEL_NAME		0x1A	// XCDVDFSV only
#define CD_SCMD_WRITE_MODEL_NAME	0x1B	// XCDVDFSV only
#define CD_SCMD_BOOT_CERTIFY		0x1E	// XCDVDFSV only
#define CD_SCMD_CANCELPOWEROFF		0x1F	// XCDVDFSV only
#define CD_SCMD_BLUELEDCTRL		0x20	// XCDVDFSV only
#define CD_SCMD_POWEROFF		0x21	// XCDVDFSV only
#define CD_SCMD_MMODE			0x22	// XCDVDFSV only
#define CD_SCMD_SETTHREADPRI		0x23	// XCDVDFSV only

#ifdef F__scmd_internals
int bindSCmd = -1;

SifRpcClientData_t clientSCmd __attribute__ ((aligned(64)));	// for s-cmds

int sCmdSemaId = -1;		// s-cmd semaphore id

u8 sCmdRecvBuff[0x440] __attribute__ ((aligned(64)));
u8 sCmdSendBuff[0x420] __attribute__ ((aligned(64)));

int sCmdNum = 0;

int sceCdConfigRdWrNumBlocks;
#endif

extern int bindSCmd;
extern SifRpcClientData_t clientSCmd;
extern int sCmdSemaId;
extern u8 sCmdRecvBuff[];
extern u8 sCmdSendBuff[];
extern int sCmdNum;

extern int sceCdConfigRdWrNumBlocks;

int sceCdCheckSCmd(int cmd);


// **** S-Command Functions ****


// read clock value from ps2s clock
// 
// args:        time/date struct
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdReadClock
int sceCdReadClock(sceCdCLOCK * clock)
{
	if (sceCdCheckSCmd(CD_SCMD_READCLOCK) == 0)
		return 0;

	if (sceCdDebug > 0)
		printf("LibsceCdvd call Clock read 1\n");

	if (SifCallRpc(&clientSCmd, CD_SCMD_READCLOCK, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy(clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);

	if (sceCdDebug > 0)
		printf("LibsceCdvd call Clock read 2\n");

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// write clock value to ps2s clock
// 
// args:        time/date struct to set clocks time with
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdWriteClock
int sceCdWriteClock(const sceCdCLOCK * clock)
{
	if (sceCdCheckSCmd(CD_SCMD_WRITECLOCK) == 0)
		return 0;

	memcpy(sCmdSendBuff, clock, 8);

	if (SifCallRpc(&clientSCmd, CD_SCMD_WRITECLOCK, 0, sCmdSendBuff, 8, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy((sceCdCLOCK *)clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// gets the type of the currently inserted disc
// 
// returns:     disk type (CDVD_TYPE_???)
#ifdef F_sceCdGetDiskType
int sceCdGetDiskType(void)
{
	if (sceCdCheckSCmd(CD_SCMD_GETDISKTYPE) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETDISKTYPE, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// gets the last error that occurred
// 
// returns:     error type (CDVD_ERR_???)
#ifdef F_sceCdGetError
int sceCdGetError(void)
{
	if (sceCdCheckSCmd(CD_SCMD_GETERROR) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETERROR, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// open/close/check disk tray
// 
// args:        param (CDVD_TRAY_???)
//                      address for returning tray state change
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdTrayReq
int sceCdTrayReq(int param, u32 * traychk)
{
	if (sceCdCheckSCmd(CD_SCMD_TRAYREQ) == 0)
		return 0;

	memcpy(sCmdSendBuff, &param, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_TRAYREQ, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (traychk)
		*traychk = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// send an s-command by function number
// 
// args:        command number
//                      input buffer  (can be null)
//                      size of input buffer  (0 - 16 byte)
//                      output buffer (can be null)
//                      size of output buffer (0 - 16 bytes)
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdApplySCmd
int sceCdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize)
{
	if (sceCdCheckSCmd(CD_SCMD_SCMD) == 0)
		return 0;

	*(u16 *) & sCmdSendBuff[0] = cmdNum;
	*(u16 *) & sCmdSendBuff[2] = inBuffSize;
	memset(&sCmdSendBuff[4], 0, 16);
	if (inBuff)
		memcpy(&sCmdSendBuff[4], inBuff, inBuffSize);

	if (SifCallRpc(&clientSCmd, CD_SCMD_SCMD, 0, sCmdSendBuff, 20, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (outBuff)
		memcpy(outBuff, UNCACHED_SEG(sCmdRecvBuff), outBuffSize);
	SignalSema(sCmdSemaId);
	return 1;
}
#endif

// gets the status of the sceCd system
// 
// returns:     status (CDVD_STAT_???)
#ifdef F_sceCdStatus
int sceCdStatus(void)
{
	if (sceCdCheckSCmd(CD_SCMD_STATUS) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_STATUS, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	if (sceCdDebug >= 2)
		printf("status called\n");

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// 'breaks' the currently executing command
// 
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdBreak
int sceCdBreak(void)
{
	if (sceCdCheckSCmd(CD_SCMD_BREAK) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_BREAK, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// cancel power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        result
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdCancelPowerOff
int sceCdCancelPowerOff(u32 * result)
{
	if (sceCdCheckSCmd(CD_SCMD_CANCELPOWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_CANCELPOWEROFF, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// blue led control
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        control value
//                      result
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdBlueLedCtrl
int sceCdBlueLedCtrl(u8 control, u32 * result)
{
	if (sceCdCheckSCmd(CD_SCMD_BLUELEDCTRL) == 0)
		return 0;

	*(u32 *) sCmdSendBuff = control;

	if (SifCallRpc(&clientSCmd, CD_SCMD_BLUELEDCTRL, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// power off
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        result
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdPowerOff
int sceCdPowerOff(u32 * result)
{
	if (sceCdCheckSCmd(CD_SCMD_POWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_POWEROFF, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// set media mode
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        media mode
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdMMode
int sceCdMMode(int media)
{
	if (sceCdCheckSCmd(CD_SCMD_MMODE) == 0)
		return 0;

	memcpy(sCmdSendBuff, &media, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_MMODE, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// change sceCd thread priority
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        media mode
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdChangeThreadPriority
int sceCdChangeThreadPriority(int priority)
{
	if (sceCdCheckSCmd(CD_SCMD_SETTHREADPRI) == 0)
		return 0;

	memcpy(sCmdSendBuff, &priority, 4);

	if (SifCallRpc(&clientSCmd, CD_SCMD_SETTHREADPRI, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

// check whether ready to send an s-command
// 
// args:        current command
// returns:     1 if ready to send
//                      0 if busy/error
#ifdef F_sceCdCheckSCmd
int sceCdCheckSCmd(int cur_cmd)
{
	int i;
	sceCdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId) {
		if (sceCdDebug > 0)
			printf("Scmd fail sema cur_cmd:%d keep_cmd:%d\n", cur_cmd, sCmdNum);
		return 0;
	}
	sCmdNum = cur_cmd;
	ReferThreadStatus(sceCdThreadId, &sceCdThreadParam);
	if (sceCdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SifInitRpc(0);
	if (bindSCmd >= 0)
		return 1;
	while (1) {
		if (SifBindRpc(&clientSCmd, CD_SERVER_SCMD, 0) < 0) {
			if (sceCdDebug > 0)
				printf("LibsceCdvd bind err S cmd\n");
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
#endif

#ifdef F_sceCdBootCertify
/* Unlocks the CD/DVD drive with the specified ROM version string.
	*** XCDVDFSV and XCDVDMAN only ***

	Argument(s):	romver - The ROM 'name' of the console in this format:
				romname[0]	- ROM major version number (In decimal).
				romname[1]	- ROM minor version number (In decimal).
				romname[2]	- ROM region.
				romname[3]	- Console type (E.g. C, X or T).

	Returns:	1 on success
			0 on failure
*/
int sceCdBootCertify(const u8 *romname){
	int result;

	if(sceCdCheckSCmd(CD_SCMD_BOOT_CERTIFY)==0) return 0;

	memcpy(sCmdSendBuff, romname, 4);
	result=SifCallRpc(&clientSCmd, CD_SCMD_BOOT_CERTIFY, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL);

	SignalSema(sCmdSemaId);
	return((result<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdForbidDVDP
/* Disables DVD video disc playback.
	*** XCDVDFSV and XCDVDMAN only ***

	Argument(s):	result - A pointer to a variable to store the result of the operation.

	Returns:	1 on success
			0 on failure
*/
int sceCdForbidDVDP(u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_FORBID_DVDP)==0) return 0;

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_FORBID_DVDP, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL);

	*result=((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
	SignalSema(sCmdSemaId);

	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdDecSet
/* Sets up data decryption parameters.
 *
 * Arguments:	arg1	- unknown
 *		arg2	- unknown
 *		shift	- Number of bits to shift by.
 *
 * Returns:	1 if successful.
 *		0 if an error occurred.
*/
int sceCdDecSet(unsigned char arg1, unsigned char arg2, unsigned char shift)
{
	int result;

	if(sceCdCheckSCmd(CD_SCMD_DEC_SET)==0) return 0;

	sCmdSendBuff[0]=arg1;
	sCmdSendBuff[1]=arg2;
	sCmdSendBuff[2]=shift;

	result=SifCallRpc(&clientSCmd, CD_SCMD_DEC_SET, 0, sCmdSendBuff, 4, sCmdRecvBuff, 16, NULL, NULL);

	SignalSema(sCmdSemaId);
	return((result<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdOpenConfig
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *result){
	int RpcCallResult;

	if(NumBlocks<0x45){
		if(sceCdCheckSCmd(CD_SCMD_OPEN_CONFIG)==0) return 0;

		*(int *)sCmdSendBuff=((NumBlocks&0xFF)<<16)|(mode&0xFF)|((block&0xFF)<<8);
		sceCdConfigRdWrNumBlocks=NumBlocks;
		if(SifCallRpc(&clientSCmd, CD_SCMD_OPEN_CONFIG, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL)>=0){
			*result=((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
			RpcCallResult=((int *)UNCACHED_SEG(sCmdRecvBuff))[0];
			SignalSema(sCmdSemaId);
		}
		else RpcCallResult=0;
	}
	else RpcCallResult=0;

	return RpcCallResult;
}
#endif

#ifdef F_sceCdCloseConfig
/*
	Value used by OSDSYS v1.01:
		arg1	-> $sp
*/
int sceCdCloseConfig(u32 *result){
	int RpcCallResult;

	if(sceCdCheckSCmd(CD_SCMD_CLOSE_CONFIG)==0) return 0;

	RpcCallResult=SifCallRpc(&clientSCmd, CD_SCMD_CLOSE_CONFIG, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL);

	*result=((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];

	SignalSema(sCmdSemaId);
	return((RpcCallResult<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdReadConfig
int sceCdReadConfig(void *buffer, u32 *result){
	int RpcCallResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_CONFIG)==0) return 0;

	RpcCallResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_CONFIG, 0, NULL, 0, sCmdRecvBuff, 0x408, NULL, NULL);

	*result=((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
	memcpy(buffer, &((unsigned int *)UNCACHED_SEG(sCmdRecvBuff))[2], (sceCdConfigRdWrNumBlocks<<4)-sceCdConfigRdWrNumBlocks);

	SignalSema(sCmdSemaId);

	return((RpcCallResult<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdWriteConfig
int sceCdWriteConfig(const void *buffer, u32 *result){
	int RpcCallResult;

	if(sceCdCheckSCmd(CD_SCMD_WRITE_CONFIG)==0) return 0;

	/* Yes, it copies the data to offset 0x20. Weird layout... I hope that it isn't ps2dis acting weird again. */
	memcpy(&sCmdSendBuff[0x20], buffer, (sceCdConfigRdWrNumBlocks<<4)-sceCdConfigRdWrNumBlocks);

	RpcCallResult=SifCallRpc(&clientSCmd, CD_SCMD_WRITE_CONFIG, 0, &sCmdSendBuff[0x20], 0x400, sCmdRecvBuff, 8, NULL, NULL);

	*result=((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
	SignalSema(sCmdSemaId);

	return((RpcCallResult<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdReadNVM
int sceCdReadNVM(unsigned int address, unsigned short int *data, unsigned char *result){
	int RpcCallResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_NVM)==0) return 0;

	*(unsigned int *)sCmdSendBuff=address;
	*(unsigned short int *)&sCmdSendBuff[4]=0;
	sCmdSendBuff[6]=0;

	RpcCallResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_NVM, 0, sCmdSendBuff, 8, sCmdRecvBuff, 0x10, NULL, NULL);

	*data=*(unsigned short int *)&sCmdRecvBuff[8];
	*result=*(unsigned char *)&sCmdRecvBuff[10];

	SignalSema(sCmdSemaId);
	return((RpcCallResult<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdWriteNVM
int sceCdWriteNVM(unsigned int address, unsigned short int data, unsigned char *result){
	int RpcCallResult;

	if(sceCdCheckSCmd(CD_SCMD_WRITE_NVM)==0) return 0;

	*(unsigned int *)sCmdSendBuff=address;
	*(unsigned short int *)&sCmdSendBuff[4]=data;
	sCmdSendBuff[6]=0;

	RpcCallResult=SifCallRpc(&clientSCmd, CD_SCMD_WRITE_NVM, 0, sCmdSendBuff, 8, sCmdRecvBuff, 0x10, NULL, NULL);

	*result=*(unsigned char *)&sCmdRecvBuff[10];

	SignalSema(sCmdSemaId);
	return((RpcCallResult<0)?0:*(int *)sCmdRecvBuff);
}
#endif

#ifdef F_sceCdRI
int sceCdRI(unsigned char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_ILINK_ID)==0) return 0;

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_ILINK_ID, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL);

	memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 8);
	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdWI
int sceCdWI(const unsigned char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_WRITE_ILINK_ID)==0) return 0;

	memcpy(sCmdSendBuff, buffer, 8);
	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_WRITE_ILINK_ID, 0, sCmdSendBuff, 8, sCmdRecvBuff, 8, NULL, NULL);

	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdReadConsoleID
int sceCdReadConsoleID(unsigned char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_CONSOLE_ID)==0) return 0;

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_CONSOLE_ID, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL);

	memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 8);
	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdWriteConsoleID
int sceCdWriteConsoleID(const unsigned char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_WRITE_CONSOLE_ID)==0) return 0;

	memcpy(sCmdSendBuff, buffer, 8);
	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_WRITE_CONSOLE_ID, 0, sCmdSendBuff, 8, sCmdRecvBuff, 8, NULL, NULL);

	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdMV
int sceCdMV(unsigned char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_MECHACON_VERSION)==0) return 0;

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_MECHACON_VERSION, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL);

	memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 3);
	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdRM
int sceCdRM(char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_READ_MODEL_NAME)==0) return 0;

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_READ_MODEL_NAME, 0, NULL, 0, sCmdRecvBuff, 0x18, NULL, NULL);

	memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 16);
	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

#ifdef F_sceCdWM
int sceCdWM(const char *buffer, u32 *result){
	int CallRpcResult;

	if(sceCdCheckSCmd(CD_SCMD_WRITE_MODEL_NAME)==0) return 0;

	memcpy(sCmdSendBuff, buffer, 16);

	CallRpcResult=SifCallRpc(&clientSCmd, CD_SCMD_WRITE_MODEL_NAME, 0, sCmdSendBuff, 0x10, sCmdRecvBuff, 8, NULL, NULL);
	*result=*(unsigned int *)UNCACHED_SEG(&sCmdRecvBuff[4]);

	SignalSema(sCmdSemaId);
	return((CallRpcResult<0)?0:*(int *)UNCACHED_SEG(sCmdRecvBuff));
}
#endif

// s-command wait
// (shouldnt really need to call this yourself)
// 
// args:        0 = wait for completion of command (blocking)
//                      1 = check current status and return immediately
// returns:     0 = completed
//                      1 = not completed
#ifdef F_sceCdSyncS
int sceCdSyncS(int mode)
{
	if (mode == 0) {
		if (sceCdDebug > 0)
			printf("S cmd wait\n");
		while (SifCheckStatRpc(&clientSCmd))
			;
		return 0;
	}
	return SifCheckStatRpc(&clientSCmd);
}
#endif
