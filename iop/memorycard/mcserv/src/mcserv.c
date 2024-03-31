/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "mcserv.h"

// TODO: last sdk 3.1.0 and sdk 3.0.3 has MCSERV module version 0x2,0x10
// Check what was changed, and maybe port changes.
// Note: currently is based on the last XMCSERV from BOOTROM:
// 0x02,0x08
IRX_ID(MODNAME, 2, 8);

#ifdef BUILDING_XMCSERV
static const u8 XMCSERV_RpcCmd[2][16] =
{	// libmc rpc cmd values for XMCMAN/XMCSERV
	{	0xFE,	// CMD_INIT
		0x01,	// CMD_GETINFO
		0x02,	// CMD_OPEN
		0x03,	// CMD_CLOSE
		0x04,	// CMD_SEEK
		0x05,	// CMD_READ
		0x06,	// CMD_WRITE
		0x0A,	// CMD_FLUSH
		0x0C,	// CMD_CHDIR
		0x0D,	// CMD_GETDIR
		0x0E,	// CMD_SETFILEINFO
		0x0F,	// CMD_DELETE
		0x10,	// CMD_FORMAT
		0x11,	// CMD_UNFORMAT
		0x12,	// CMD_GETENTSPACE
		0x33	// CMD_CHECKBLOCK (calls xmcman_funcs: 45)
	},
	{ // corresponding internal rpc function
		0x00,	// sceMcInit
		0x12,	// sceMcGetInfo2
		0x01,	// sceMcOpen
		0x02,	// sceMcClose
		0x05,	// sceMcSeek
		0x11,	// sceMcRead2
		0x04,	// sceMcWrite
		0x0A,	// sceMcFlush
		0x0B,	// sceMcChDir
		0x06,	// sceMcGetDir
		0x0C,	// sceMcSetFileInfo
		0x09,	// sceMcDelete
		0x07,	// sceMcFormat
		0x10,	// sceMcUnformat
		0x13,	// sceMcGetEntSpace
		0x14	// sceMcCheckBlock (calls xmcman_funcs: 45)
	}
};
#endif

// rpc command handling array
static void *rpc_funcs_array[21] = {
    (void *)sceMcInit,
    (void *)sceMcOpen,
    (void *)sceMcClose,
    (void *)sceMcRead,
    (void *)sceMcWrite,
    (void *)sceMcSeek,
    (void *)sceMcGetDir,
    (void *)sceMcFormat,
    (void *)sceMcGetInfo,
    (void *)sceMcDelete,
    (void *)sceMcFlush,
    (void *)sceMcChDir,
    (void *)sceMcSetFileInfo,
    (void *)sceMcEraseBlock,
    (void *)sceMcReadPage,
    (void *)sceMcWritePage,
    (void *)sceMcUnformat,
    (void *)sceMcRead2,
#ifdef BUILDING_XMCSERV
    (void *)sceMcGetInfo2,
#else
    (void *)NULL,
#endif
#ifdef BUILDING_XMCSERV
    (void *)sceMcGetEntSpace,
#else
    (void *)NULL,
#endif
#ifdef BUILDING_XMCSERV
    (void *)sceMcCheckBlock,
#else
    (void *)NULL,
#endif
};

static int mcserv_tidS_0400;
static int mcserv_in_rpc_0400 = 0;

static SifRpcDataQueue_t mcserv_qdS_0400 __attribute__((aligned(16)));
static SifRpcServerData_t mcserv_sdS_0400 __attribute__((aligned(16)));

static u8 mcserv_rpc_buf[2048] __attribute__((__aligned__(4)));
static mcRpcStat_t rpc_stat __attribute__((aligned(16)));

#define MCSERV_BUFSIZE 8192
static u8 mcserv_buf[MCSERV_BUFSIZE] __attribute__((aligned(16)));

extern struct irx_export_table _exp_mcserv;

//--------------------------------------------------------------
int _start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
	iop_thread_t thread_param;
	register int thread_id;

	(void)argc;
	(void)argv;
	(void)startaddr;

