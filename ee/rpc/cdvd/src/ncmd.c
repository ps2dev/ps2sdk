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
#
# Function definitions for libcdvd (EE side calls to the iop module cdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#
# NOTE: not all functions work with each set of modules!
*/

#include "stdio.h"
#include "kernel.h"
#include "sifrpc.h"
#include "libcdvd.h"
#include "string.h"

#include "internal.h"

#define CD_SERVER_NCMD			0x80000595	// non-blocking commands (Non-synchronous)

#define CD_NCMD_READ			0x01
#define CD_NCMD_CDDAREAD		0x02
#define CD_NCMD_DVDREAD			0x03
#define CD_NCMD_GETTOC			0x04
#define CD_NCMD_SEEK			0x05
#define CD_NCMD_STANDBY			0x06
#define CD_NCMD_STOP			0x07
#define CD_NCMD_PAUSE			0x08
#define CD_NCMD_STREAM			0x09
#define CD_NCMD_CDDASTREAM		0x0A
#define CD_NCMD_NCMD			0x0C
#define CD_NCMD_READIOPMEM		0x0D
#define CD_NCMD_DISKREADY		0x0E
#define CD_NCMD_READCHAIN		0x0F	// XCDVDFSV only

s32 cdCheckNCmd(s32 cmd);

s32 bindNCmd = -1;

SifRpcClientData_t clientNCmd __attribute__ ((aligned(64)));	// for n-cmds

s32 nCmdSemaId = -1;		// n-cmd semaphore id

s32 nCmdNum = 0;

u32 readStreamData[5] __attribute__ ((aligned(64)));
u32 readData[6] __attribute__ ((aligned(64)));
CdvdChain_t readChainData[66] __attribute__ ((aligned(64)));
u32 getTocSendBuff[3] __attribute__ ((aligned(64)));	// get toc
u32 _rd_intr_data[64] __attribute__ ((aligned(64)));
u32 curReadPos __attribute__ ((aligned(64)));
u8 tocBuff[2064] __attribute__ ((aligned(64)));	// toc buffer (for cdGetToc())
u8 nCmdRecvBuff[48] __attribute__ ((aligned(64)));
u8 nCmdSendBuff[48] __attribute__ ((aligned(64)));
s32 streamStatus = 0;
CdvdReadMode_t dummyMode;
u32 seekSector __attribute__ ((aligned(64)));
u8 cdda_st_buf[64] ALIGNED(64);

s32 cdNCmdDiskReady(void);

// **** N-Command Functions ****

struct _cdvd_read_data
{
	u32		size1;
	u32		size2;
	void	*dest1;
	void	*dest2;
	u32		src1;
	u32		src2;
};

// this gets called when the cdRead function finishes
// to copy the data read in to unaligned buffers
static void cdAlignReadBuffer(struct _cdvd_read_data *data)
{
	struct _cdvd_read_data *uncached = UNCACHED_SEG(data);
	
	if (uncached->size1 && uncached->dest1)	{
		memcpy(uncached->dest1, &uncached->src1, uncached->size1);
	}
	
	if (uncached->size2 && uncached->dest2)	{
		memcpy(uncached->dest2, &uncached->src2, uncached->size2);
	}
	
	cdCallback((void*)&cdCallbackNum);
}

// read data from cd
// non-blocking, requires cdSync() call
// 
// args:        sector location to start reading from
//                      number of sectors to read
//                      buffer to read to
//                      mode to read as
// returns: 1 if successful
//                      0 if error
s32 cdRead(u32 sectorLoc, u32 numSectors, void *buf, CdvdReadMode_t * mode)
{
	s32 bufSize;

	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_READ) == 0)
		return 0;

	readData[0] = sectorLoc;
	readData[1] = numSectors;
	readData[2] = (u32) buf;
	readData[3] = (mode->retries) | (mode->readSpeed << 8) | (mode->sectorType << 16);
	readData[4] = (u32) _rd_intr_data;
	readData[5] = (u32) &curReadPos;

	// work out buffer size
	if (mode->sectorType == CDVD_SECTOR_2328)
		bufSize = numSectors * 2328;
	else if (mode->sectorType == CDVD_SECTOR_2340)
		bufSize = numSectors * 2340;
	else
		bufSize = numSectors * 2048;

	curReadPos = 0;
	SifWriteBackDCache(buf, bufSize);
	SifWriteBackDCache(_rd_intr_data, 144);
	SifWriteBackDCache(readData, 24);
	SifWriteBackDCache(&curReadPos, 4);

	if (cdDebug > 0)
		printf("call cdread cmd\n");

	cdCallbackNum = CD_NCMD_READ;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_READ, SIF_RPC_M_NOWAIT, readData, 24, 0, 0,
				(void *) cdAlignReadBuffer, _rd_intr_data) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (cdDebug > 0)
		printf("cdread end\n");

	SignalSema(nCmdSemaId);
	return 1;
}

