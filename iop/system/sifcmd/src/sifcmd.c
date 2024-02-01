/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "irx_imports.h"
#include "sifcmd.h"

extern struct irx_export_table _exp_sifcmd;

#ifdef _IOP
IRX_ID("IOP_SIF_rpc_interface", 2, 8);
#endif
// Based on the module from SCE SDK 3.1.0.

typedef struct sif_cmd_data_
{
	void *pktbuf;
	void *unused;
	int sif_send_eebuf;
	SifCmdSysHandlerData_t *sys_cmd_handlers;
	int nr_sys_handlers;
	SifCmdHandlerData_t *usr_cmd_handlers;
	int nr_usr_handlers;
	unsigned int *sregs_ptr;
	int ef;
	void (*sif_1_callback)(void *userdata);
	void *sif_1_callback_userdata;
	SifCmdSysHandlerData_t sys_cmd_handler_handler[32];
	unsigned int sregs[32];
} sif_cmd_data_t;

typedef struct t_SifCmdChgAddrData
{
	SifCmdHeader_t header;
	u32 newaddr;
} SifCmdChgAddrData_t;

static sif_cmd_data_t sif_cmd_data;
static u8 sif_iop_recvbuf[0x80];
static u8 sif_unused[0x40];

static int sif_cmd_int_handler(sif_cmd_data_t *sci);

static void sif_sys_cmd_handler_set_sreg(const SifCmdSRegData_t *pkt, sif_cmd_data_t *sci)
{
	sci->sregs_ptr[pkt->index] = pkt->value;
}

static void sif_sys_cmd_handler_change_addr(const SifCmdChgAddrData_t *pkt, sif_cmd_data_t *sci)
{
	sci->sif_send_eebuf = pkt->newaddr;
}

unsigned int sceSifGetSreg(int index)
{
	return sif_cmd_data.sregs[index];
}

void sceSifSetSreg(int index, unsigned int value)
{
	sif_cmd_data.sregs[index] = value;
}

#if 0
sif_cmd_data_t *sif_cmd_get_internal_data()
{
	return &sif_cmd_data;
}
#endif

static void sif_sys_cmd_handler_init_from_ee(const SifCmdChgAddrData_t *pkt, sif_cmd_data_t *sci)
{
	if ( pkt->header.opt )
	{
		iSetEventFlag(sci->ef, 0x800u);
	}
	else
	{
		iSetEventFlag(sci->ef, 0x100u);
		sceSifSetMSFlag(SIF_STAT_CMDINIT);
		sci->sif_send_eebuf = pkt->newaddr;
	}
}

int _start(int ac, char **av)
{
	const int *BootMode3;

	(void)ac;
	(void)av;

	BootMode3 = QueryBootMode(3);
	if ( BootMode3 )
	{
		int BootMode3_1;

		BootMode3_1 = BootMode3[1];
		if ( (BootMode3_1 & 1) != 0 )
		{
			printf(" No SIF service(sifcmd)\n");
			return 1;
		}
		if ( (BootMode3_1 & 2) != 0 )
		{
			printf(" No SIFCMD/RPC service\n");
			return 1;
		}
	}
	if ( !sceSifCheckInit() )
		sceSifInit();
	if ( RegisterLibraryEntries(&_exp_sifcmd) == 0 )
	{
		unsigned int i;

		sif_cmd_data.pktbuf = sif_iop_recvbuf;
		sif_cmd_data.unused = sif_unused;
		sif_cmd_data.sys_cmd_handlers = sif_cmd_data.sys_cmd_handler_handler;
		sif_cmd_data.nr_sys_handlers =
			sizeof(sif_cmd_data.sys_cmd_handler_handler) / sizeof(sif_cmd_data.sys_cmd_handler_handler[0]);
		sif_cmd_data.sif_send_eebuf = 0;
		sif_cmd_data.usr_cmd_handlers = 0;
		sif_cmd_data.nr_usr_handlers = 0;
		sif_cmd_data.sregs_ptr = sif_cmd_data.sregs;
		sif_cmd_data.sif_1_callback = 0;
		sif_cmd_data.sif_1_callback_userdata = 0;
		for ( i = 0; i < sizeof(sif_cmd_data.sys_cmd_handler_handler) / sizeof(sif_cmd_data.sys_cmd_handler_handler[0]);
					i += 1 )
		{
			sif_cmd_data.sys_cmd_handler_handler[i].handler = NULL;
			sif_cmd_data.sys_cmd_handler_handler[i].harg = NULL;
		}
		for ( i = 0; i < sizeof(sif_cmd_data.sregs) / sizeof(sif_cmd_data.sregs[0]); i += 1 )
		{
			sif_cmd_data.sregs[i] = 0;
		}
		sif_cmd_data.sys_cmd_handler_handler[0].handler = (SifCmdHandler_t)sif_sys_cmd_handler_change_addr;
		sif_cmd_data.sys_cmd_handler_handler[0].harg = &sif_cmd_data;
		sif_cmd_data.sys_cmd_handler_handler[1].handler = (SifCmdHandler_t)sif_sys_cmd_handler_set_sreg;
		sif_cmd_data.sys_cmd_handler_handler[1].harg = &sif_cmd_data;
		sif_cmd_data.ef = GetSystemStatusFlag();
		sif_cmd_data.sys_cmd_handler_handler[2].handler = (SifCmdHandler_t)sif_sys_cmd_handler_init_from_ee;
		sif_cmd_data.sys_cmd_handler_handler[2].harg = &sif_cmd_data;
		RegisterIntrHandler(IOP_IRQ_DMA_SIF1, 1, (int (*)(void *))sif_cmd_int_handler, &sif_cmd_data);
		EnableIntr(0x22B);
		sceSifSetSubAddr((u32)sif_iop_recvbuf);
		return 0;
	}
	return 1;
}