#ifdef SIO_DEBUG
	sio_init(38400, 0, 0, 0, 0);
#endif
	DPRINTF("_start...\n");

	if (argc < 0)
	{
		int release_res;
		int state;

		if (mcserv_in_rpc_0400)
		{
			return MODULE_REMOVABLE_END;
		}

		CpuSuspendIntr(&state);
		release_res = ReleaseLibraryEntries(&_exp_mcserv);
		CpuResumeIntr(state);
		if (release_res == 0 || release_res == -213)
		{
			TerminateThread(mcserv_tidS_0400);
			DeleteThread(mcserv_tidS_0400);
			sceSifRemoveRpc(&mcserv_sdS_0400, &mcserv_qdS_0400);
			sceSifRemoveRpcQueue(&mcserv_qdS_0400);
			return MODULE_NO_RESIDENT_END;
		}
		return MODULE_REMOVABLE_END;
	}

	// Register mcserv dummy export table
	DPRINTF("registering exports...\n");
	if (RegisterLibraryEntries(&_exp_mcserv) != 0)
		goto err_out;

	CpuEnableIntr();

	DPRINTF("starting RPC thread...\n");
 	thread_param.attr = TH_C;
 	thread_param.thread = (void *)thread_rpc_S_0400;
 	thread_param.priority = 0x68;
 	thread_param.stacksize = 0x1000;
 	thread_param.option = 0;

	thread_id = CreateThread(&thread_param);
	mcserv_tidS_0400 = thread_id;

	StartThread(thread_id, 0);

	DPRINTF("_start returns MODULE_RESIDENT_END...\n");

	if (mi && ((mi->newflags & 2) != 0))
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;

err_out:

	DPRINTF("_start returns MODULE_NO_RESIDENT_END...\n");

	return MODULE_NO_RESIDENT_END;
}

//--------------------------------------------------------------
void thread_rpc_S_0400(void* arg)
{
	(void)arg;

	if (!sceSifCheckInit())
		sceSifInit();

	sceSifInitRpc(0);
	sceSifSetRpcQueue(&mcserv_qdS_0400, GetThreadId());
	sceSifRegisterRpc(&mcserv_sdS_0400, 0x80000400, (void *)cb_rpc_S_0400, &mcserv_rpc_buf, NULL, NULL, &mcserv_qdS_0400);
	sceSifRpcLoop(&mcserv_qdS_0400);
}

//--------------------------------------------------------------
void *cb_rpc_S_0400(u32 fno, void *buf, int size)
{
	// Rpc Callback function
	int (*rpc_func)(void);

	(void)buf;
	(void)size;

#ifdef BUILDING_XMCSERV
	{
		register int i;

		for (i=0; i<16; i++) { // retrieve correct function number for xmcserv
			if (fno == XMCSERV_RpcCmd[0][i]) {
				fno = XMCSERV_RpcCmd[1][i];
				break;
			}
		}
	}
#else
	{ // retrieve correct function number for mcserv
		fno -= 112;
		if (fno > 16)
			return (void *)&rpc_stat;
	}
#endif

	// Get function pointer
	rpc_func = (void *)rpc_funcs_array[fno];

	mcserv_in_rpc_0400 = 1;

	// Call needed rpc func
	rpc_stat.result = rpc_func();

	McReplaceBadBlock();

	mcserv_in_rpc_0400 = 0;

	return (void *)&rpc_stat;
}

//--------------------------------------------------------------
int sceMcInit(void)
{
	register int r;
#ifdef BUILDING_XMCSERV
	struct irx_id *ModuleInfo;
#endif

	r = _McInit(&mcserv_rpc_buf);

#ifdef BUILDING_XMCSERV
	{
		ModuleInfo = McGetModuleInfo();
		rpc_stat.mcserv_version = MODVER;
		rpc_stat.mcman_version = ModuleInfo->v;
	}
#endif

	return r;
}