int cdDvdRead(u32 lbn, u32 nsectors, void *buf, CdvdReadMode_t *rm)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_DVDREAD) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = nsectors;
	readData[2] = (u32)buf;
	readData[3] = (rm->retries) | (rm->readSpeed << 8) | (rm->sectorType << 16);
	readData[4] = (u32)_rd_intr_data;

	SifWriteBackDCache(buf, nsectors * 2064);
	SifWriteBackDCache(_rd_intr_data, 144);
	SifWriteBackDCache(readData, 24);

	cdCallbackNum = CD_NCMD_DVDREAD;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_DVDREAD, SIF_RPC_M_NOWAIT, readData, 24,
				NULL, 0, NULL, NULL) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

int cdCddaRead(u32 lbn, u32 nsectors, void *buf, CdvdReadMode_t *rm)
{
	u32 sector_size;

	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_CDDAREAD) == 0)
		return 0;

	readData[0] = lbn;
	readData[1] = nsectors;
	readData[2] = (u32)buf;
	readData[3] = (rm->retries) | (rm->readSpeed << 8) | (rm->sectorType << 16);
	readData[4] = (u32)_rd_intr_data;

	/* Calculate the size of the read buffer.  */
	switch (rm->sectorType) {
		case CDVD_SECTOR_2368:
			sector_size = 2368;
			break;
		case CDVD_SECTOR_2448:
			sector_size = 2448;
			break;
		default:
			sector_size = 2352;
	}

	SifWriteBackDCache(buf, nsectors * sector_size);
	SifWriteBackDCache(_rd_intr_data, 144);
	SifWriteBackDCache(readData, 24);

	cdCallbackNum = CD_NCMD_CDDAREAD;
	cbSema = 1;

	if (SifCallRpc(&clientNCmd, CD_NCMD_CDDAREAD, SIF_RPC_M_NOWAIT, readData, 24,
				NULL, 0, (void *)cdAlignReadBuffer, _rd_intr_data) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

// get toc from inserted disc
// 
// args:        buffer to hold toc (1024 or 2064 bytes?)
// returns:     1 if successful
//                      0 otherwise
s32 cdGetToc(u8 * toc)
{
	u8 *tocPtr, *tocEnd;

	if (cdCheckNCmd(CD_NCMD_GETTOC) == 0)
		return 0;

	getTocSendBuff[0] = (u32) tocBuff;
	SifWriteBackDCache(tocBuff, 2064);
	SifWriteBackDCache(getTocSendBuff, 12);

	if (SifCallRpc(&clientNCmd, CD_NCMD_GETTOC, 0, getTocSendBuff, 12, nCmdRecvBuff, 8, 0, 0) < 0) {
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
	return *(s32 *) UNCACHED_SEG(nCmdRecvBuff);
}

// seek to given sector on disc
// non-blocking, requires cdSync() call
// 
// args:        sector to seek to on disc
// returns:     1 if successful
//                      0 if error
s32 cdSeek(u32 sectorLoc)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_SEEK) == 0)
		return 0;

	seekSector = sectorLoc;
	SifWriteBackDCache(&seekSector, 4);

	cdCallbackNum = CD_NCMD_SEEK;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_SEEK, SIF_RPC_M_NOWAIT, &seekSector, 4, 0, 0, cdCallback, (void*)&cdCallbackNum) <
	    0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

// puts ps2 cd drive into standby mode
// non-blocking, requires cdSync() call
// 
// returns:     1 if successful
//                      0 if error
s32 cdStandby(void)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_STANDBY) == 0)
		return 0;

	cdCallbackNum = CD_NCMD_STANDBY;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_STANDBY, SIF_RPC_M_NOWAIT, 0, 0, 0, 0, cdCallback, (void*)&cdCallbackNum) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

// stops ps2 cd drive from spinning
// non-blocking, requires cdSync() call
// 
// returns:     1 if successful
//                      0 if error
s32 cdStop(void)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_STOP) == 0)
		return 0;

	cdCallbackNum = CD_NCMD_STOP;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_STOP, SIF_RPC_M_NOWAIT, 0, 0, 0, 0, cdCallback, (void*)&cdCallbackNum) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

