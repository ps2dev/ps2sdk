/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
## (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
#     2003 loser (loser@internalreality.com)
# (c) 2004 Marcus R. Brown <mrbrown@0xd6.org> Licenced under Academic Free License version 2.0
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
#include <kernel.h>
#include <sifrpc.h>
#include <libcdvd.h>
#include <libcdvd-rpc.h>
#include <string.h>
#include <time.h>
#include <osd_config.h>

#include "internal.h"

/** blocking commands (Synchronous) */
#define CD_SERVER_SCMD			0x80000593

enum CD_SCMD_CMDS {
	CD_SCMD_READCLOCK	=	0x01,
	CD_SCMD_WRITECLOCK,
	CD_SCMD_GETDISKTYPE,
	CD_SCMD_GETERROR,
	CD_SCMD_TRAYREQ,
	CD_SCMD_READ_ILINK_ID,
	CD_SCMD_WRITE_ILINK_ID,
	CD_SCMD_READ_NVM,
	CD_SCMD_WRITE_NVM,
	CD_SCMD_DEC_SET,
	CD_SCMD_SCMD,
	CD_SCMD_STATUS,
	CD_SCMD_SET_HD_MODE,
	CD_SCMD_OPEN_CONFIG,
	CD_SCMD_CLOSE_CONFIG,
	CD_SCMD_READ_CONFIG,
	CD_SCMD_WRITE_CONFIG,
	CD_SCMD_READ_CONSOLE_ID,
	CD_SCMD_WRITE_CONSOLE_ID,
	CD_SCMD_READ_MECHACON_VERSION,
	CD_SCMD_CTRL_AD_OUT,
	CD_SCMD_BREAK,
	CD_SCMD_READ_SUBQ,
	CD_SCMD_FORBID_DVDP,
	CD_SCMD_AUTO_ADJUST_CTRL,
	CD_SCMD_READ_MODEL_NAME,
	CD_SCMD_WRITE_MODEL_NAME,
	CD_SCMD_FORBID_READ,
	CD_SCMD_SPIN_CTRL,
	CD_SCMD_BOOT_CERTIFY,
	CD_SCMD_CANCELPOWEROFF,
	CD_SCMD_BLUELEDCTRL,
	CD_SCMD_POWEROFF,
	CD_SCMD_MMODE,
	CD_SCMD_SETTHREADPRI,
};

typedef union {
	s32 s32arg;
	u32 u32arg;
	u8 bcertify[4];
	sceCdCLOCK clock;
	struct cdvdScmdParam scmd;
	struct cdvdDecSetParam decSet;
	struct cdvdReadWriteNvmParam nvm;
	u8 id[8];
	char mname[16];
	u8 data[0x420];
} sCmdSendParams_t;

#ifdef F__scmd_internals
int bindSCmd = -1;

SifRpcClientData_t clientSCmd __attribute__ ((aligned(64)));

int sCmdSemaId = -1;

u8 sCmdRecvBuff[0x440] __attribute__ ((aligned(64)));
sCmdSendParams_t sCmdSendBuff __attribute__ ((aligned(64)));

int sCmdNum = 0;

int CdConfigRdWrNumBlocks;
#endif

extern int bindSCmd;
extern SifRpcClientData_t clientSCmd;
extern int sCmdSemaId;
extern u8 sCmdRecvBuff[];
extern sCmdSendParams_t sCmdSendBuff;
extern int sCmdNum;

extern int CdConfigRdWrNumBlocks;

extern void convertfrombcd(sceCdCLOCK* time);

int _CdCheckSCmd(int cmd);

/* S-Command Functions */