//--------------------------------------------------------------
int sceMcOpen(void)
{
	return _McOpen(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcClose(void)
{
	return _McClose(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcRead(void)
{
	return _McRead(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcRead2(void)
{
	return _McRead2(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcWrite(void)
{
	return _McWrite(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcSeek(void)
{
	return _McSeek(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcGetDir(void)
{
	return _McGetDir(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcFormat(void)
{
	return _McFormat(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcUnformat(void)
{
	return _McUnformat(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcGetInfo(void)
{
	return _McGetInfo(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int sceMcGetInfo2(void)
{
	return _McGetInfo2(&mcserv_rpc_buf);
}
#endif

//--------------------------------------------------------------
int sceMcDelete(void)
{
	return _McDelete(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcFlush(void)
{
	return _McFlush(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcChDir(void)
{
	return _McChDir(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcEraseBlock(void)
{
	return _McEraseBlock(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcReadPage(void)
{
	return _McReadPage(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcWritePage(void)
{
	return _McWritePage(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
int sceMcSetFileInfo(void)
{
	return _McSetFileInfo(&mcserv_rpc_buf);
}

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int sceMcGetEntSpace(void)
{
	return _McGetEntSpace(&mcserv_rpc_buf);
}
#endif

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int sceMcCheckBlock(void)
{
	return _McCheckBlock(&mcserv_rpc_buf);
}
#endif

//--------------------------------------------------------------
int _McInit(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McInit fd %d offset %d\n", dP->fd, dP->offset);

#ifndef BUILDING_XMCSERV
	{
		int ps1flag = 0;

		if (dP->offset == -217)
			ps1flag = 1;

		McSetPS1CardFlag(ps1flag);
	}
#endif

	return McRetOnly(dP->fd);
}

//--------------------------------------------------------------
int _McOpen(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;

	DPRINTF("_McOpen port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);

	return McOpen(nP->port, nP->slot, nP->name, nP->flags);
}

//--------------------------------------------------------------
int _McClose(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McClose fd %d\n", dP->fd);

	return McClose(dP->fd);
}

//--------------------------------------------------------------
int _McSeek(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McSeek fd %d offset %d origin %d\n", dP->fd, dP->offset, dP->origin);

	return McSeek(dP->fd, dP->offset, dP->origin);
}

//--------------------------------------------------------------
int _McRead(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int file_offset, status;
	register int left_to_read, size_readed, size_to_read;
	mcEndParam_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
	void *eedata;

	DPRINTF("_McRead fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);

	eP.size1 = 0;
	eP.size2 = 0;
	eP.dest1 = NULL;
	eP.dest2 = NULL;

	size_readed = 0;
	file_offset = 0;

	eedata = dP->buffer;

	eP.size1 = dP->size;

	if (dP->size > 16)
		eP.size1 = (((int)(eedata)-1) & 0xfffffff0) - ((int)(eedata) - 16);

	eP.size2 = (dP->size - eP.size1) & 0x0f;
	left_to_read = (dP->size - eP.size1) - eP.size2;

	if (eP.size2 != 0)
		eP.dest2 = (void *)((u8 *)eedata + eP.size1 + left_to_read);

	if (eP.size1 != 0) {
		size_readed = McRead(dP->fd, eP.src1, eP.size1);

		if (size_readed < 0) {
			eP.size1 = 0;
			eP.size2 = 0;
			goto dma_transfer2;
		}
		else {
			file_offset = size_readed;
			eP.dest1 = eedata;
			eedata = (void *)((u8 *)eedata + size_readed);

			if (size_readed != eP.size1) {
				eP.size1 = size_readed;
				eP.size2 = 0;
				size_readed = 0;
				goto dma_transfer2;
			}
			else
				size_readed = 0;
		}
	}

	while (left_to_read > 0) {

		size_to_read = left_to_read;

		if (left_to_read > MCSERV_BUFSIZE)
			size_to_read = MCSERV_BUFSIZE;

		size_readed = McRead(dP->fd, mcserv_buf, size_to_read);

		if (size_readed < 0) {
			eP.size2 = 0;
			goto dma_transfer2;
		}

		dmaStruct.src = (void *)mcserv_buf;
		dmaStruct.dest = (void *)eedata;
		dmaStruct.size = size_readed;
		dmaStruct.attr = 0;

		CpuSuspendIntr(&intStatus);
		sceSifSetDma(&dmaStruct, 1);

		file_offset += size_readed;
		left_to_read -= size_readed;

		CpuResumeIntr(intStatus);

		eedata = (void *)((u8 *)eedata + size_readed);

		if (size_to_read != size_readed) {
			eP.size2 = 0;
			size_readed = 0;
			goto dma_transfer2;
		}
		size_readed = 0;
	}

	if (eP.size2 == 0)
		goto dma_transfer2;

	size_readed = McRead(dP->fd, eP.src2, eP.size2);

	if (size_readed < 0) {
		eP.size2 = 0;
		goto dma_transfer2;
	}

	file_offset += size_readed;
	eP.size2 = size_readed;
	size_readed = 0;

dma_transfer2:
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (mcEndParam_t);
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	if (size_readed != 0)
		return size_readed;

	return file_offset;
}

//--------------------------------------------------------------
int _McRead2(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int file_offset, status;
	register int left_to_read, size_readed, size_to_read;
	mcEndParam2_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
	void *eedata;

	DPRINTF("_McRead2 fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);

	eP.size1 = 0;
	eP.size2 = 0;
	eP.dest1 = NULL;
	eP.dest2 = NULL;

	size_readed = 0;
	file_offset = 0;

	eedata = dP->buffer;

	eP.size1 = dP->size;

	if (dP->size > 64)
		eP.size1 = (((u32)(eedata)-1) & 0xffffffc0) - ((u32)(eedata) - 64);

	eP.size2 = (dP->size - eP.size1) & 0x3f;
	left_to_read = (dP->size - eP.size1) - eP.size2;

	if (eP.size2 != 0)
		eP.dest2 = (void *)((u8 *)eedata + eP.size1 + left_to_read);

	if (eP.size1 != 0) {
		size_readed = McRead(dP->fd, eP.src1, eP.size1);

		if (size_readed < 0) {
			eP.size1 = 0;
			eP.size2 = 0;
			goto dma_transfer2;
		}
		else {
			file_offset = size_readed;
			eP.dest1 = eedata;
			eedata = (void *)((u8 *)eedata + size_readed);

			if (size_readed != eP.size1) {
				eP.size1 = size_readed;
				eP.size2 = 0;
				size_readed = 0;
				goto dma_transfer2;
			}
			else
				size_readed = 0;
		}
	}

	while (left_to_read > 0) {

		size_to_read = left_to_read;

		if (left_to_read > MCSERV_BUFSIZE)
			size_to_read = MCSERV_BUFSIZE;

		size_readed = McRead(dP->fd, mcserv_buf, size_to_read);

		if (size_readed < 0) {
			eP.size2 = 0;
			goto dma_transfer2;
		}

		if (size_readed == size_to_read) {
			dmaStruct.size = size_readed;
			goto dma_transfer;
		}

		eP.size2 = size_readed & 0x3f;
		if ((size_readed & 0x3f) != 0) {
			eP.dest2 = (void *)((u8 *)eedata + (size_readed & 0xffffffc0));
			memcpy(eP.src2, (void *)(mcserv_buf + (size_readed & 0xffffffc0)), size_readed & 0x3f);
		}

		if (eP.size2 == size_readed)
			goto skip_dma_transfer;

		dmaStruct.size = size_readed - eP.size2;

dma_transfer:
		dmaStruct.src = (void *)mcserv_buf;
		dmaStruct.dest = (void *)eedata;
		dmaStruct.attr = 0;

		CpuSuspendIntr(&intStatus);
		sceSifSetDma(&dmaStruct, 1);
		CpuResumeIntr(intStatus);

skip_dma_transfer:
		file_offset += size_readed;
		left_to_read -= size_readed;
		eedata = (void *)((u8 *)eedata + size_readed);

		if (size_to_read != size_readed) {
			size_readed = 0;
			goto dma_transfer2;
		}
		size_readed = 0;
	}

	if (eP.size2 == 0)
		goto dma_transfer2;

	size_readed = McRead(dP->fd, eP.src2, eP.size2);

	if (size_readed < 0) {
		eP.size2 = 0;
		goto dma_transfer2;
	}

	file_offset += size_readed;
	eP.size2 = size_readed;
	size_readed = 0;

dma_transfer2:
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (mcEndParam2_t);
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	if (size_readed != 0)
		return size_readed;

	return file_offset;
}

//--------------------------------------------------------------
int _McWrite(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	SifRpcReceiveData_t	rD;
	register int size_written;

	DPRINTF("_McWrite fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);

	size_written = 0;

	if (dP->origin != 0) {
		size_written = McWrite(dP->fd, dP->data, dP->origin);

		if (size_written != dP->origin)
			return size_written;
	}

	while (dP->size > 0) {
		register int size_to_write, r;

		size_to_write = dP->size;

		if (dP->size > MCSERV_BUFSIZE)
			size_to_write = MCSERV_BUFSIZE;

		sceSifGetOtherData(&rD, dP->buffer, &mcserv_buf, size_to_write, 0);

		r = McWrite(dP->fd, &mcserv_buf, size_to_write);
		if (r != size_to_write) {
			if (r < 0)
				return r;
			return r + size_written;
		}

		size_written += size_to_write;
		dP->size -= size_to_write;

		dP->buffer += size_to_write;
	}

	return size_written;
}

//--------------------------------------------------------------
int _McGetDir(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;
	register int status, file_entries, flags, r;
	SifDmaTransfer_t dmaStruct;
	int intStatus;

	DPRINTF("_McGetDir port%d slot%d dir %s flags %d maxent %d mcT addr %x\n", nP->port, nP->slot, nP->name, nP->flags, nP->maxent, (int)nP->mcT);

	status = 0;
	file_entries = 0;
	flags = nP->flags;

	nP->maxent--;

	while (nP->maxent > -1) {

		r = McGetDir(nP->port, nP->slot, nP->name, flags & 0xffff, 1, (sceMcTblGetDir *)mcserv_buf);
		if (r < 0)
			return r;
		if (r == 0)
			goto dma_wait;

		file_entries++;

		dmaStruct.src = (void *)mcserv_buf;
		dmaStruct.dest = (void *)nP->mcT;
		dmaStruct.size = sizeof (sceMcTblGetDir);
		dmaStruct.attr = 0;

		CpuSuspendIntr(&intStatus);
		status = sceSifSetDma(&dmaStruct, 1);
		CpuResumeIntr(intStatus);

		flags = 1;
		nP->mcT++;
		nP->maxent--;
	}

dma_wait:
	if (status == 0)
		return file_entries;

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	return file_entries;
}

//--------------------------------------------------------------
int _McChDir(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;
	register int status, r;
	SifDmaTransfer_t dmaStruct;
	int intStatus;

	DPRINTF("_McChDir port%d slot%d newdir %s\n", nP->port, nP->slot, nP->name);

	r = McChDir(nP->port, nP->slot, nP->name, (char *)mcserv_buf);

	dmaStruct.src = (void *)mcserv_buf;
	dmaStruct.dest = (void *)nP->curdir;
	dmaStruct.size = 1024;
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	return r;
}

//--------------------------------------------------------------
int _McFormat(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McFormat port%d slot%d\n", dP->port, dP->slot);

	return McFormat(dP->port, dP->slot);
}

//--------------------------------------------------------------
int _McUnformat(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McUnformat port%d slot%d\n", dP->port, dP->slot);

	return McUnformat(dP->port, dP->slot);
}

//--------------------------------------------------------------
int _McGetInfo(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int status, mc_free, r;
	mcEndParam_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;

	DPRINTF("_McGetInfo port%d slot%d\n", dP->port, dP->slot);

	mc_free = 0;

	r = McDetectCard(dP->port, dP->slot);

	if (dP->size > 0)
		eP.type = McGetMcType(dP->port, dP->slot);

	eP.free = 0;
	if (r >= -1) {
		if (dP->offset == 0)
			goto dma_transfer;

		mc_free = McGetFreeClusters(dP->port, dP->slot);
		if (mc_free >= 0)
			eP.free = mc_free;
	}

dma_transfer:

	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (mcEndParam_t);
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	if (mc_free < 0)
		return mc_free;

	return r;
}

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int _McGetInfo2(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int status, mc_free, r;
	mcEndParam2_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;

	DPRINTF("_McGetInfo2 port%d slot%d\n", dP->port, dP->slot);

	mc_free = 0;

	r = McDetectCard2(dP->port, dP->slot);

	if (dP->origin > 0)
		eP.type = McGetMcType(dP->port, dP->slot);

	if (r < -1) {
		eP.free = 0;
		eP.formatted = 0;
		goto dma_transfer;
	}

	if (dP->offset > 0) {
		mc_free = McGetFreeClusters(dP->port, dP->slot);

		if (mc_free < 0)
			eP.free = 0;
		else
			eP.free = mc_free;
	}

	if (dP->size > 0) {
		eP.formatted = 0;
		if (McGetFormat(dP->port, dP->slot) > 0)
			eP.formatted = 1;
	}

dma_transfer:

	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (mcEndParam2_t);
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	if (mc_free < 0)
		return mc_free;

	return r;
}
#endif

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int _McGetEntSpace(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;

	DPRINTF("_McGetEntSpace port%d slot%d dirname %s\n", nP->port, nP->slot, nP->name);

	return McGetEntSpace(nP->port, nP->slot, nP->name);
}
#endif

//--------------------------------------------------------------
int _McDelete(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;

	DPRINTF("_McDelete port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);

	return McDelete(nP->port, nP->slot, nP->name, nP->flags);
}

//--------------------------------------------------------------
int _McFlush(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McFlush fd %d\n", dP->fd);

	return McFlush(dP->fd);
}

//--------------------------------------------------------------
int _McEraseBlock(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int r;
	u8 eccbuf[16];

	DPRINTF("_McEraseBlock port%d slot%d offset %d\n", dP->port, dP->slot, dP->offset);

	dP->port = (dP->port & 1) + 2;

	if (McGetMcType(dP->port, dP->slot) == 2) {
		register int pagenum;

		r = McEraseBlock(dP->port, dP->offset, NULL, NULL);
		if (r != 0)
			return r;

		if (dP->origin == -1)
			return r;

		memset(mcserv_buf, dP->origin, 512);

		McDataChecksum(&mcserv_buf[0], &eccbuf[0]);
		McDataChecksum(&mcserv_buf[128], &eccbuf[3]);
		McDataChecksum(&mcserv_buf[256], &eccbuf[6]);
		McDataChecksum(&mcserv_buf[384], &eccbuf[9]);

		pagenum = 0;

		do {
			r = McWritePage(dP->port, dP->slot, (dP->offset << 4) + pagenum, mcserv_buf, eccbuf);

		} while (++pagenum < 16);   // <-- and the last page of the block ???

		return r;

	}

	memset(mcserv_buf, dP->origin, 128);

	r = McWritePS1PDACard(dP->port, dP->slot, dP->offset, mcserv_buf);

	return r;
}

//--------------------------------------------------------------
int _McReadPage(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	mcEndParam_t eP;
	register int status, fastsize, r, i, j;
	SifDmaTransfer_t dmaStruct;
	int intStatus;

	DPRINTF("_McReadPage port%d slot%d page %d\n", dP->port, dP->slot, dP->fd);

	eP.dest1 = dP->buffer;

	fastsize = ((u32)dP->buffer) & 0xf;
	dP->port = (dP->port & 1) + 2;

	if (McGetMcType(dP->port, dP->slot) == 2) {
		r = 0;
		McReadPage(dP->port, dP->slot, dP->fd, (void *)(mcserv_buf + fastsize));
	}
	else {
		memset((void *)(mcserv_buf + fastsize), 0, 512);
		r = McReadPS1PDACard(dP->port, dP->slot, dP->fd, (void *)(mcserv_buf + fastsize));
	}

	if (fastsize == 0)
		goto fullpage;

	i = 0;
	j = fastsize;
	while (j < 16) {
		eP.src1[i] = mcserv_buf[j];
		j++;
		i++;
	}

	j = 0;
	if (fastsize > 0) {
		while (j < 16) {
			eP.src1[i] = mcserv_buf[512 + j];
			j++;
			i++;
		}
	}

	dmaStruct.src = (void *)(mcserv_buf + 16);
	dmaStruct.dest = (void *)(dP->buffer - (fastsize - 16));
	dmaStruct.size = 496;
	dmaStruct.attr = 0;
	goto dma_transfer;

fullpage:
	dmaStruct.src = (void *)mcserv_buf;
	dmaStruct.dest = (void *)dP->buffer;
	dmaStruct.size = 512;
	dmaStruct.attr = 0;

dma_transfer:
	CpuSuspendIntr(&intStatus);
	sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (mcEndParam_t);
	dmaStruct.attr = 0;

	CpuSuspendIntr(&intStatus);
	status = sceSifSetDma(&dmaStruct, 1);
	CpuResumeIntr(intStatus);

	while (sceSifDmaStat(status) >= 0)
		DelayThread(100);

	return r;
}

//--------------------------------------------------------------
int _McWritePage(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;
	register int fastsize, i, j;
	SifRpcReceiveData_t	rD;
	u8 eccbuf[16];

	DPRINTF("_McWritePage port%d slot%d page %d\n", dP->port, dP->slot, dP->fd);

	fastsize = ((u32)dP->buffer) & 0xf;
	if (fastsize == 0)
		goto fullpage;

	sceSifGetOtherData(&rD, (void *)(dP->buffer - (fastsize - 16)), &mcserv_buf, 496, 0);

	i = fastsize;
	while (i < 16) {
		mcserv_buf[i] = dP->data[i - fastsize];
		i++;
	}

	if (fastsize == 0)
		goto ecc_calc;


	j = 16;
	i = 0;
	while (i < fastsize) {
		mcserv_buf[496 + j] = dP->data[j - fastsize];
		i++;
		j++;
	}

	goto ecc_calc;

fullpage:

	sceSifGetOtherData(&rD, dP->buffer, &mcserv_buf, 512, 0);

ecc_calc:

	McDataChecksum((void *)(mcserv_buf + fastsize), &eccbuf[0]);
	McDataChecksum(&mcserv_buf[128], &eccbuf[3]);
	McDataChecksum(&mcserv_buf[256], &eccbuf[6]);
	McDataChecksum(&mcserv_buf[384], &eccbuf[9]);

	return McWritePage((dP->port & 1) | 2, dP->slot, dP->fd, (void *)(mcserv_buf + fastsize), eccbuf);
}

//--------------------------------------------------------------
int _McSetFileInfo(void *rpc_buf)
{
	mcNameParam_t *nP = (mcNameParam_t *)rpc_buf;
	SifRpcReceiveData_t	rD;

	DPRINTF("_McSetFileInfo port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);

	sceSifGetOtherData(&rD, (void *)nP->mcT, &mcserv_buf, sizeof (sceMcTblGetDir), 0);

	return McSetFileInfo(nP->port, nP->slot, nP->name, (sceMcTblGetDir *)mcserv_buf, nP->flags);
}

//--------------------------------------------------------------
#ifdef BUILDING_XMCSERV
int _McCheckBlock(void *rpc_buf)
{
	mcDescParam_t *dP = (mcDescParam_t *)rpc_buf;

	DPRINTF("_McCheckBlock port%d slot%d block %d\n", dP->port, dP->slot, dP->offset);

	return McCheckBlock(dP->port, dP->slot, dP->offset);
}
#endif

//--------------------------------------------------------------