// pauses ps2 cd drive
// non-blocking, requires cdSync() call
// 
// returns:     1 if successful
//                      0 if error
s32 cdPause(void)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_PAUSE) == 0)
		return 0;

	cdCallbackNum = CD_NCMD_PAUSE;
	cbSema = 1;
	if (SifCallRpc(&clientNCmd, CD_NCMD_PAUSE, SIF_RPC_M_NOWAIT, 0, 0, 0, 0, cdCallback, (void*)&cdCallbackNum) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return 1;
}

// send an n-command by function number
// 
// args:        command number
//                      input buffer  (can be null)
//                      size of input buffer  (0 - 16 byte)
//                      output buffer (can be null)
//                      size of output buffer (0 - 16 bytes)
// returns:     1 if successful
//                      0 if error
s32 cdApplyNCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_NCMD) == 0)
		return 0;

	*(u16 *) & nCmdRecvBuff[0] = cmdNum;
	*(u16 *) & nCmdRecvBuff[2] = inBuffSize;
	memset(&nCmdRecvBuff[4], 0, 16);
	if (inBuff)
		memcpy(&nCmdRecvBuff[4], inBuff, inBuffSize);
	SifWriteBackDCache(nCmdRecvBuff, 20);

	if (SifCallRpc(&clientNCmd, CD_NCMD_NCMD, 0, nCmdRecvBuff, 20, nCmdRecvBuff, 16, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (outBuff)
		memcpy((void *) outBuff, UNCACHED_SEG(nCmdRecvBuff), outBuffSize);

	SignalSema(nCmdSemaId);
	return *(s32 *) UNCACHED_SEG(nCmdRecvBuff);
}

// read data to iop memory
// non-blocking, requires cdSync() call
// 
// args:        sector location to read from
//                      number of sectors to read
//                      buffer to read to (in iop memory)
//                      read mode
// returns:     1 if successful
//                      0 if error
s32 cdReadIOPMem(u32 sectorLoc, u32 numSectors, void *buf, CdvdReadMode_t * mode)
{
	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_READIOPMEM) == 0)
		return 0;

	readData[0] = sectorLoc;
	readData[1] = numSectors;
	readData[2] = (u32) buf;
	readData[3] = (mode->retries) | (mode->readSpeed << 8) | (mode->sectorType << 16);
	readData[4] = (u32) _rd_intr_data;
	readData[5] = (u32) & curReadPos;
	SifWriteBackDCache(readData, 24);

	cdCallbackNum = CD_NCMD_READIOPMEM;
	cbSema = 1;
	if (SifCallRpc
	    (&clientNCmd, CD_NCMD_READIOPMEM, SIF_RPC_M_NOWAIT, readData, 24, 0, 0, cdCallback, (void*)&cdCallbackNum) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (cdDebug > 0)
		printf("cdread end\n");

	SignalSema(nCmdSemaId);
	return 1;
}

