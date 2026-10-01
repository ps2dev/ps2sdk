/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "sifcmd.h"
#include "irx_imports.h"

extern struct irx_export_table _exp_sifcmd;

#ifdef _IOP
IRX_ID("IOP_SIF_rpc_interface", 2, 8);
#endif
// Based on the module from SCE SDK 3.1.0.

typedef struct sif_cmd_data_
{
	void *m_pckt_buffer;
	void *m_sys_buffer;
	int m_sif_send_eebuf;
	SifCmdSysHandlerData_t *m_sys_cmd_handlers;
	int m_nr_sys_handlers;
	SifCmdHandlerData_t *m_usr_cmd_handlers;
	int m_nr_usr_handlers;
	unsigned int *m_sregs_ptr;
	int m_ef;
	void (*m_sif_1_callback)(void *userdata);
	void *m_sif_1_callback_userdata;
	SifCmdSysHandlerData_t m_sys_cmd_handler_handler[32];
	unsigned int m_soft_reg[32];
} __attribute__((aligned(16))) sif_cmd_data_t;

typedef struct t_SifCmdChgAddrData
{
	SifCmdHeader_t m_header;
	u32 m_newaddr;
} SifCmdChgAddrData_t;

static sif_cmd_data_t g_sif_cmd_data_table;
static u8 g_sif_pckt_buffer[0x80] __attribute__((aligned(16)));
static u8 g_sif_sys_buffer[0x40] __attribute__((aligned(16)));

static int _sceSifCmdIntrHdlr(void *userdata);

static void _set_sreg(void *data, void *harg)
{
	const SifCmdSRegData_t *pkt;
	sif_cmd_data_t *sci;

	pkt = (const SifCmdSRegData_t *)data;
	sci = (sif_cmd_data_t *)harg;
	sci->m_sregs_ptr[pkt->index] = pkt->value;
}

static void _change_addr(void *data, void *harg)
{
	const SifCmdChgAddrData_t *pkt;
	sif_cmd_data_t *sci;

	pkt = (const SifCmdChgAddrData_t *)data;
	sci = (sif_cmd_data_t *)harg;
	sci->m_sif_send_eebuf = pkt->m_newaddr;
}

unsigned int sceSifGetSreg(int sreg)
{
	// Unofficial: use m_sregs_ptr
	return g_sif_cmd_data_table.m_sregs_ptr[sreg];
}

void sceSifSetSreg(int sreg, unsigned int value)
{
	// Unofficial: use m_sregs_ptr
	g_sif_cmd_data_table.m_sregs_ptr[sreg] = value;
}

#ifdef DEAD_CODE
sif_cmd_data_t *sceSifGetDataTable(void)
{
	return (sif_cmd_data_t *)&g_sif_cmd_data_table;
}
#endif

static void sif_sys_cmd_handler_init_from_ee(void *data, void *harg)
{
	const SifCmdChgAddrData_t *pkt;
	sif_cmd_data_t *sci;

	pkt = (const SifCmdChgAddrData_t *)data;
	sci = (sif_cmd_data_t *)harg;
	iSetEventFlag(sci->m_ef, pkt->m_header.opt ? 0x800 : 0x100);
	if ( pkt->m_header.opt )
		return;
	sceSifSetMSFlag(SIF_STAT_CMDINIT);
	sci->m_sif_send_eebuf = pkt->m_newaddr;
}

