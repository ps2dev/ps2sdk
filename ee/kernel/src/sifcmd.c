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
# EE SIF commands
# MRB: This file now contains the SIF routines included
# with libpsware.  Bug reports welcome.
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifcmd.h>

#define CMD_PACKET_MAX		128
#define CMD_PACKET_DATA_MAX 	112
#define CMD_HANDLER_MAX		32

/* EE DMAC registers.  */
#define DMAC_COMM_STAT	0x1000e010
#define DMAC_SIF0_CHCR	0x1000c000
#define   CHCR_STR		0x100
#define   STAT_SIF0		0x20

/* Even though I'm reluctant to do this, I've made this structure binary 
   compatible with the SCE libs and ps2lib.  In all implementations, a pointer
   to this data is stored in SIF register 0x80000001.  Each routine that
   relies on this data can then use the data referenced from that register, so
   that even if a SIF library is initialized after this one, we should still
   work exactly as expected.  */
struct cmd_data {
	void	*pktbuf;	/* Command packet received from the IOP */
	void	*unused;	/* Called "cmdbuf", but unused. */
	void	*iopbuf;	/* Address of IOP SIF DMA receive address */
	SifCmdHandlerData_t *sys_cmd_handlers;
	u32	nr_sys_handlers;
	SifCmdHandlerData_t *usr_cmd_handlers;
	u32	nr_usr_handlers;
	int	*sregs;
};

extern struct cmd_data _sif_cmd_data;
int _SifCmdIntHandler();

#ifdef F_sif_cmd_send
static u32 _SifSendCmd(int cid, int mode, void *pkt, u32 pktsize, void *src,
		void *dest, int size)
{
	SifDmaTransfer_t dmat[2];
	SifCmdHeader_t *header;
	int count = 0;

	pktsize &= 0xff;

	if (pktsize > CMD_PACKET_DATA_MAX)
		return 0;

	header = (SifCmdHeader_t *)pkt;
	header->cid  = cid;
	header->size = pktsize;
	header->dest = NULL;

	if (size > 0) {
		header->size = pktsize | (size << 8);
		header->dest = dest;

		if (mode & 4)	/* if mode is & 4, flush reference cache */
			SifWriteBackDCache(src, size);

		dmat[count].src  = src;
		dmat[count].dest = dest;
		dmat[count].size = size;
		dmat[count].attr = 0;
		count++;
	}

	dmat[count].src  = pkt;
	dmat[count].dest = _sif_cmd_data.iopbuf;
	dmat[count].size = pktsize;
	dmat[count].attr = 0x40 | SIF_DMA_INT_O;
	count++;

	SifWriteBackDCache(pkt, pktsize);

	if (mode & 1)  /* INTERRUPT DMA TRANSFER */
		return iSifSetDma(dmat, count);
	else
		return SifSetDma(dmat, count);
}

u32
SifSendCmd( int command, void *send_data, int send_len, 
   		   void *extra_from, void *extra_dest, int extra_len)
{
   return _SifSendCmd( command, 0, send_data, send_len, 
      		extra_from, extra_dest, extra_len);
}

u32 
iSifSendCmd( int command, void *send_data, int send_len, 
   		   void *extra_from, void *extra_dest, int extra_len)
{
   return _SifSendCmd( command, 1, send_data, send_len, 
      		extra_from, extra_dest, extra_len);
}
#endif

#ifdef F__sif_cmd_int_handler
int _SifCmdIntHandler()
{
	u128 packet[8];
	u128 *pktbuf;
	struct cmd_data *cmd_data = &_sif_cmd_data;
	SifCmdHeader_t *header;
	SifCmdHandlerData_t *cmd_handlers;
	int size, pktquads, id, i = 0;

	EI();

	header = (SifCmdHeader_t *)cmd_data->pktbuf;

	if (!(size = (header->size & 0xff)))
		goto out;

	/* TODO: Don't copy anything extra */
	pktquads = (size + 30) >> 4;
	header->size = 0;
	if (pktquads) {
		pktbuf = (u128 *)cmd_data->pktbuf;
		while (pktquads--) {
			packet[i] = pktbuf[i];
			i++;
		}
	}

	iSifSetDChain();

	header = (SifCmdHeader_t *)packet;
	/* Get the command handler id and determine which handler list to
	   dispatch from.  */
	id = header->cid & ~SYSTEM_CMD;

	if (id < CMD_HANDLER_MAX) {
		if (header->cid & SYSTEM_CMD) {
			cmd_handlers = cmd_data->sys_cmd_handlers;
		}
		else {
			cmd_handlers = cmd_data->usr_cmd_handlers;
		}
	} else {
		goto out;
	}

	if (cmd_handlers[id].handler)
		cmd_handlers[id].handler(packet, cmd_handlers[id].harg);

out:
	EE_SYNC();
	EI();
	return 0;
}
#endif

#ifdef F_sif_cmd_main
static u8 pktbuf[128] __attribute__((aligned(64)));
/* Define this so that in the unlikely case another SIF implementation decides
   to use it, it won't crash.  Otherwise unused.  */