// wait for disc to finish all n-commands
// (shouldnt really need to call this yourself)
// 
// returns:     6 if busy
//                      2 if ready
//                      0 if error
s32 cdNCmdDiskReady(void)
{
	if (cdCheckNCmd(CD_NCMD_DISKREADY) == 0)
		return 0;

	if (SifCallRpc(&clientNCmd, CD_NCMD_DISKREADY, 0, 0, 0, &nCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	SignalSema(nCmdSemaId);
	return *(s32 *) UNCACHED_SEG(nCmdRecvBuff);
}

// do a 'chain' of reads with one command
// last chain values must be all 0xFFFFFFFF
// (max of 64 reads can be set at once)
// non-blocking, requires cdSync() call
// 
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
// 
// args:        pointer to an array of read chain data
//                      read mode
// returns:     1 if successful
//                      0 if error
s32 cdReadChain(CdvdChain_t * readChain, CdvdReadMode_t * mode)
{
	s32 chainNum, i, sectorType;

	if (cdNCmdDiskReady() == CDVD_READY_NOTREADY)
		return 0;
	if (cdCheckNCmd(CD_NCMD_READCHAIN) == 0)
		return 0;

	if (cdDebug > 0)
		printf("call cdReadChain cmd 0\n");

	for (chainNum = 0; chainNum < 64; chainNum++) {
		if (readChain[chainNum].sectorLoc == 0xFFFFFFFF ||
		    readChain[chainNum].numSectors == 0xFFFFFFFF || readChain[chainNum].buffer == 0xFFFFFFFF)
			break;
		readChainData[chainNum].sectorLoc = readChain[chainNum].sectorLoc;
		readChainData[chainNum].numSectors = readChain[chainNum].numSectors;
		readChainData[chainNum].buffer = readChain[chainNum].buffer;
	}
	// store 'end of read-chain' data in chain
	readChainData[chainNum].sectorLoc = 0xFFFFFFFF;
	readChainData[chainNum].numSectors = 0xFFFFFFFF;
	readChainData[chainNum].buffer = 0xFFFFFFFF;

	// store read mode and read position in read chain data
	readChainData[65].sectorLoc = (mode->retries) | (mode->readSpeed << 8) | (mode->sectorType << 16);
	readChainData[65].numSectors = (u32) & curReadPos;

	if (mode->sectorType == CDVD_SECTOR_2328)
		sectorType = 2328;
	else if (mode->sectorType == CDVD_SECTOR_2340)
		sectorType = 2340;
	else
		sectorType = 2048;

	curReadPos = 0;
	if (cdDebug > 0)
		printf("call cdReadChain cmd 1\n");

	for (i = 0; i < chainNum; i++) {
		// if memory is on EE, make sure its not cached
		if ((readChainData[i].buffer & 1) == 0) {
			if (cdDebug > 0)
				printf("sceSifWriteBackDCache addr= 0x%08x size= %d, sector= %d\n",
				       readChainData[i].buffer, readChainData[i].numSectors * sectorType,
				       readChainData[i].sectorLoc);
			SifWriteBackDCache((void *) (readChainData[i].buffer),
					   readChainData[i].numSectors * sectorType);
		}
	}

	SifWriteBackDCache(readChainData, 24);
	SifWriteBackDCache(&curReadPos, 4);

	if (cdDebug > 0)
		printf("call cdReadChain cmd 2\n");
	cdCallbackNum = CD_NCMD_READCHAIN;
	cbSema = 1;
	if (SifCallRpc
	    (&clientNCmd, CD_NCMD_READCHAIN, SIF_RPC_M_NOWAIT, readChainData, 788, 0, 0, cdCallback,
	     (void*)&cdCallbackNum) < 0) {
		cdCallbackNum = 0;
		cbSema = 0;
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (cdDebug > 0)
		printf("cdread end\n");
	SignalSema(nCmdSemaId);
	return 1;
}

// get the current read position (when reading using cdRead)
u32 cdGetReadPos(void)
{
	if (cdCallbackNum == CD_NCMD_READ) {
		return *(u32 *) UNCACHED_SEG(curReadPos);
	}
	return 0;
}

// **** Stream Functions ****


// start streaming data
// 
// args:        sector location to start streaming from
//                      mode to read in
// returns:     1 if successful
//                      0 otherwise
s32 cdStStart(u32 sectorLoc, CdvdReadMode_t * mode)
{
	streamStatus = 1;
	return cdStream(sectorLoc, 0, NULL, CDVD_ST_CMD_START, mode);
}

// read stream data
// 
// args:        number of sectors to read
//                      read buffer address
//                      data stream mode (CDVD_STREAM_BLOCK oo CDVD_STREAM_NONBLOCK)
//                      error value holder
// returns:     number of sectors read if successful
//                      0 otherwise
s32 cdStRead(u32 sectorType, u32 * buffer, u32 mode, u32 * error)
{
	s32 ret, i, err, sectorReadSize;

	*error = 0;
	if (cdDebug > 0)
		printf("sceCdStRead call read size=%d mode=%d\n", sectorType, mode);
	if (streamStatus == 0)
		return 0;
	SifWriteBackDCache(buffer, sectorType * 2048);

	// read only data currently in stream buffer
	if (mode == CDVD_STREAM_BLOCK) {
		ret = cdStream(0, sectorType, buffer, CDVD_ST_CMD_READ, &dummyMode);
		*error = ret >> 16;
		return ret & 0xFFFF;
	}
	// do block reads until all data is read, or error occurs
	for (i = 0; i < sectorType;) {
		ret = cdStream(0, sectorType - i, (buffer + (i * 2048)), CDVD_ST_CMD_READ, &dummyMode);
		sectorReadSize = ret & 0xFFFF;
		i += sectorReadSize;
		err = ret >> 16;

		if (err) {
			*error = err;
			if (cdDebug > 0)
				printf("sceCdStRead BLK Read cur_size= %d read_size= %d req_size= %d err 0x%x\n", i,
				       sectorReadSize, sectorType, err);

			if (sectorReadSize == 0)
				break;
		}
	}

	if (cdDebug > 0)
		printf("sceCdStRead BLK Read Ended\n");
	return i;
}

// stop streaming
// 
// returns:     1 if successful
//                      0 otherwise
s32 cdStStop(void)
{
	streamStatus = 0;
	return cdStream(0, 0, NULL, CDVD_ST_CMD_STOP, &dummyMode);
}

// seek to a new stream position
// 
// args:        sector location to start streaming from
// returns:     1 if successful
//                      0 otherwise
s32 cdStSeek(u32 sectorLoc)
{
	return cdStream(sectorLoc, 0, NULL, CDVD_ST_CMD_SEEK, &dummyMode);
}

// init streaming
// 
// args:        stream buffer size
//                      number of ring buffers
//                      buffer address on iop
// returns:     1 if successful
//                      0 otherwise
s32 cdStInit(u32 buffSize, u32 numBuffers, void *buf)
{
	streamStatus = 0;
	return cdStream(buffSize, numBuffers, buf, CDVD_ST_CMD_INIT, &dummyMode);
}

// get stream read status
// 
// returns:     number of sectors read if successful
//                      0 otherwise
s32 cdStStat(void)
{
	streamStatus = 0;
	return cdStream(0, 0, NULL, CDVD_ST_CMD_STAT, &dummyMode);
}

// pause streaming
// 
// returns:     1 if successful
//                      0 otherwise
s32 cdStPause(void)
{
	streamStatus = 0;
	return cdStream(0, 0, NULL, CDVD_ST_CMD_PAUSE, &dummyMode);
}

// continue streaming
// 
// returns:     1 if successful
//                      0 otherwise
s32 cdStResume(void)
{
	streamStatus = 0;
	return cdStream(0, 0, NULL, CDVD_ST_CMD_RESUME, &dummyMode);
}

// perform the stream operation
int cdStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, CdvdReadMode_t *rm)
{
	if (cdCheckNCmd(15) == 0)
		return 0;

	if (cdDebug > 0)
		printf("call cdreadstm call\n");

	readStreamData[0] = lbn;
	readStreamData[1] = nsectors;
	readStreamData[2] = (u32)buf;
	readStreamData[3] = cmd;
	if (rm) {
		readStreamData[4] = (rm->retries) | (rm->readSpeed << 8) | (rm->sectorType << 16);
	}
	if (cdDebug > 0)
		printf("call cdreadstm cmd\n");

	SifWriteBackDCache(readStreamData, 20);
	if (SifCallRpc(&clientNCmd, CD_NCMD_STREAM, 0, readStreamData, 20, nCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return 0;
	}

	if (cdDebug > 0)
		printf("cdread end\n");
	SignalSema(nCmdSemaId);
	return *(int *)UNCACHED_SEG(nCmdRecvBuff);
}

int cdCddaStream(u32 lbn, u32 nsectors, void *buf, CdvdStCmd_t cmd, CdvdReadMode_t *rm)
{
	u32 sector_size;

	if (cdCheckNCmd(17) == 0)
		return cmd < CDVD_ST_CMD_INIT ? -1 : 0;

	if (rm->sectorType == CDVD_SECTOR_2368)
		sector_size = 2368;
	else
		sector_size = 2352;

	readStreamData[0] = lbn;
	readStreamData[1] = nsectors * sector_size;
	readStreamData[2] = (u32)buf;
	readStreamData[3] = cmd;
	readStreamData[4] = (rm->retries) | (rm->readSpeed << 8) | (rm->sectorType << 16);

	if (cmd == CDVD_ST_CMD_INIT)
		readStreamData[2] = (u32)cdda_st_buf;

	*(u32 *)cdda_st_buf = 0;

	if (SifCallRpc(&clientNCmd, CD_NCMD_CDDASTREAM, 0, readStreamData, 20, nCmdRecvBuff, 4, 0, 0) < 0) {
		SignalSema(nCmdSemaId);
		return cmd < CDVD_ST_CMD_INIT ? -1 : 0;
	}

	SignalSema(nCmdSemaId);
	return *(int *)UNCACHED_SEG(nCmdRecvBuff);
}


// waits/checks for completion of n-commands
// 
// args:        mode:   0 = wait for completion of command (blocking)
//                      1 = check current status and return immediately
// returns:     0 if completed
//              1 if still executing command
s32 cdSync(s32 mode)
{
	// block till completed mode
	if (mode == 0) {
		if (cdDebug > 0)
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

// check whether ready to send an n-command
// 
// args:        current command
// returns:     1 if read to send
//                      0 if busy/error
s32 cdCheckNCmd(s32 cmd)
{
	s32 i;
	cdSemaInit();
	if (PollSema(nCmdSemaId) != nCmdSemaId) {
		if (cdDebug > 0)
			printf("Ncmd fail sema cmd:%d keep_cmd:%d\n", cmd, nCmdNum);
		return 0;
	}

	nCmdNum = cmd;
	ReferThreadStatus(cdThreadId, &cdThreadParam);
	if (cdSync(1)) {
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
			if (cdDebug > 0)
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