int _start(int ac, char **av)
{
	const int *BootMode3;
	unsigned int i;

	(void)ac;
	(void)av;

	BootMode3 = QueryBootMode(3);
	if ( BootMode3 && (BootMode3[1] & 1) )
	{
		printf(" No SIF service(sifcmd)\n");
		return MODULE_NO_RESIDENT_END;
	}
	if ( BootMode3 && (BootMode3[1] & 2) )
	{
		printf(" No SIFCMD/RPC service\n");
		return MODULE_NO_RESIDENT_END;
	}
	if ( !sceSifCheckInit() )
		sceSifInit();
	if ( RegisterLibraryEntries(&_exp_sifcmd) )
		return MODULE_NO_RESIDENT_END;
	g_sif_cmd_data_table.m_pckt_buffer = g_sif_pckt_buffer;
	g_sif_cmd_data_table.m_sys_buffer = g_sif_sys_buffer;
	sceSifSetSysCmdBuffer(
		g_sif_cmd_data_table.m_sys_cmd_handler_handler,
		sizeof(g_sif_cmd_data_table.m_sys_cmd_handler_handler) / sizeof(g_sif_cmd_data_table.m_sys_cmd_handler_handler[0]));
	g_sif_cmd_data_table.m_sif_send_eebuf = 0;
	sceSifSetCmdBuffer(NULL, 0);
	g_sif_cmd_data_table.m_sregs_ptr = g_sif_cmd_data_table.m_soft_reg;
	sceSifClearSif1CB();
	for ( i = 0; i < (unsigned int)g_sif_cmd_data_table.m_nr_sys_handlers; i += 1 )
		sceSifRemoveCmdHandler(0x80000000 + i);
	for ( i = 0; i < sizeof(g_sif_cmd_data_table.m_soft_reg) / sizeof(g_sif_cmd_data_table.m_soft_reg[0]); i += 1 )
		sceSifSetSreg(i, 0);
	sceSifAddCmdHandler(SIF_CMD_CHANGE_SADDR, _change_addr, (void *)&g_sif_cmd_data_table);
	sceSifAddCmdHandler(SIF_CMD_SET_SREG, _set_sreg, (void *)&g_sif_cmd_data_table);
	sceSifAddCmdHandler(SIF_CMD_INIT_CMD, sif_sys_cmd_handler_init_from_ee, (void *)&g_sif_cmd_data_table);
	g_sif_cmd_data_table.m_ef = GetSystemStatusFlag();
	RegisterIntrHandler(IOP_IRQ_DMA_SIF1, 1, _sceSifCmdIntrHdlr, &g_sif_cmd_data_table);
	EnableIntr(0x200 | IOP_IRQ_DMA_SIF1);
	sceSifSetSubAddr((u32)g_sif_cmd_data_table.m_pckt_buffer);
	return MODULE_RESIDENT_END;
}

int sifcmd_deinit(void)
{
	sceSifExitCmd();
#if 0
	// FIXME: Do we really need this call?
	sifman_2();
#endif
	return 0;
}

void sceSifInitCmd(void)
{
	sceSifSetSMFlag(SIF_STAT_CMDINIT);
	WaitEventFlag(g_sif_cmd_data_table.m_ef, 0x100, WEF_AND, NULL);
}

void sceSifExitCmd(void)
{
	int old_irq;

	DisableIntr(IOP_IRQ_DMA_SIF1, &old_irq);
	ReleaseIntrHandler(IOP_IRQ_DMA_SIF1);
}

void sceSifSetCmdBuffer(SifCmdHandlerData_t *db, int size)
{
	g_sif_cmd_data_table.m_usr_cmd_handlers = db;
	g_sif_cmd_data_table.m_nr_usr_handlers = size;
}

void sceSifSetSysCmdBuffer(SifCmdSysHandlerData_t *db, int size)
{
	g_sif_cmd_data_table.m_sys_cmd_handlers = db;
	g_sif_cmd_data_table.m_nr_sys_handlers = size;
}

void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg)
{
	*((!(cid & 0x80000000)) ? &(g_sif_cmd_data_table.m_usr_cmd_handlers[cid].handler) :
														&(g_sif_cmd_data_table.m_sys_cmd_handlers[(cid & 0x7FFFFFFF)].handler)) = handler;
	*((!(cid & 0x80000000)) ? &(g_sif_cmd_data_table.m_usr_cmd_handlers[cid].harg) :
														&(g_sif_cmd_data_table.m_sys_cmd_handlers[(cid & 0x7FFFFFFF)].harg)) = harg;
}

void sceSifRemoveCmdHandler(int cid)
{
	// Unofficial: also set harg to NULL
	sceSifAddCmdHandler(cid, NULL, NULL);
}

static int _sceSifSendCmd(
	int cid,
	char flags,
	SifCmdHeader_t *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *userdata),
	void *completion_cb_userdata)
{
	int dmatc;
	unsigned int trid;
	SifDmaTransfer_t dmat[2];
	int state;

	if ( (unsigned int)(packet_size - 16) >= 0x61 )
		return 0;
	dmatc = 0;
	packet->dest = (size_extra > 0) ? dest_extra : NULL;
	packet->dsize = (size_extra > 0) ? size_extra : 0;
	dmat[dmatc].dest = dest_extra;
	dmat[dmatc].size = size_extra;
	dmat[dmatc].attr = 0;
	dmat[dmatc].src = src_extra;
	dmatc += !!(size_extra > 0);
	packet->psize = packet_size;
	packet->cid = cid;
	dmat[dmatc].src = packet;
	dmat[dmatc].attr = SIF_DMA_INT_O;
	dmat[dmatc].size = packet_size;
	dmat[dmatc].dest = (void *)(g_sif_cmd_data_table.m_sif_send_eebuf);
	if ( (flags & 1) )
		return (flags & 8) ? sceSifSetDmaIntr(dmat, dmatc + 1, completion_cb, completion_cb_userdata) :
												 (unsigned int)sceSifSetDma(dmat, dmatc + 1);
	CpuSuspendIntr(&state);
	trid = (flags & 8) ? sceSifSetDmaIntr(dmat, dmatc + 1, completion_cb, completion_cb_userdata) :
											 (unsigned int)sceSifSetDma(dmat, dmatc + 1);
	CpuResumeIntr(state);
	return trid;
}