#ifdef F_sceCdReadClock
int sceCdReadClock(sceCdCLOCK * clock)
{
	if (_CdCheckSCmd(CD_SCMD_READCLOCK) == 0)
		return 0;

	if (CdDebug > 0)
		printf("Libcdvd call Clock read 1\n");

	if (SifCallRpc(&clientSCmd, CD_SCMD_READCLOCK, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy(clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);

	if (CdDebug > 0)
		printf("Libcdvd call Clock read 2\n");

	SignalSema(sCmdSemaId);
	return *(int *) UNCACHED_SEG(sCmdRecvBuff);
}
#endif

#ifdef F_time
/*
 * newlib function, unfortunately depends on the 'cdvd' library.
 * In libc there is a dummy   'time' function declared as WEAK.
 * In cdvd there is a working 'time' function declared as STRONG
 * Include libcdvd if you need to use the time function.
 */
time_t time(time_t *t)
{
        sceCdCLOCK ps2tim;
	struct tm tim;
        time_t tim2;

	sceCdReadClock(&ps2tim);
        configConvertToGmtTime(&ps2tim);
        //configConvertToLocalTime(&ps2tim);
        convertfrombcd(&ps2tim);
#ifdef DEBUG
        printf("ps2time: %d-%d-%d %d:%d:%d\n",
                ps2tim.day,
                ps2tim.month,
                ps2tim.year,
                ps2tim.hour,
                ps2tim.minute,
                ps2tim.second);
#endif
	tim.tm_sec  = ps2tim.second;
        tim.tm_min  = ps2tim.minute;
        tim.tm_hour = ps2tim.hour;
        tim.tm_mday = ps2tim.day;
        tim.tm_mon  = ps2tim.month - 1;
        tim.tm_year = ps2tim.year + 100;

        tim2 = mktime(&tim);

        if(t != NULL)
                *t = tim2;

	return tim2;
}
#endif

#ifdef F_sceCdWriteClock
int sceCdWriteClock(const sceCdCLOCK * clock)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_WRITECLOCK) == 0)
		return 0;

	memcpy(&sCmdSendBuff.clock, clock, 8);

	if (SifCallRpc(&clientSCmd, CD_SCMD_WRITECLOCK, 0, &sCmdSendBuff, 8, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	memcpy((sceCdCLOCK *)clock, UNCACHED_SEG(sCmdRecvBuff + 4), 8);
	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdGetDiskType
int sceCdGetDiskType(void)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_GETDISKTYPE) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETDISKTYPE, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdGetError
int sceCdGetError(void)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_GETERROR) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_GETERROR, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdTrayReq
int sceCdTrayReq(int param, u32 * traychk)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_TRAYREQ) == 0)
		return 0;

	sCmdSendBuff.s32arg = param;

	if (SifCallRpc(&clientSCmd, CD_SCMD_TRAYREQ, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (traychk)
		*traychk = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdApplySCmd
int sceCdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize)
{
	if (_CdCheckSCmd(CD_SCMD_SCMD) == 0)
		return 0;

	sCmdSendBuff.scmd.cmdNum = cmdNum;
	sCmdSendBuff.scmd.inBuffSize = inBuffSize;
	memset(sCmdSendBuff.scmd.inBuff, 0, 16);
	if (inBuff)
		memcpy(sCmdSendBuff.scmd.inBuff, inBuff, inBuffSize);

	if (SifCallRpc(&clientSCmd, CD_SCMD_SCMD, 0, &sCmdSendBuff, 20, sCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	if (outBuff)
		memcpy(outBuff, UNCACHED_SEG(sCmdRecvBuff), outBuffSize);
	SignalSema(sCmdSemaId);
	return 1;
}
#endif

#ifdef F_sceCdStatus
int sceCdStatus(void)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_STATUS) == 0)
		return -1;

	if (SifCallRpc(&clientSCmd, CD_SCMD_STATUS, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return -1;
	}

	if (CdDebug >= 2)
		printf("status called\n");
	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdBreak
int sceCdBreak(void)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_BREAK) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_BREAK, 0, NULL, 0, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}
	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdCancelPOffRdy
