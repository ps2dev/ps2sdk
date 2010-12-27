/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2009 jimmikaelkael
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: mcserv.c 1410 2009-01-18 15:24:54Z jimmikaelkael $
*/

#include "mcserv.h"

IRX_ID(MODNAME, 2, 8);

#ifdef SIO_DEBUG
	#include <sior.h>
	#define DEBUG
	#define DPRINTF(args...)	sio_printf(args)
#else
	#define DPRINTF(args...)	printf(args)
#endif

struct irx_export_table _exp_mcserv;

//-------------------------------------------------------------- 
int _start(int argc, const char **argv)
{
	iop_thread_t thread_param;
	register int thread_id;	
	iop_library_table_t *libtable;
	iop_library_t *libptr;
	int i, mcman_loaded;
	void **export_tab;

#ifdef SIO_DEBUG
	sio_init(38400, 0, 0, 0, 0);
#endif
#ifdef DEBUG
	DPRINTF("mcserv: _start...\n");
#endif
	
	// Get mcman lib ptr
	mcman_loaded = 0;
	libtable = GetLibraryEntryTable();
	libptr = libtable->tail;
	while (libptr != 0) {
		for (i=0; i<8; i++) {
			if (libptr->name[i] != mcman_modname[i])
				break;
		} 
		if (i == 8) {
			mcman_loaded = 1;
			break;
		}
		libptr = libptr->prev;	
	}
	
	if (!mcman_loaded) {
#ifdef DEBUG		
		DPRINTF("mcserv: mcman module is not loaded...\n"); 		
#endif		
		goto err_out;
	}

#ifdef DEBUG
	DPRINTF("mcserv: mcman version=%03x\n", libptr->version);
#endif	
	if (libptr->version > 0x200)
		mcman_type = XMCMAN;
	
	// Get mcman export table	 
	export_tab = (void **)(((struct irx_export_table *)libptr)->fptrs);
		
	// Set functions pointers to match MCMAN exports
	McDetectCard = export_tab[5];
	McOpen = export_tab[6];
	McClose = export_tab[7];
	McRead = export_tab[8];
	McWrite = export_tab[9];
	McSeek = export_tab[10];
	McFormat = export_tab[11];
	McGetDir = export_tab[12];
	McDelete = export_tab[13];
	McFlush = export_tab[14];
	McChDir = export_tab[15];
	McSetFileInfo = export_tab[16];
	McEraseBlock = export_tab[17];
	McReadPage = export_tab[18];
	McWritePage = export_tab[19];
	McDataChecksum = export_tab[20];
	McReplaceBadBlock = export_tab[24]; // dummy in MCMAN !!!
	McReadPS1PDACard = export_tab[29];
	McWritePS1PDACard = export_tab[30];
	McUnformat = export_tab[36];
	McRetOnly = export_tab[37];
	McGetFreeClusters = export_tab[38];
	McGetMcType = export_tab[39];
	McSetPS1CardFlag = export_tab[40];

	// Set functions pointers to match XMCMAN exports if needed	
	if (mcman_type == XMCMAN) {
	McDetectCard2 = export_tab[21];
	McGetFormat = export_tab[22];
	McGetEntSpace = export_tab[23];
	McGetModuleInfo = export_tab[42];
	McCheckBlock = export_tab[45];
	}	
		
	// Register mcserv dummy export table
#ifdef DEBUG
	DPRINTF("mcserv: registering exports...\n");
#endif	
	if (RegisterLibraryEntries(&_exp_mcserv) != 0)
		goto err_out;
	
	CpuEnableIntr();	

#ifdef DEBUG
	DPRINTF("mcserv: starting RPC thread...\n");
#endif		
 	thread_param.attr = TH_C;
 	thread_param.thread = (void *)thread_rpc_S_0400;
 	thread_param.priority = 0x68;
 	thread_param.stacksize = 0x1000;
 	thread_param.option = 0;
			
	thread_id = CreateThread(&thread_param);
	mcserv_tidS_0400 = thread_id;
		
	StartThread(thread_id, 0);		

#ifdef DEBUG
	DPRINTF("mcserv: _start returns MODULE_RESIDENT_END...\n");
#endif		

	return MODULE_RESIDENT_END;
	
err_out:

#ifdef DEBUG
	DPRINTF("mcserv: _start returns MODULE_NO_RESIDENT_END...\n");
#endif		

	return MODULE_NO_RESIDENT_END;	
}

