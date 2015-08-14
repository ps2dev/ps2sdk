/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
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
#define SYS_CMD_HANDLER_MAX	32

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
	void	*unused;
	void	*iopbuf;	/* Address of IOP SIF DMA receive address */
	SifCmdHandlerData_t *sys_cmd_handlers;
	u32	nr_sys_handlers;
	SifCmdHandlerData_t *usr_cmd_handlers;
	u32	nr_usr_handlers;
	int	*sregs;
};

extern int _iop_reboot_count;
extern struct cmd_data _sif_cmd_data;
int _SifCmdIntHandler(int channel);

#ifdef F_sif_cmd_send
static unsigned int _SifSendCmd(int cid, int mode, void *pkt, int pktsize, void *src,
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

		if (mode & SIF_CMD_M_WBDC)	/* if mode is & 4, flush reference cache */
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
	dmat[count].attr = SIF_DMA_ERT | SIF_DMA_INT_O;
	count++;

	SifWriteBackDCache(pkt, pktsize);

	if (mode & SIF_CMD_M_INTR)  /* INTERRUPT DMA TRANSFER */
		return iSifSetDma(dmat, count);
	else
		return SifSetDma(dmat, count);
}

unsigned int
SifSendCmd( int command, void *send_data, int send_len,
   		   void *extra_from, void *extra_dest, int extra_len)
{
	return _SifSendCmd( command, 0, send_data, send_len,
      		extra_from, extra_dest, extra_len);
}

unsigned int
iSifSendCmd( int command, void *send_data, int send_len,
   		   void *extra_from, void *extra_dest, int extra_len)
{
	return _SifSendCmd( command, SIF_CMD_M_INTR, send_data, send_len,
      		extra_from, extra_dest, extra_len);
}
#endif

#ifdef F__sif_cmd_int_handler
int _SifCmdIntHandler(int channel)
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
	id = header->cid & ~SIF_CMD_ID_SYSTEM;

	if (header->cid & SIF_CMD_ID_SYSTEM) {
		if (id < cmd_data->nr_sys_handlers)
			cmd_handlers = cmd_data->sys_cmd_handlers;
		else
			goto out;
	}
	else {
		if (id < cmd_data->nr_usr_handlers)
			cmd_handlers = cmd_data->usr_cmd_handlers;
		else
			goto out;
	}

	if (cmd_handlers[id].handler)
		cmd_handlers[id].handler(packet, cmd_handlers[id].harg);

out:
	ExitHandler();
	return 0;
}
#endif

#ifdef F_sif_cmd_main
static u8 pktbuf[CMD_PACKET_MAX] __attribute__((aligned(64)));
/* Define this so that in the unlikely case another SIF implementation decides
   to use it, it won't crash.  Otherwise unused.  */
static u8 sif_unused[64] __attribute__((aligned(64)));

static SifCmdHandlerData_t sys_cmd_handlers[SYS_CMD_HANDLER_MAX];
static int sregs[32];

struct cmd_data _sif_cmd_data;
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

void SifInitCmd(void)
{
	u32 packet[5];	/* Implicitly aligned to 16 bytes */
	int i;
	static int _rb_count = 0;
	if(_rb_count != _iop_reboot_count)
	{
	    _rb_count = _iop_reboot_count;
	    if (sif0_id >= 0)
	    {
    	        DisableDmac(DMAC_SIF0);
    	        RemoveDmacHandler(DMAC_SIF0, sif0_id);
	    }
	    init = 0;
	}

	if (init)
		return;

	DI();

	_sif_cmd_data.pktbuf = UNCACHED_SEG(pktbuf);
	_sif_cmd_data.unused = UNCACHED_SEG(sif_unused);
	_sif_cmd_data.sys_cmd_handlers = sys_cmd_handlers;
	_sif_cmd_data.nr_sys_handlers = SYS_CMD_HANDLER_MAX;
	_sif_cmd_data.usr_cmd_handlers = NULL;
	_sif_cmd_data.nr_usr_handlers = 0;
	_sif_cmd_data.sregs = sregs;

	for (i = 0; i < SYS_CMD_HANDLER_MAX; i++) {
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

	//If SIF0 (IOP -> EE) is not enabled, enable it.
	if (!(_lw(DMAC_SIF0_CHCR) & CHCR_STR))
		SifSetDChain();

	sif0_id = AddDmacHandler(DMAC_SIF0, &_SifCmdIntHandler, 0);
	EnableDmac(DMAC_SIF0);

	init = 1;

	_sif_cmd_data.iopbuf = (void *)SifGetReg(SIF_SYSREG_SUBADDR);
	if (_sif_cmd_data.iopbuf) {
		/* IOP SIF CMD is already initialized, so give it our new
		   receive address.  */
		((struct ca_pkt *)(packet))->buf = _sif_cmd_data.pktbuf;
		SifSendCmd(SIF_CMD_CHANGE_SADDR, packet, sizeof packet, NULL, NULL, 0);
	} else {
		/* Sync with the IOP side's SIFCMD implementation. */
		while (!(SifGetReg(SIF_REG_SMFLAG) & SIF_STAT_CMDINIT)) ;

		_sif_cmd_data.iopbuf = (void *)SifGetReg(SIF_REG_SUBADDR);
		SifSetReg(SIF_SYSREG_SUBADDR, (u32)_sif_cmd_data.iopbuf);
		/* See the note above about struct cmd_data, and the use of
		   this register.  */
		SifSetReg(SIF_SYSREG_MAINADDR, (u32)&_sif_cmd_data);
		packet[3] = 0;
		packet[4] = (u32)_sif_cmd_data.pktbuf;
		SifSendCmd(SIF_CMD_INIT_CMD, packet, sizeof packet, NULL, NULL, 0);
	}
}

void SifExitCmd(void)
{
	DisableDmac(DMAC_SIF0);
	RemoveDmacHandler(DMAC_SIF0, sif0_id);
	init = 0;
}
#endif

#ifdef F_sif_cmd_client
SifCmdHandlerData_t *SifSetCmdBuffer(SifCmdHandlerData_t *db, int size)
{
	SifCmdHandlerData_t *old;

	old = _sif_cmd_data.usr_cmd_handlers;
	_sif_cmd_data.usr_cmd_handlers = db;
	_sif_cmd_data.nr_usr_handlers = size;

	return old;
}

void SifAddCmdHandler(int pos, SifCmdHandler_t handler, void *harg)
{
	SifCmdHandlerData_t *cmd_handlers;
	u32 id = pos & ~SIF_CMD_ID_SYSTEM;

	cmd_handlers = (pos & SIF_CMD_ID_SYSTEM) ? _sif_cmd_data.sys_cmd_handlers : _sif_cmd_data.usr_cmd_handlers;

	cmd_handlers[id].handler = handler;
	cmd_handlers[id].harg    = harg;
}
#endif

#ifdef F_sif_cmd_remove_cmdhandler
void SifRemoveCmdHandler(int pos)
{
	u32 id = pos & ~SIF_CMD_ID_SYSTEM;

	if (pos & SIF_CMD_ID_SYSTEM)
		_sif_cmd_data.sys_cmd_handlers[id].handler = NULL;
	else
		_sif_cmd_data.usr_cmd_handlers[id].handler = NULL;
}
#endif

#ifdef F_sif_sreg_get
int SifGetSreg(int sreg)
{
	return _sif_cmd_data.sregs[sreg];
}
#endif