int sceCdCancelPOffRdy(u32 * result)
{
	int status;

	if (_CdCheckSCmd(CD_SCMD_CANCELPOWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_CANCELPOWEROFF, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	status = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdBlueLedCtrl
int sceCdBlueLedCtrl(u8 control, u32 *result)
{
	int status;

	if (_CdCheckSCmd(CD_SCMD_BLUELEDCTRL) == 0)
		return 0;

	sCmdSendBuff.u32arg = control;
	if (SifCallRpc(&clientSCmd, CD_SCMD_BLUELEDCTRL, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	status = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdPowerOff
int sceCdPowerOff(u32 * result)
{
	int status;

	if (_CdCheckSCmd(CD_SCMD_POWEROFF) == 0)
		return 0;

	if (SifCallRpc(&clientSCmd, CD_SCMD_POWEROFF, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	*result = *(u32 *) UNCACHED_SEG(sCmdRecvBuff + 4);
	status = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdMmode
int sceCdMmode(int media)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_MMODE) == 0)
		return 0;

	sCmdSendBuff.s32arg = media;
	if (SifCallRpc(&clientSCmd, CD_SCMD_MMODE, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdChangeThreadPriority
int sceCdChangeThreadPriority(int priority)
{
	int result;

	if (_CdCheckSCmd(CD_SCMD_SETTHREADPRI) == 0)
		return 0;

	sCmdSendBuff.s32arg = priority;
	if (SifCallRpc(&clientSCmd, CD_SCMD_SETTHREADPRI, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	result = *(int *) UNCACHED_SEG(sCmdRecvBuff);

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F__CdCheckSCmd
int _CdCheckSCmd(int cur_cmd)
{
	int i;
	_CdSemaInit();
	if (PollSema(sCmdSemaId) != sCmdSemaId) {
		if (CdDebug > 0)
			printf("Scmd fail sema cur_cmd:%d keep_cmd:%d\n", cur_cmd, sCmdNum);
		return 0;
	}
	sCmdNum = cur_cmd;
	ReferThreadStatus(CdThreadId, &CdThreadParam);
	if (_CdSyncS(1)) {
		SignalSema(sCmdSemaId);
		return 0;
	}

	SifInitRpc(0);
	if (bindSCmd >= 0)
		return 1;
	while (1) {
		if (SifBindRpc(&clientSCmd, CD_SERVER_SCMD, 0) < 0) {
			if (CdDebug > 0)
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
#endif

#ifdef F_sceCdForbidRead
int sceCdForbidRead(u32 *status)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_FORBID_READ)==0) return 0;
	if(SifCallRpc(&clientSCmd, CD_SCMD_FORBID_READ, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*status = ((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];
		result = ((int *)UNCACHED_SEG(sCmdRecvBuff))[0];
	}else{
		result = 0;
	}
	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdSpinCtrlEE
int sceCdSpinCtrlEE(u32 speed)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_SPIN_CTRL)==0) return 0;
	sCmdSendBuff.u32arg = speed;
	if(SifCallRpc(&clientSCmd, CD_SCMD_SPIN_CTRL, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL)>=0){
		result = ((int *)UNCACHED_SEG(sCmdRecvBuff))[0];
	}else{
		result = 0;
	}
	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdBootCertify
int sceCdBootCertify(const u8 *romname)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_BOOT_CERTIFY)==0) return 0;

	memcpy(sCmdSendBuff.bcertify, romname, 4);
	if(SifCallRpc(&clientSCmd, CD_SCMD_BOOT_CERTIFY, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL)>=0){
		result=*(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		result=0;
	}

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdReadSUBQ
int sceCdReadSUBQ(void *buffer, u32 *status)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_READ_SUBQ)==0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_SUBQ, 0, NULL, 0, sCmdRecvBuff, 0x12, NULL, NULL)>=0){
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 0x12);
		*status=*(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		result=*(int *)UNCACHED_SEG(sCmdRecvBuff);
	}
	else{
		result=0;
	}

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdForbidDVDP
int sceCdForbidDVDP(u32 *result)
{
	int status;

	if(_CdCheckSCmd(CD_SCMD_FORBID_DVDP) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_FORBID_DVDP, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = ((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];
		status = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdAutoAdjustCtrl
int sceCdAutoAdjustCtrl(int mode, u32 *result)
{
	int status;

	if(_CdCheckSCmd(CD_SCMD_AUTO_ADJUST_CTRL) == 0) return 0;

	sCmdSendBuff.s32arg = mode;
	if(SifCallRpc(&clientSCmd, CD_SCMD_AUTO_ADJUST_CTRL, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = ((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];
		status = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdDecSet
int sceCdDecSet(unsigned char arg1, unsigned char arg2, unsigned char shift)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_DEC_SET)==0) return 0;

	sCmdSendBuff.decSet.arg1 = arg1;
	sCmdSendBuff.decSet.arg2 = arg2;
	sCmdSendBuff.decSet.shift = shift;

	if(SifCallRpc(&clientSCmd, CD_SCMD_DEC_SET, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 16, NULL, NULL)>=0){
		result = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		result = 0;
	}

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdSetHDMode
int sceCdSetHDMode(u32 mode)
{
	int result;

	if(_CdCheckSCmd(CD_SCMD_SET_HD_MODE)==0) return 0;
	sCmdSendBuff.u32arg = mode;
	if(SifCallRpc(&clientSCmd, CD_SCMD_SET_HD_MODE, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL)>=0){
		result=*(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		result=0;
	}
	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdOpenConfig
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *status){
	int result;

	if(NumBlocks<0x45){
		if(_CdCheckSCmd(CD_SCMD_OPEN_CONFIG) == 0) return 0;

		sCmdSendBuff.u32arg = ((NumBlocks&0xFF)<<16) | (mode&0xFF) | ((block&0xFF)<<8);
		CdConfigRdWrNumBlocks = NumBlocks;
		if(SifCallRpc(&clientSCmd, CD_SCMD_OPEN_CONFIG, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL)>=0){
			*status = ((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];
			result = ((int *)UNCACHED_SEG(sCmdRecvBuff))[0];
		}
		else result = 0;

		SignalSema(sCmdSemaId);
	}
	else result=0;

	return result;
}
#endif

#ifdef F_sceCdCloseConfig
int sceCdCloseConfig(u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_CLOSE_CONFIG) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_CLOSE_CONFIG, 0, NULL, 0, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = ((u32 *)UNCACHED_SEG(sCmdRecvBuff))[1];
		status = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdReadConfig
int sceCdReadConfig(void *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_CONFIG) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_CONFIG, 0, NULL, 0, sCmdRecvBuff, 0x408, NULL, NULL)>=0){
		*result = ((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
		memcpy(buffer, &((u32 *)UNCACHED_SEG(sCmdRecvBuff))[2], (CdConfigRdWrNumBlocks<<4)-CdConfigRdWrNumBlocks);
		status = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdWriteConfig
int sceCdWriteConfig(const void *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_WRITE_CONFIG) == 0) return 0;

	memcpy(sCmdSendBuff.data, buffer, (CdConfigRdWrNumBlocks<<4)-CdConfigRdWrNumBlocks);
	if(SifCallRpc(&clientSCmd, CD_SCMD_WRITE_CONFIG, 0, &sCmdSendBuff, 0x400, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = ((int *)UNCACHED_SEG(sCmdRecvBuff))[1];
		status = *(int*)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);

	return status;
}
#endif

#ifdef F_sceCdReadNVM
int sceCdReadNVM(u32 address, u16 *data, u8 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_NVM) == 0) return 0;

	sCmdSendBuff.nvm.address = address;
	sCmdSendBuff.nvm.value = 0;
	sCmdSendBuff.nvm.pad = 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_NVM, 0, &sCmdSendBuff, 8, sCmdRecvBuff, 0x10, NULL, NULL)>=0){
		*data = *(unsigned short int *)UNCACHED_SEG(&sCmdRecvBuff[8]);
		*result = *(u8 *)UNCACHED_SEG(&sCmdRecvBuff[10]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status=0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdWriteNVM
int sceCdWriteNVM(u32 address, u16 data, u8 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_WRITE_NVM) == 0) return 0;

	sCmdSendBuff.nvm.address=address;
	sCmdSendBuff.nvm.value = data;
	sCmdSendBuff.nvm.pad = 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_WRITE_NVM, 0, &sCmdSendBuff, 8, sCmdRecvBuff, 0x10, NULL, NULL)>=0){
		*result = *(u8 *)UNCACHED_SEG(&sCmdRecvBuff[10]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdRI
int sceCdRI(unsigned char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_ILINK_ID) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_ILINK_ID, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL)>=0){
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 8);
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdWI
int sceCdWI(const unsigned char *buffer, u32 *status){
	int result;

	if(_CdCheckSCmd(CD_SCMD_WRITE_ILINK_ID) == 0) return 0;

	memcpy(sCmdSendBuff.id, buffer, 8);
	if(SifCallRpc(&clientSCmd, CD_SCMD_WRITE_ILINK_ID, 0, &sCmdSendBuff, 8, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*status = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		result = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		result = 0;
	}

	SignalSema(sCmdSemaId);
	return result;
}
#endif

#ifdef F_sceCdReadConsoleID
int sceCdReadConsoleID(unsigned char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_CONSOLE_ID) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_CONSOLE_ID, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL)>=0){
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 8);
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdWriteConsoleID
int sceCdWriteConsoleID(const unsigned char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_WRITE_CONSOLE_ID) == 0) return 0;

	memcpy(sCmdSendBuff.id, buffer, 8);
	if(SifCallRpc(&clientSCmd, CD_SCMD_WRITE_CONSOLE_ID, 0, &sCmdSendBuff, 8, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdMV
int sceCdMV(unsigned char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_MECHACON_VERSION) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_MECHACON_VERSION, 0, NULL, 0, sCmdRecvBuff, 16, NULL, NULL)>=0){

#ifdef _XCDVD
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 4);
#else
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 3);
#endif
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdCtrlADout
int sceCdCtrlADout(int arg1, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_CTRL_AD_OUT) == 0) return 0;

	sCmdSendBuff.s32arg = arg1;
	if(SifCallRpc(&clientSCmd, CD_SCMD_CTRL_AD_OUT, 0, &sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdRM
int sceCdRM(char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_READ_MODEL_NAME) == 0) return 0;

	if(SifCallRpc(&clientSCmd, CD_SCMD_READ_MODEL_NAME, 0, NULL, 0, sCmdRecvBuff, 0x18, NULL, NULL)>=0){
		memcpy(buffer, UNCACHED_SEG(&sCmdRecvBuff[8]), 16);
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status=0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F_sceCdWM
int sceCdWM(const char *buffer, u32 *result){
	int status;

	if(_CdCheckSCmd(CD_SCMD_WRITE_MODEL_NAME) == 0) return 0;

	memcpy(sCmdSendBuff.mname, buffer, 16);
	if(SifCallRpc(&clientSCmd, CD_SCMD_WRITE_MODEL_NAME, 0, &sCmdSendBuff, 0x10, sCmdRecvBuff, 8, NULL, NULL)>=0){
		*result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
		status = *(int *)UNCACHED_SEG(sCmdRecvBuff);
	}else{
		status = 0;
	}

	SignalSema(sCmdSemaId);
	return status;
}
#endif

#ifdef F__CdSyncS
int _CdSyncS(int mode)
{
	if (mode == 0) {
		if (CdDebug > 0)
			printf("S cmd wait\n");
		while (SifCheckStatRpc(&clientSCmd))
			;
		return 0;
	}

	return SifCheckStatRpc(&clientSCmd);
}
#endif