//--------------------------------------------------------------
void thread_rpc_S_0400(void* arg)
{
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
	
	register int i;
	
	if (mcman_type == XMCMAN) {
		for (i=0; i<16; i++) { // retrieve correct function number for xmcserv
			if (fno == XMCSERV_RpcCmd[0][i]) {
				fno = XMCSERV_RpcCmd[1][i];
				break;
			}
		}
	}
	else { // retrieve correct function number for mcserv
		fno -= 112;
		if (fno > 16)
			return (void *)&rpc_stat;
	}

	// Get function pointer
	rpc_func = (void *)rpc_funcs_array[fno];
	
	// Call needed rpc func
	rpc_stat.rpc_func_ret = rpc_func();
	
	McReplaceBadBlock();

	return (void *)&rpc_stat;
}

//-------------------------------------------------------------- 
int sceMcInit(void)
{
	register int r;
	struct modInfo_t *ModuleInfo;
	
	r = _McInit(&mcserv_rpc_buf);
	
	if (mcman_type == XMCMAN) {
		ModuleInfo = McGetModuleInfo();
		rpc_stat.mcserv_version = MODVER;
		rpc_stat.mcman_version = ModuleInfo->version;
	}
	
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
int sceMcGetInfo2(void)
{
	return _McGetInfo2(&mcserv_rpc_buf);
}

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
int sceMcGetEntSpace(void)
{
	return _McGetEntSpace(&mcserv_rpc_buf);
}

//-------------------------------------------------------------- 
int sceMcCheckBlock(void)
{
	return _McCheckBlock(&mcserv_rpc_buf);
}

//-------------------------------------------------------------- 
int _McInit(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	int ps1flag = 0;

#ifdef DEBUG	
	DPRINTF("mcserv: _McInit fd %d offset %d\n", dP->fd, dP->offset);
#endif
		
	if (mcman_type == MCMAN) {
		if (dP->offset == 217)
			ps1flag = 1;
	
		McSetPS1CardFlag(ps1flag);
	}
	
	return McRetOnly(dP->fd);	
}

//-------------------------------------------------------------- 
int _McOpen(void *rpc_buf)
{
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McOpen port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);
#endif
	
	return McOpen(nP->port, nP->slot, nP->name, nP->flags);
}

//-------------------------------------------------------------- 
int _McClose(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McClose fd %d\n", dP->fd);
#endif
	
	return McClose(dP->fd);
}

//-------------------------------------------------------------- 
int _McSeek(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;

#ifdef DEBUG	
	DPRINTF("mcserv: _McSeek fd %d offset %d origin %d\n", dP->fd, dP->offset, dP->origin);
#endif
		
	return McSeek(dP->fd, dP->offset, dP->origin);
}

//-------------------------------------------------------------- 
int _McRead(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int file_offset, status;
	register int left_to_read, size_readed, size_to_read;
	g_endParam_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
	void *eedata;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McRead fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);
