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
#
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
#include <libcdvd.h>
#include <string.h>

#include "internal.h"

#define CD_SERVER_NCMD		0x80000595	// non-blocking commands (Non-synchronous)

/* Stream commands. */
typedef enum {
	CDVD_ST_CMD_START = 1,
	CDVD_ST_CMD_READ,
	CDVD_ST_CMD_STOP,
	CDVD_ST_CMD_SEEK,
	CDVD_ST_CMD_INIT,
	CDVD_ST_CMD_STAT,
	CDVD_ST_CMD_PAUSE,
	CDVD_ST_CMD_RESUME,
	CDVD_ST_CMD_SEEKF
} CdvdStCmd_t;

int sceCdStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, sceCdRMode *rm);
int sceCdCddaStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, sceCdRMode *rm);

enum CD_NCMD_CMDS {
	CD_NCMD_READ		= 0x01,
	CD_NCMD_CDDAREAD,
	CD_NCMD_DVDREAD,
	CD_NCMD_GETTOC,
	CD_NCMD_SEEK,
	CD_NCMD_STANDBY,
	CD_NCMD_STOP,
	CD_NCMD_PAUSE,
	CD_NCMD_STREAM,
	CD_NCMD_CDDASTREAM,
	CD_NCMD_READ_KEY,
	CD_NCMD_NCMD,
	CD_NCMD_READIOPMEM,
	CD_NCMD_DISKREADY,
	CD_NCMD_READCHAIN	// XCDVDFSV only
};

int _CdCheckNCmd(int cmd);

#ifdef F__ncmd_internals
int bindNCmd = -1;

SifRpcClientData_t clientNCmd __attribute__ ((aligned(64)));	// for n-cmds

int nCmdSemaId = -1;		// n-cmd semaphore id

int nCmdNum = 0;

u32 readStreamData[5] __attribute__ ((aligned(64)));
u32 readData[6] __attribute__ ((aligned(64)));
sceCdRChain readChainData[66] __attribute__ ((aligned(64)));
u32 getTocSendBuff[3] __attribute__ ((aligned(64)));	// get toc
u32 _rd_intr_data[64] __attribute__ ((aligned(64)));
u32 curReadPos __attribute__ ((aligned(64)));
u8 tocBuff[2064] __attribute__ ((aligned(64)));	// toc buffer (for sceCdGetToc())
u8 nCmdRecvBuff[48] __attribute__ ((aligned(64)));
u8 nCmdSendBuff[48] __attribute__ ((aligned(64)));
int streamStatus = 0;
sceCdRMode dummyMode;
u32 seekSector __attribute__ ((aligned(64)));
u8 cdda_st_buf[64] ALIGNED(64);
#endif

extern int bindNcmd;
extern SifRpcClientData_t clientNCmd;
extern int nCmdSemaId;
extern int nCmdNum;
extern u32 readStreamData[5];
extern u32 readData[6];
extern sceCdRChain readChainData[66];
extern u32 getTocSendBuff[3];
extern u32 _rd_intr_data[64];
extern u32 curReadPos;
extern u8 tocBuff[2064];
extern u8 nCmdRecvBuff[48];
extern u8 nCmdSendBuff[48];
extern int streamStatus;
extern sceCdRMode dummyMode;
extern u32 seekSector;
extern u8 cdda_st_buf[64];

int sceCdNCmdDiskReady(void);

// **** N-Command Functions ****

#ifdef _XCDVD
struct _cdvd_read_data
{
	u32	size1;
	u32	size2;
	void	*dest1;
	void	*dest2;
	u8	src1[64];
	u8	src2[64];
};
#else
struct _cdvd_read_data
{
	u32	size1;
	u32	size2;
	void	*dest1;
	void	*dest2;
	u8	src1[16];
	u8	src2[16];
};
#endif

void _CdAlignReadBuffer(void *data);
// this gets called when the sceCdRead function finishes
// to copy the data read in to unaligned buffers
#ifdef F__CdAlignReadBuffer
void _CdAlignReadBuffer(void *data)
{
	struct _cdvd_read_data *uncached = UNCACHED_SEG(data);
	
	if (uncached->size1 && uncached->dest1)	{
		memcpy(uncached->dest1, &uncached->src1, uncached->size1);
	}
	
	if (uncached->size2 && uncached->dest2)	{
		memcpy(uncached->dest2, &uncached->src2, uncached->size2);
	}
	
	_CdGenericCallbackFunction((void*)&CdCallbackNum);
}
#endif