int sifcmd_deinit(void)
{
	int old_irq;

	DisableIntr(IOP_IRQ_DMA_SIF1, &old_irq);
	ReleaseIntrHandler(IOP_IRQ_DMA_SIF1);
#if 0
  // FIXME: Do we really need this call?
  sifman_2();
#endif
	return 0;
}

void sceSifInitCmd(void)
{
	sceSifSetSMFlag(SIF_STAT_CMDINIT);
	WaitEventFlag(sif_cmd_data.ef, 0x100u, 0, 0);
}

void sceSifExitCmd(void)
{
	int old_irq;

	DisableIntr(IOP_IRQ_DMA_SIF1, &old_irq);
	ReleaseIntrHandler(IOP_IRQ_DMA_SIF1);
}

void sceSifSetCmdBuffer(SifCmdHandlerData_t *cmdBuffer, int size)
{
	sif_cmd_data.usr_cmd_handlers = cmdBuffer;
	sif_cmd_data.nr_usr_handlers = size;
}

void sceSifSetSysCmdBuffer(SifCmdSysHandlerData_t *sysCmdBuffer, int size)
{
	sif_cmd_data.sys_cmd_handlers = sysCmdBuffer;
	sif_cmd_data.nr_sys_handlers = size;
}

void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg)
{
	if ( cid >= 0 )
	{
		sif_cmd_data.usr_cmd_handlers[cid].handler = handler;
		sif_cmd_data.usr_cmd_handlers[cid].harg = harg;
	}
	else
	{
		sif_cmd_data.sys_cmd_handlers[cid + (cid & 0x7FFFFFFF)].handler = handler;
		sif_cmd_data.sys_cmd_handlers[cid + (cid & 0x7FFFFFFF)].harg = harg;
	}
}

void sceSifRemoveCmdHandler(int cid)
{
	if ( cid >= 0 )
		sif_cmd_data.usr_cmd_handlers[cid].handler = NULL;
	else
		sif_cmd_data.sys_cmd_handlers[cid + (cid & 0x7FFFFFFF)].handler = NULL;
}

static int sif_send_cmd_common(
	int cmd,
	char flags,
	SifCmdHeader_t *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *),
	void *completion_cb_userdata)
{
	int dmatc1;
	SifDmaTransfer_t *dmatp;
	int dmatc2;
	int sif_send_eebuf;
	unsigned int dmar1;
	SifDmaTransfer_t dmat[2];
	int state;

	if ( (unsigned int)(packet_size - 16) >= 0x61 )
		return 0;
	dmatc1 = 0;
	if ( size_extra <= 0 )
	{
		int tmp1;

		tmp1 = *(u8 *)packet;
		packet->dest = 0;
		*(u32 *)packet = tmp1;
	}
	else
	{
		int tmp2;

		dmatc1 = 1;
		dmat[0].dest = dest_extra;
		dmat[0].size = size_extra;
		dmat[0].attr = 0;
		dmat[0].src = src_extra;
		tmp2 = *(u8 *)packet;
		packet->dest = dest_extra;
		*(u32 *)packet = tmp2 | (size_extra << 8);
	}
	dmatp = &dmat[dmatc1];
	dmatc2 = dmatc1 + 1;
	*(u8 *)packet = packet_size;
	packet->cid = cmd;
	dmatp->src = packet;
	sif_send_eebuf = sif_cmd_data.sif_send_eebuf;
	dmatp->attr = 4;
	dmatp->size = packet_size;
	dmatp->dest = (void *)sif_send_eebuf;
	if ( (flags & 1) != 0 )
	{
		if ( (flags & 8) != 0 )
			return sceSifSetDmaIntr(dmat, dmatc2, completion_cb, completion_cb_userdata);
		else
			return sceSifSetDma(dmat, dmatc2);
	}
	else
	{
		unsigned int dmar2;

		CpuSuspendIntr(&state);
		if ( (flags & 8) != 0 )
			dmar2 = sceSifSetDmaIntr(dmat, dmatc2, completion_cb, completion_cb_userdata);
		else
			dmar2 = sceSifSetDma(dmat, dmatc2);
		dmar1 = dmar2;
		CpuResumeIntr(state);
	}
	return dmar1;
}