unsigned int sceSifSendCmd(int cid, void *packet, int packet_size, void *src_extra, void *dest_extra, int size_extra)
{
	return _sceSifSendCmd(cid, 0, (SifCmdHeader_t *)packet, packet_size, src_extra, dest_extra, size_extra, NULL, NULL);
}

unsigned int sceSifSendCmdIntr(
	int cid,
	void *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *userdata),
	void *completion_cb_userdata)
{
	return _sceSifSendCmd(
		cid,
		8,
		(SifCmdHeader_t *)packet,
		packet_size,
		src_extra,
		dest_extra,
		size_extra,
		completion_cb,
		completion_cb_userdata);
}

unsigned int isceSifSendCmd(int cid, void *packet, int packet_size, void *src_extra, void *dest_extra, int size_extra)
{
	return _sceSifSendCmd(cid, 1, (SifCmdHeader_t *)packet, packet_size, src_extra, dest_extra, size_extra, NULL, NULL);
}

unsigned int isceSifSendCmdIntr(
	int cid,
	void *packet,
	int packet_size,
	void *src_extra,
	void *dest_extra,
	int size_extra,
	void (*completion_cb)(void *userdata),
	void *completion_cb_userdata)
{
	return _sceSifSendCmd(
		cid,
		9,
		(SifCmdHeader_t *)packet,
		packet_size,
		src_extra,
		dest_extra,
		size_extra,
		completion_cb,
		completion_cb_userdata);
}

static void sif_sif1_handler_noop(void *userdata)
{
	(void)userdata;
}

void sceSifSetSif1CB(void (*func)(void *userdata), void *userdata)
{
	// Unofficial: use no-op function if function is NULL
	g_sif_cmd_data_table.m_sif_1_callback = func ? func : &sif_sif1_handler_noop;
	g_sif_cmd_data_table.m_sif_1_callback_userdata = userdata;
}

void sceSifClearSif1CB(void)
{
	sceSifSetSif1CB(NULL, NULL);
}

static int _sceSifCmdIntrHdlr(void *userdata)
{
	SifCmdHeader_t *pkt;
	int size;
	int i;
	u32 tmpbuf[32];
	sif_cmd_data_t *sci;

	sci = (sif_cmd_data_t *)userdata;
	// Unofficial: unconditionally call callback
	sci->m_sif_1_callback(sci->m_sif_1_callback_userdata);
	pkt = (SifCmdHeader_t *)sci->m_pckt_buffer;
	size = pkt->psize;
	if ( !size )
	{
		sceSifSetDChain();
		return 1;
	}
	pkt->psize = 0;
	size += 3;
	size += (size < 0) ? 3 : 0;
	tmpbuf[2] = 0;  // cid
	for ( i = 0; i < (size / (int)sizeof(tmpbuf[0])); i += 1 )
		tmpbuf[i] = ((u32 *)pkt)[i];
	pkt = (SifCmdHeader_t *)tmpbuf;
	sceSifSetDChain();
	if ( ((int)(pkt->cid & 0x7FFFFFFF) < ((!(pkt->cid & 0x80000000)) ? sci->m_nr_usr_handlers : sci->m_nr_sys_handlers)) && ((!(pkt->cid & 0x80000000)) ? g_sif_cmd_data_table.m_usr_cmd_handlers[pkt->cid].handler : g_sif_cmd_data_table.m_sys_cmd_handlers[(pkt->cid & 0x7FFFFFFF)].handler) )
		((!(pkt->cid & 0x80000000)) ? g_sif_cmd_data_table.m_usr_cmd_handlers[pkt->cid].handler :
																	g_sif_cmd_data_table.m_sys_cmd_handlers[(pkt->cid & 0x7FFFFFFF)].handler)(
			pkt,
			(!(pkt->cid & 0x80000000)) ? g_sif_cmd_data_table.m_usr_cmd_handlers[pkt->cid].harg :
																	 g_sif_cmd_data_table.m_sys_cmd_handlers[(pkt->cid & 0x7FFFFFFF)].harg);
	return 1;
}