#endif	

	eP.fastIO1_size = 0;
	eP.fastIO2_size = 0;
	eP.fastIO1_eeaddr = NULL;
	eP.fastIO2_eeaddr = NULL;
		
	size_readed = 0;
	file_offset = 0;
	
	eedata = dP->buffer;
	
	eP.fastIO1_size = dP->size;
		
	if (dP->size > 16)
		eP.fastIO1_size = (((int)(eedata)-1) & 0xfffffff0) - ((int)(eedata) - 16);
			
	eP.fastIO2_size = (dP->size - eP.fastIO1_size) & 0x0f;
	left_to_read = (dP->size - eP.fastIO1_size) - eP.fastIO2_size;

	if (eP.fastIO2_size != 0)
		eP.fastIO2_eeaddr = (void *)(eedata + eP.fastIO1_size + left_to_read);
	
	if (eP.fastIO1_size != 0) {
		size_readed = McRead(dP->fd, eP.fastIO1_data, eP.fastIO1_size);

		if (size_readed < 0) {
			eP.fastIO1_size = 0;
			eP.fastIO2_size = 0;
			goto dma_transfer2;
		}
		else {
			file_offset = size_readed;
			eP.fastIO1_eeaddr = eedata;
			eedata += size_readed;
			
			if (size_readed != eP.fastIO1_size) {
				eP.fastIO1_size = size_readed;
				eP.fastIO2_size = 0;
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
			eP.fastIO2_size = 0;
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
		
		eedata += size_readed;
				
		if (size_to_read != size_readed) {
			eP.fastIO2_size = 0;
			size_readed = 0;
			goto dma_transfer2;
		}
		size_readed = 0;
	}
	
	if (eP.fastIO2_size == 0)
		goto dma_transfer2;
	
	size_readed = McRead(dP->fd, eP.fastIO2_data, eP.fastIO2_size);				
	
	if (size_readed < 0) {
		eP.fastIO2_size = 0;
		goto dma_transfer2;
	}
	
	file_offset += size_readed;
	eP.fastIO2_size = size_readed;
	size_readed = 0;
	
dma_transfer2:
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (g_endParam_t);
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
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int file_offset, status;
	register int left_to_read, size_readed, size_to_read;
	g_endParam2_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
	void *eedata;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McRead2 fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);
#endif	

	eP.fastIO1_size = 0;
	eP.fastIO2_size = 0;
	eP.fastIO1_eeaddr = NULL;
	eP.fastIO2_eeaddr = NULL;
		
	size_readed = 0;
	file_offset = 0;
	
	eedata = dP->buffer;
	
	eP.fastIO1_size = dP->size;
		
	if (dP->size > 64)
		eP.fastIO1_size = (((u32)(eedata)-1) & 0xffffffc0) - ((u32)(eedata) - 64);
	
	eP.fastIO2_size = (dP->size - eP.fastIO1_size) & 0x3f;
	left_to_read = (dP->size - eP.fastIO1_size) - eP.fastIO2_size;
	
	if (eP.fastIO2_size != 0)
		eP.fastIO2_eeaddr = (void *)(eedata + eP.fastIO1_size + left_to_read);
	
	if (eP.fastIO1_size != 0) {
		size_readed = McRead(dP->fd, eP.fastIO1_data, eP.fastIO1_size);

		if (size_readed < 0) {
			eP.fastIO1_size = 0;
			eP.fastIO2_size = 0;
			goto dma_transfer2;
		}
		else {
			file_offset = size_readed;
			eP.fastIO1_eeaddr = eedata;
			eedata += size_readed;
			
			if (size_readed != eP.fastIO1_size) {
				eP.fastIO1_size = size_readed;
				eP.fastIO2_size = 0;
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
			eP.fastIO2_size = 0;
			goto dma_transfer2;
		}
				
		if (size_readed == size_to_read) {
			dmaStruct.size = size_readed;
			goto dma_transfer;
		}
		
		eP.fastIO2_size = size_readed & 0x3f;
		if ((size_readed & 0x3f) != 0) {
			eP.fastIO2_eeaddr = (void *)(eedata + (size_readed & 0xffffffc0));
			memcpy(eP.fastIO2_data, (void *)(mcserv_buf + (size_readed & 0xffffffc0)), size_readed & 0x3f);
		}
			
		if (eP.fastIO2_size == size_readed)
			goto skip_dma_transfer;
			
		dmaStruct.size = size_readed - eP.fastIO2_size;
		
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
		eedata += size_readed;
						
		if (size_to_read != size_readed) {
			size_readed = 0;
			goto dma_transfer2;
		}
		size_readed = 0;
	}
	
	if (eP.fastIO2_size == 0)
		goto dma_transfer2;
	
	size_readed = McRead(dP->fd, eP.fastIO2_data, eP.fastIO2_size);				
	
	if (size_readed < 0) {
		eP.fastIO2_size = 0;
		goto dma_transfer2;
	}
	
	file_offset += size_readed;
	eP.fastIO2_size = size_readed;
	size_readed = 0;
	
dma_transfer2:
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (g_endParam2_t);
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
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	SifRpcReceiveData_t	rD;
	register int size_to_write, size_written, r;
	
#ifdef DEBUG		
	DPRINTF("mcserv: _McWrite fd %d ee buffer addr %x size %d\n", dP->fd, (int)dP->buffer, dP->size);
#endif	
	
	size_written = 0;
	
	if (dP->origin != 0) {
		size_written = McWrite(dP->fd, dP->data, dP->origin);
		
		if (size_written != dP->origin)
			return size_written;
	}

	while (dP->size > 0) {
			
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
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	register int status, file_entries, flags, r;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McGetDir port%d slot%d dir %s flags %d maxent %d mcT addr %x\n", nP->port, nP->slot, nP->name, nP->flags, nP->maxent, (int)nP->mcT);
#endif	

	status = 0;
	file_entries = 0;
	flags = nP->flags;
	
	nP->maxent--;
	
	while (nP->maxent > -1) {	
		
		r = McGetDir(nP->port, nP->slot, nP->name, flags & 0xffff, 1, (mcTable_t *)mcserv_buf);
		if (r < 0)
			return r;
		if (r == 0)
			goto dma_wait;
		
		file_entries++;
	
		dmaStruct.src = (void *)mcserv_buf;
		dmaStruct.dest = (void *)nP->mcT;
		dmaStruct.size = sizeof (mcTable_t);
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
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	register int status, r;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McChDir port%d slot%d newdir %s\n", nP->port, nP->slot, nP->name);
#endif	

	r = McChDir(nP->port, nP->slot, nP->name, (char *)mcserv_buf); 
	
	dmaStruct.src = (void *)mcserv_buf;
	dmaStruct.dest = (void *)nP->mcT;
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
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;

#ifdef DEBUG	
	DPRINTF("mcserv: _McFormat port%d slot%d\n", dP->port, dP->slot);
#endif
		
	return McFormat(dP->port, dP->slot);
}

//-------------------------------------------------------------- 
int _McUnformat(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;

#ifdef DEBUG	
	DPRINTF("mcserv: _McUnformat port%d slot%d\n", dP->port, dP->slot);
#endif
		
	return McUnformat(dP->port, dP->slot);
}

//-------------------------------------------------------------- 
int _McGetInfo(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int status, mc_free, r;
	g_endParam_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McGetInfo port%d slot%d\n", dP->port, dP->slot);
#endif	

	mc_free = 0;
	
	r = McDetectCard(dP->port, dP->slot);

	if (dP->size > 0)
		eP.fastIO1_size = McGetMcType(dP->port, dP->slot);
	
	eP.fastIO2_size = 0;	
	if (r >= -1) {
		if (dP->offset == 0)
			goto dma_transfer; 
			
		mc_free = McGetFreeClusters(dP->port, dP->slot);	
		if (mc_free >= 0)
			eP.fastIO2_size = mc_free;
	}	

dma_transfer:
	
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (g_endParam_t);
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
int _McGetInfo2(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int status, mc_free, r;
	g_endParam2_t eP;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McGetInfo2 port%d slot%d\n", dP->port, dP->slot);
#endif	

	mc_free = 0;
	
	r = McDetectCard2(dP->port, dP->slot);
	
	if (dP->origin > 0)
		eP.fastIO1_size = McGetMcType(dP->port, dP->slot);
	
	if (r < -1) {
		eP.fastIO2_size = 0;
		eP.flag = 0;
		goto dma_transfer; 
	}	
	
	if (dP->offset > 0) {	
		mc_free = McGetFreeClusters(dP->port, dP->slot);
		
		if (mc_free < 0)
			eP.fastIO2_size = 0;
		else	
			eP.fastIO2_size = mc_free;
	}
	
	if (dP->size > 0) {
		eP.flag = 0;
		if (McGetFormat(dP->port, dP->slot) > 0)
			eP.flag = 1;
	}

dma_transfer:
	
	dmaStruct.src = (void *)&eP;
	dmaStruct.dest = (void *)dP->param;
	dmaStruct.size = sizeof (g_endParam2_t);
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
int _McGetEntSpace(void *rpc_buf)
{
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McGetEntSpace port%d slot%d dirname %s\n", nP->port, nP->slot, nP->name);
#endif
	
	return McGetEntSpace(nP->port, nP->slot, nP->name);
}

//-------------------------------------------------------------- 
int _McDelete(void *rpc_buf)
{
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McDelete port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);
#endif
	
	return McDelete(nP->port, nP->slot, nP->name, nP->flags);
}

//-------------------------------------------------------------- 
int _McFlush(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;

#ifdef DEBUG	
	DPRINTF("mcserv: _McFlush fd %d\n", dP->fd);
#endif
		
	return McFlush(dP->fd);
}

//-------------------------------------------------------------- 
int _McEraseBlock(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int pagenum, r;
	u8 eccbuf[16];
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McEraseBlock port%d slot%d offset %d\n", dP->port, dP->slot, dP->offset);
#endif	

	dP->port = (dP->port & 1) + 2;
		
	if (McGetMcType(dP->port, dP->slot) == 2) {
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
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	g_endParam_t eP;
	register int status, fastsize, r, i, j;
	SifDmaTransfer_t dmaStruct;
	int intStatus;
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McReadPage port%d slot%d page %d\n", dP->port, dP->slot, dP->fd);
#endif	

	eP.fastIO1_eeaddr = dP->buffer;
	
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
		eP.fastIO1_data[i] = mcserv_buf[j];			
		j++;		
		i++;
	}
	
	j = 0;
	if (fastsize > 0) {
		while (j < 16) {
			eP.fastIO1_data[i] = mcserv_buf[512 + j];			
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
	status = sceSifSetDma(&dmaStruct, 1); 
	CpuResumeIntr(intStatus);

	dmaStruct.src = (void *)&eP;	
	dmaStruct.dest = (void *)dP->param;	
	dmaStruct.size = sizeof (g_endParam_t);
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
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	register int fastsize, i, j;
	SifRpcReceiveData_t	rD;
	u8 eccbuf[16];
		
#ifdef DEBUG		
	DPRINTF("mcserv: _McWritePage port%d slot%d page %d\n", dP->port, dP->slot, dP->fd);
#endif	

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
	g_nameParam_t *nP = (g_nameParam_t *)rpc_buf;
	SifRpcReceiveData_t	rD;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McSetFileInfo port%d slot%d file %s flags %d\n", nP->port, nP->slot, nP->name, nP->flags);
#endif
	
	sceSifGetOtherData(&rD, (void *)nP->mcT, &mcserv_buf, sizeof (mcTable_t), 0);
	
	return McSetFileInfo(nP->port, nP->slot, nP->name, (mcTable_t *)mcserv_buf, nP->flags);
}

//-------------------------------------------------------------- 
int _McCheckBlock(void *rpc_buf)
{
	g_descParam_t *dP = (g_descParam_t *)rpc_buf;
	
#ifdef DEBUG	
	DPRINTF("mcserv: _McCheckBlock port%d slot%d block %d\n", dP->port, dP->slot, dP->offset);
#endif
	
	return McCheckBlock(dP->port, dP->slot, dP->offset);
}

//-------------------------------------------------------------- 