unsigned int sceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra, void *dest_extra, int size_extra)
{
	return sif_send_cmd_common(cmd, 0, (SifCmdHeader_t *)packet, packet_size, src_extra, dest_extra, size_extra, 0, 0);
}

unsigned int sceSifSendCmdIntr(
	int cmd,
	void *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *),
	void *completion_cb_userdata)
{
	return sif_send_cmd_common(
		cmd,
		8,
		(SifCmdHeader_t *)packet,
		packet_size,
		src_extra,
		dest_extra,
		size_extra,
		(void (*)(void *))completion_cb,
		completion_cb_userdata);
}

unsigned int isceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra, void *dest_extra, int size_extra)
{
	return sif_send_cmd_common(cmd, 1, (SifCmdHeader_t *)packet, packet_size, src_extra, dest_extra, size_extra, 0, 0);
}

unsigned int isceSifSendCmdIntr(
	int cmd,
	void *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *),
	void *completion_cb_userdata)
{
	return sif_send_cmd_common(
		cmd,
		9,
		(SifCmdHeader_t *)packet,
		packet_size,
		src_extra,
		dest_extra,
		size_extra,
		(void (*)(void *))completion_cb,
		completion_cb_userdata);
}

void sceSifSetSif1CB(void (*func)(void *userdata), void *userdata)
{
	sif_cmd_data.sif_1_callback = func;
	sif_cmd_data.sif_1_callback_userdata = userdata;
}

void sceSifClearSif1CB(void)
{
	sif_cmd_data.sif_1_callback = NULL;
	sif_cmd_data.sif_1_callback_userdata = NULL;
}

static int sif_cmd_int_handler(sif_cmd_data_t *sci)
{
	void (*sif_1_callback)(void *);
	SifCmdHeader_t *pktbuf1;
	int size;
	int size_calc1;
	SifCmdHeader_t *pktbuf2;
	int pktwords;
	int i;
	u32 tmp1;
	u32 tmpbuf1[32];

	sif_1_callback = sci->sif_1_callback;
	if ( sif_1_callback )
		sif_1_callback(sci->sif_1_callback_userdata);
	pktbuf1 = (SifCmdHeader_t *)sci->pktbuf;
	size = *(u8 *)sci->pktbuf;
	if ( !*(u8 *)sci->pktbuf )
	{
		sceSifSetDChain();
		return 1;
	}
	*(u8 *)pktbuf1 = 0;
	size_calc1 = size + 3;
	pktbuf2 = pktbuf1;
	if ( size + 3 < 0 )
		size_calc1 = size + 6;
	pktwords = size_calc1 >> 2;
	i = 0;
	tmpbuf1[2] = 0;
	if ( size_calc1 >> 2 > 0 )
	{
		u32 *tmpptr1;

		tmpptr1 = tmpbuf1;
		do
		{
			tmp1 = *(u32 *)pktbuf2;
			pktbuf2 = (SifCmdHeader_t *)((char *)pktbuf2 + 4);
			++i;
			*tmpptr1++ = tmp1;
		} while ( i < pktwords );
	}
	sceSifSetDChain();
	if ( (tmpbuf1[2] & 0x80000000) == 0 )
	{
		if ( (int)tmpbuf1[2] < sci->nr_usr_handlers )
		{
			if ( sif_cmd_data.usr_cmd_handlers[tmpbuf1[2]].handler != NULL )
			{
				sif_cmd_data.usr_cmd_handlers[tmpbuf1[2]].handler(tmpbuf1, sif_cmd_data.usr_cmd_handlers[tmpbuf1[2]].harg);
			}
		}
	}
	else if ( (signed int)(tmpbuf1[2] & 0x7FFFFFFF) < sci->nr_sys_handlers )
	{
		if ( sif_cmd_data.sys_cmd_handlers[tmpbuf1[2] + (tmpbuf1[2] & 0x7FFFFFFF)].handler != NULL )
		{
			sif_cmd_data.sys_cmd_handlers[tmpbuf1[2] + (tmpbuf1[2] & 0x7FFFFFFF)].handler(
				tmpbuf1, sif_cmd_data.usr_cmd_handlers[tmpbuf1[2] + (tmpbuf1[2] & 0x7FFFFFFF)].harg);
		}
	}
	return 1;
}