// read data from sceCd
// non-blocking, requires sceCdSync() call
// 
// args:        sector location to start reading from
//                      number of sectors to read
//                      buffer to read to
//                      mode to read as
// returns: 1 if successful
//                      0 if error
#ifdef F_sceCdRead
int sceCdRead(u32 lbn, u32 sectors, void *buf, sceCdRMode * mode)
{
	int bufSize;

	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_READ) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = sectors;
	readData[2] = (u32) buf;
	readData[3] = (mode->trycount) | (mode->spindlctrl << 8) | (mode->datapattern << 16);
	readData[4] = (u32) _rd_intr_data;
	readData[5] = (u32) &curReadPos;

	// work out buffer size
	if (mode->datapattern == SCECdSecS2328)
		bufSize = sectors * 2328;
	else if (mode->datapattern == SCECdSecS2340)
		bufSize = sectors * 2340;
	else
		bufSize = sectors * 2048;

	curReadPos = 0;
	SifWriteBackDCache(buf, bufSize);
	SifWriteBackDCache(&curReadPos, sizeof(curReadPos));

	if (CdDebug > 0)
		printf("call sceCdread cmd\n");

	CdCallbackNum = CD_NCMD_READ;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_READ, SIF_RPC_M_NOWAIT, readData, 24, NULL, 0, &_CdAlignReadBuffer, _rd_intr_data) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (CdDebug > 0)
		printf("sceCdread end\n");

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

#ifdef F_sceCdReadDVDV
int sceCdReadDVDV(u32 lbn, u32 nsectors, void *buf, sceCdRMode *rm)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_DVDREAD) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = nsectors;
	readData[2] = (u32)buf;
	readData[3] = (rm->trycount) | (rm->spindlctrl << 8) | (rm->datapattern << 16);
	readData[4] = (u32)_rd_intr_data;

	SifWriteBackDCache(buf, nsectors * 2064);

	CdCallbackNum = CD_NCMD_DVDREAD;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_DVDREAD, SIF_RPC_M_NOWAIT, readData, 24, NULL, 0, &_CdAlignReadBuffer, _rd_intr_data) < 0) {	//The Sony function was broken here: It doesn't set a callback, which will cause the library to enter a deadlocked state.
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

#ifdef F_sceCdReadCDDA
int sceCdReadCDDA(u32 lbn, u32 nsectors, void *buf, sceCdRMode *rm)
{
	unsigned int sector_size;

	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_CDDAREAD) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = nsectors;
	readData[2] = (u32)buf;
	readData[3] = (rm->trycount) | (rm->spindlctrl << 8) | (rm->datapattern << 16);
	readData[4] = (u32)_rd_intr_data;

	/* Calculate the size of the read buffer.  */
	switch (rm->datapattern) {
		case SCECdSecS2368:
			sector_size = 2368;
			break;
		case SCECdSecS2448:
			sector_size = 2448;
			break;
		default:
			sector_size = 2352;
	}

	SifWriteBackDCache(buf, nsectors * sector_size);

	CdCallbackNum = CD_NCMD_CDDAREAD;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_CDDAREAD, SIF_RPC_M_NOWAIT, readData, 24, NULL, 0, &_CdAlignReadBuffer, _rd_intr_data) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// get toc from inserted disc