static u8 cmdbuf[64] __attribute__((aligned(64)));

static SifCmdHandlerData_t sys_cmd_handlers[CMD_HANDLER_MAX];
static SifCmdHandlerData_t usr_cmd_handlers[CMD_HANDLER_MAX];
static int sregs[32];

/* I'd rather do this statically than to fill this in with code.  It's both
   smaller and faster to do it this way.  */
struct cmd_data _sif_cmd_data = {
	pktbuf:		pktbuf,
	unused:		cmdbuf,
	sys_cmd_handlers: sys_cmd_handlers,
	nr_sys_handlers: CMD_HANDLER_MAX,
	usr_cmd_handlers: usr_cmd_handlers,
	nr_usr_handlers: CMD_HANDLER_MAX,
	sregs:		sregs
};

static int init = 0;
static int sif0_id = -1;

struct ca_pkt {
	SifCmdHeader_t header;
	void	*buf;
};

static void change_addr(void *packet, void *harg)
{
	struct cmd_data *cmd_data = (struct cmd_data *)harg;
	struct ca_pkt *pkt = (struct ca_pkt *)packet;

	cmd_data->iopbuf = pkt->buf;
}

struct sr_pkt {
	SifCmdHeader_t header;
	u32	sreg;
	int	val;
};

static void set_sreg(void *packet, void *harg)
{
	struct cmd_data *cmd_data = (struct cmd_data *)harg;
	struct sr_pkt *pkt = (struct sr_pkt *)packet;

	cmd_data->sregs[pkt->sreg] = pkt->val;
}

void SifInitCmd()
{
	u32 packet[5];	/* Implicitly aligned to 16 bytes */
	int i;

	if (init)
		return;

	DI();

	_sif_cmd_data.pktbuf = UNCACHED_SEG(_sif_cmd_data.pktbuf);
	_sif_cmd_data.unused = UNCACHED_SEG(_sif_cmd_data.unused);

	for (i = 0; i < CMD_HANDLER_MAX; i++) {
		_sif_cmd_data.sys_cmd_handlers[i].handler = NULL;
		_sif_cmd_data.sys_cmd_handlers[i].harg = NULL;
	}

	for (i = 0; i < 32; i++)
		_sif_cmd_data.sregs[i] = 0;

	_sif_cmd_data.sys_cmd_handlers[0].handler = change_addr;
	_sif_cmd_data.sys_cmd_handlers[0].harg    = &_sif_cmd_data;
	_sif_cmd_data.sys_cmd_handlers[1].handler = set_sreg;
	_sif_cmd_data.sys_cmd_handlers[1].harg    = &_sif_cmd_data;

	EI();
	FlushCache(0);

	if (_lw(DMAC_COMM_STAT) & STAT_SIF0)
		_sw(STAT_SIF0, DMAC_COMM_STAT);

	if (!(_lw(DMAC_SIF0_CHCR) & CHCR_STR))
		SifSetDChain();

	sif0_id = AddDmacHandler(5, _SifCmdIntHandler, 0);
	EnableDmac(5);
	
	init = 1;

	_sif_cmd_data.iopbuf = (void *)SifGetReg(0x80000000);
	if (_sif_cmd_data.iopbuf) {
		/* IOP SIF CMD is already initialized, so give it our new
		   receive address.  */
		((struct ca_pkt *)(packet))->buf = _sif_cmd_data.pktbuf;
		SifSendCmd(0x80000000, packet, sizeof packet, NULL, NULL, 0);
	} else {
		/* Sync */
		while (!(SifGetReg(SIF_REG_SMFLAG) & 0x20000)) ;

		_sif_cmd_data.iopbuf = (void *)SifGetReg(SIF_REG_SUBADDR);
		SifSetReg(0x80000000, (u32)_sif_cmd_data.iopbuf);
		/* See the note above about struct cmd_data, and the use of
		   this register.  */
		SifSetReg(0x80000001, (u32)&_sif_cmd_data);
		packet[3] = 0;
		packet[4] = (u32)_sif_cmd_data.pktbuf;
		SifSendCmd(0x80000002, packet, sizeof packet, NULL, NULL, 0);
	}
}

void SifExitCmd()
{
    DisableDmac(5);
    RemoveDmacHandler(5, sif0_id);
    init = 0;
}
#endif

#ifdef F_sif_cmd_addhandler
void SifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg)
{
	struct cmd_data *cmd_data = &_sif_cmd_data;
	SifCmdHandlerData_t *cmd_handlers;
	u32 id = cid & ~SYSTEM_CMD;

	if (cid & SYSTEM_CMD)
		cmd_handlers = cmd_data->sys_cmd_handlers;
	else
		cmd_handlers = cmd_data->usr_cmd_handlers;

	cmd_handlers[id].handler = handler;
	cmd_handlers[id].harg    = harg;
}
#endif

#ifdef F_sif_sreg_get
int SifGetSreg(int sreg)
{
	struct cmd_data *cmd_data = &_sif_cmd_data;

	return cmd_data->sregs[sreg];
}
#endif