// 
// args:        buffer to hold toc (1024 or 2064 bytes?)
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdGetToc
int sceCdGetToc(u8 * toc)
{
	u8 *tocPtr, *tocEnd;

	if (_CdCheckNCmd(CD_NCMD_GETTOC) == 0)
		return 0;

	getTocSendBuff[0] = (u32) tocBuff;
	SifWriteBackDCache(tocBuff, 2064);

	if (SifCallRpc(&clientNCmd, CD_NCMD_GETTOC, 0, getTocSendBuff, 12, nCmdRecvBuff, 8, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	tocPtr = UNCACHED_SEG(tocBuff);
	tocEnd = tocPtr + 1024;
	if (*(u32 *) (nCmdRecvBuff + 4)) {
		do {
			memcpy(toc, tocPtr, 32);
			tocPtr += 32;
			toc += 32;
		} while (tocPtr < tocEnd);
	} else {
		tocEnd = tocPtr + 2048;

		do {
			memcpy(toc, tocPtr, 32);
			tocPtr += 32;
			toc += 32;
		} while (tocPtr < tocEnd);

		memcpy(toc, tocPtr, 16);
	}

	SignalSema(nCmdSemaId);
	return *(int *) UNCACHED_SEG(nCmdRecvBuff);
}
#endif

// seek to given sector on disc
// non-blocking, requires sceCdSync() call
// 
// args:        sector to seek to on disc
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdSeek
int sceCdSeek(u32 lbn)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_SEEK) == 0)
		return 0;

	seekSector = lbn;

	CdCallbackNum = CD_NCMD_SEEK;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_SEEK, SIF_RPC_M_NOWAIT, &seekSector, 4, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// puts ps2 sceCd drive into standby mode
// non-blocking, requires sceCdSync() call
// 
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdStandby
int sceCdStandby(void)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_STANDBY) == 0)
		return 0;

	CdCallbackNum = CD_NCMD_STANDBY;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_STANDBY, SIF_RPC_M_NOWAIT, NULL, 0, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// stops ps2 sceCd drive from spinning
// non-blocking, requires sceCdSync() call
// 
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdStop
int sceCdStop(void)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_STOP) == 0)
		return 0;

	CdCallbackNum = CD_NCMD_STOP;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_STOP, SIF_RPC_M_NOWAIT, NULL, 0, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// pauses ps2 sceCd drive
// non-blocking, requires sceCdSync() call
// 
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdPause
int sceCdPause(void)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_PAUSE) == 0)
		return 0;

	CdCallbackNum = CD_NCMD_PAUSE;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_PAUSE, SIF_RPC_M_NOWAIT, NULL, 0, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// send an n-command by function number
// 
// args:        command number
//                      input buffer  (can be null)
//                      size of input buffer  (0 - 16 byte)
//                      output buffer (can be null)
//                      size of output buffer (0 - 16 bytes)
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdApplyNCmd
int sceCdApplyNCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_NCMD) == 0)
		return 0;

	*(u16 *) & nCmdRecvBuff[0] = cmdNum;
	*(u16 *) & nCmdRecvBuff[2] = inBuffSize;
	memset(&nCmdRecvBuff[4], 0, 16);
	if (inBuff)
		memcpy(&nCmdRecvBuff[4], inBuff, inBuffSize);

	if (SifCallRpc(&clientNCmd, CD_NCMD_NCMD, 0, nCmdRecvBuff, 20, nCmdRecvBuff, 16, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (outBuff)
		memcpy((void *) outBuff, UNCACHED_SEG(nCmdRecvBuff), outBuffSize);

	SignalSema(nCmdSemaId);
	return *(int *) UNCACHED_SEG(nCmdRecvBuff);
}
#endif

// read data to iop memory
// non-blocking, requires sceCdSync() call
// 
// args:        sector location to read from
//                      number of sectors to read
//                      buffer to read to (in iop memory)
//                      read mode
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdReadIOPMem
int sceCdReadIOPMem(u32 lbn, u32 sectors, void *buf, sceCdRMode * mode)
{
	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_READIOPMEM) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = sectors;
	readData[2] = (u32) buf;
	readData[3] = (mode->trycount) | (mode->spindlctrl << 8) | (mode->datapattern << 16);
	readData[4] = (u32) _rd_intr_data;
	readData[5] = (u32) & curReadPos;

	CdCallbackNum = CD_NCMD_READIOPMEM;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_READIOPMEM, SIF_RPC_M_NOWAIT, readData, 24, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (CdDebug > 0)
		printf("sceCdread end\n");

	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// wait for disc to finish all n-commands
// (shouldnt really need to call this yourself)
// 
// returns:     6 if busy
//                      2 if ready
//                      0 if error
#ifdef F_sceCdNCmdDiskReady
int sceCdNCmdDiskReady(void)
{
	if (_CdCheckNCmd(CD_NCMD_DISKREADY) == 0)
		return 0;

	if (SifCallRpc(&clientNCmd, CD_NCMD_DISKREADY, 0, NULL, 0, &nCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return *(int *) UNCACHED_SEG(nCmdRecvBuff);
}
#endif

// do a 'chain' of reads with one command
// last chain values must be all 0xFFFFFFFF
// (max of 64 reads can be set at once)
// non-blocking, requires sceCdSync() call
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        pointer to an array of read chain data
//                      read mode
// returns:     1 if successful
//                      0 if error
#ifdef F_sceCdReadChain
int sceCdReadChain(sceCdRChain * readChain, sceCdRMode * mode)
{
	int chainNum, i, sectorType;

	if (sceCdNCmdDiskReady() == SCECdNotReady)
		return 0;
	if (_CdCheckNCmd(CD_NCMD_READCHAIN) == 0)
		return 0;

	if (CdDebug > 0)
		printf("call sceCdReadChain cmd 0\n");

	for (chainNum = 0; chainNum < 64; chainNum++) {
		if (readChain[chainNum].lbn == 0xFFFFFFFF ||
		    readChain[chainNum].sectors == 0xFFFFFFFF || readChain[chainNum].buffer == 0xFFFFFFFF)
			break;
		readChainData[chainNum].lbn = readChain[chainNum].lbn;
		readChainData[chainNum].sectors = readChain[chainNum].sectors;
		readChainData[chainNum].buffer = readChain[chainNum].buffer;
	}
	// store 'end of read-chain' data in chain
	readChainData[chainNum].lbn = 0xFFFFFFFF;
	readChainData[chainNum].sectors = 0xFFFFFFFF;
	readChainData[chainNum].buffer = 0xFFFFFFFF;

	// store read mode and read position in read chain data
	readChainData[65].lbn = (mode->trycount) | (mode->spindlctrl << 8) | (mode->datapattern << 16);
	readChainData[65].sectors = (u32) & curReadPos;

	if (mode->datapattern == SCECdSecS2328)
		sectorType = 2328;
	else if (mode->datapattern == SCECdSecS2340)
		sectorType = 2340;
	else
		sectorType = 2048;

	curReadPos = 0;
	if (CdDebug > 0)
		printf("call sceCdReadChain cmd 1\n");

	for (i = 0; i < chainNum; i++) {
		// if memory is on EE, make sure its not cached
		if ((readChainData[i].buffer & 1) == 0) {
			if (CdDebug > 0)
				printf("sceSifWriteBackDCache addr= 0x%08x size= %d, sector= %d\n",
				       readChainData[i].buffer, readChainData[i].sectors * sectorType,
				       readChainData[i].lbn);
			SifWriteBackDCache((void *) (readChainData[i].buffer),
					   readChainData[i].sectors * sectorType);
		}
	}

	SifWriteBackDCache(&curReadPos, 4);

	if (CdDebug > 0)
		printf("call sceCdReadChain cmd 2\n");
	CdCallbackNum = CD_NCMD_READCHAIN;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_READCHAIN, SIF_RPC_M_NOWAIT, readChainData, 788, NULL, 0, &_CdGenericCallbackFunction, (void*)&CdCallbackNum) < 0) {
		CdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (CdDebug > 0)
		printf("sceCdread end\n");
	SignalSema(nCmdSemaId);
	return 1;
}
#endif

// get the current read position (when reading using sceCdRead)
#ifdef F_sceCdGetReadPos
u32 sceCdGetReadPos(void)
{
	if (CdCallbackNum == CD_NCMD_READ) {
		return *(u32 *) UNCACHED_SEG(curReadPos);
	}
	return 0;
}
#endif

// **** Stream Functions ****


// start streaming data
// 
// args:        sector location to start streaming from
//                      mode to read in
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStStart
int sceCdStStart(u32 lbn, sceCdRMode * mode)
{
	streamStatus = 1;
	return sceCdStream(lbn, 0, NULL, CDVD_ST_CMD_START, mode);
}
#endif

// read stream data
// 
// args:        number of sectors to read
//                      read buffer address
//                      data stream mode (STMBLK oo CDVD_STREAM_NONBLOCK)
//                      error value holder
// returns:     number of sectors read if successful
//                      0 otherwise
#ifdef F_sceCdStRead
int sceCdStRead(u32 sectorType, u32 * buffer, u32 mode, u32 * error)
{
	int ret, i, err, sectorReadSize;

	*error = 0;
	if (CdDebug > 0)
		printf("sceCdStRead call read size=%d mode=%d\n", sectorType, mode);
	if (streamStatus == 0)
		return 0;
	SifWriteBackDCache(buffer, sectorType * 2048);

	// read only data currently in stream buffer
	if (mode == STMBLK) {
		ret = sceCdStream(0, sectorType, buffer, CDVD_ST_CMD_READ, &dummyMode);
		*error = ret >> 16;
		return ret & 0xFFFF;
	}
	// do block reads until all data is read, or error occurs
	for (i = 0; i < sectorType;) {
		ret = sceCdStream(0, sectorType - i, (buffer + (i * 2048)), CDVD_ST_CMD_READ, &dummyMode);
		sectorReadSize = ret & 0xFFFF;
		i += sectorReadSize;
		err = ret >> 16;

		if (err) {
			*error = err;
			if (CdDebug > 0)
				printf("sceCdStRead BLK Read cur_size= %d read_size= %d req_size= %d err 0x%x\n", i,
				       sectorReadSize, sectorType, err);

			if (sectorReadSize == 0)
				break;
		}
	}

	if (CdDebug > 0)
		printf("sceCdStRead BLK Read Ended\n");
	return i;
}
#endif

// stop streaming
// 
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStStop
int sceCdStStop(void)
{
	streamStatus = 0;
	return sceCdStream(0, 0, NULL, CDVD_ST_CMD_STOP, &dummyMode);
}
#endif

// seek to a new stream position
// 
// args:        sector location to start streaming from
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStSeek
int sceCdStSeek(u32 lbn)
{
	return sceCdStream(lbn, 0, NULL, CDVD_ST_CMD_SEEK, &dummyMode);
}
#endif

// init streaming
// 
// args:        stream buffer size
//                      number of ring buffers
//                      buffer address on iop
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStInit
int sceCdStInit(u32 buffSize, u32 numBuffers, void *buf)
{
	streamStatus = 0;
	return sceCdStream(buffSize, numBuffers, buf, CDVD_ST_CMD_INIT, &dummyMode);
}
#endif

// get stream read status
// 
// returns:     number of sectors read if successful
//                      0 otherwise
#ifdef F_sceCdStStat
int sceCdStStat(void)
{
	streamStatus = 0;
	return sceCdStream(0, 0, NULL, CDVD_ST_CMD_STAT, &dummyMode);
}
#endif

// pause streaming
// 
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStPause
int sceCdStPause(void)
{
	streamStatus = 0;
	return sceCdStream(0, 0, NULL, CDVD_ST_CMD_PAUSE, &dummyMode);
}
#endif

// continue streaming
// 
// returns:     1 if successful
//                      0 otherwise
#ifdef F_sceCdStResume
int sceCdStResume(void)
{
	streamStatus = 0;
	return sceCdStream(0, 0, NULL, CDVD_ST_CMD_RESUME, &dummyMode);
}
#endif

// perform the stream operation
#ifdef F_sceCdStream
int sceCdStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, sceCdRMode *rm)
{
	if (_CdCheckNCmd(15) == 0)
		return 0;

	if (CdDebug > 0)
		printf("call sceCdreadstm call\n");

	readStreamData[0] = lbn;
	readStreamData[1] = nsectors;
	readStreamData[2] = (u32)buf;
	readStreamData[3] = cmd;
	if (rm) {
		readStreamData[4] = (rm->trycount) | (rm->spindlctrl << 8) | (rm->datapattern << 16);
	}
	if (CdDebug > 0)
		printf("call sceCdreadstm cmd\n");

	if (SifCallRpc(&clientNCmd, CD_NCMD_STREAM, 0, readStreamData, 20, nCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (CdDebug > 0)
		printf("sceCdread end\n");
	SignalSema(nCmdSemaId);
	return *(int *)UNCACHED_SEG(nCmdRecvBuff);
}
#endif

#ifdef F_sceCdCddaStream
int sceCdCddaStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, sceCdRMode *rm)
{
	unsigned int sector_size;

	if (_CdCheckNCmd(17) == 0)
		return cmd < CDVD_ST_CMD_INIT ? -1 : 0;

	if (rm->datapattern == SCECdSecS2368)
		sector_size = 2368;
	else
		sector_size = 2352;

	readStreamData[0] = lbn;
	readStreamData[1] = nsectors * sector_size;
	readStreamData[2] = (u32)buf;
	readStreamData[3] = cmd;
	readStreamData[4] = (rm->trycount) | (rm->spindlctrl << 8) | (rm->datapattern << 16);

	if (cmd == CDVD_ST_CMD_INIT)
		readStreamData[2] = (u32)cdda_st_buf;

	*(u32 *)cdda_st_buf = 0;

	if (SifCallRpc(&clientNCmd, CD_NCMD_CDDASTREAM, 0, readStreamData, 20, nCmdRecvBuff, 4, NULL, NULL) < 0) {
		SignalSema(nCmdSemaId);
		return cmd < CDVD_ST_CMD_INIT ? -1 : 0;
	}

	SignalSema(nCmdSemaId);
	return *(int *)UNCACHED_SEG(nCmdRecvBuff);
}
#endif

// waits/checks for completion of n-commands
// 
// args:        mode:   0 = wait for completion of command (blocking)
//                      1 = check current status and return immediately
// returns:     0 if completed
//              1 if still executing command
#ifdef F_sceCdSync
int sceCdSync(int mode)
{
	// block till completed mode
	if (mode == 0) {
		if (CdDebug > 0)
			printf("N cmd wait\n");

		// wait till callback semaphore and client are ready
		while (cbSema || SifCheckStatRpc(&clientNCmd))
			;
		return 0;
	}
	// check status and return
	if (cbSema || SifCheckStatRpc(&clientNCmd))
		return 1;

	return 0;
}
#endif

// check whether ready to send an n-command
// 
// args:        current command
// returns:     1 if read to send
//                      0 if busy/error
#ifdef F__CdCheckNCmd
int _CdCheckNCmd(int cmd)
{
	int i;
	_CdSemaInit();
	if (PollSema(nCmdSemaId) != nCmdSemaId) {
		if (CdDebug > 0)
			printf("Ncmd fail sema cmd:%d keep_cmd:%d\n", cmd, nCmdNum);
		return 0;
	}

	nCmdNum = cmd;
	ReferThreadStatus(CdThreadId, &CdThreadParam);
	if (sceCdSync(1)) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	SifInitRpc(0);
	// if already bound, return ok
	if (bindNCmd >= 0)
		return 1;
	// bind rpc for n-commands
	while (1) {
		if (SifBindRpc(&clientNCmd, CD_SERVER_NCMD, 0) < 0) {
			if (CdDebug > 0)
				printf("Libcdvd bind err N CMD\n");
		}
		if (clientNCmd.server != 0)
			break;

		i = 0x10000;
		while (i--)
			;
	}

	bindNCmd = 0;
	return 1;
}
#endif

#ifdef F_sceCdReadKey
/* Retrieves the specified key from the currently inserted CD/DVD.
 *
 * Arguments:	arg1	- unknown
 *		arg2	- unknown
 *		command	- The command to execute.
 *		key	- A 16-byte buffer for containing the retrieved key.
 *
 * Returns:	1 if successful.
 *		0 if an error occurred.
*/
int sceCdReadKey(unsigned char arg1, unsigned char arg2, unsigned int command, unsigned char *key)
{
	int result;

	if(_CdCheckNCmd(CD_NCMD_READ_KEY)==0) return 0;

	((unsigned int *)nCmdSendBuff)[0]=arg1;
	((unsigned int *)nCmdSendBuff)[1]=arg2;
	((unsigned int *)nCmdSendBuff)[2]=command;

	if(SifCallRpc(&clientNCmd, CD_NCMD_READ_KEY, 0, nCmdSendBuff, 0xC, nCmdRecvBuff, 0x18, NULL, NULL)>=0){
		memcpy(key, UNCACHED_SEG(&nCmdRecvBuff[4]), 16);
		result=1;
	}
	else{
		SignalSema(nCmdSemaId);
		result=0;
	}

	return result;
}
#endif

