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
# IOP reset and status routines.
*/

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <string.h>

#include <iopcontrol.h>

#define RESET_ARG_MAX	79

#ifdef F_SifIopReset
struct _iop_reset_pkt {
	struct t_SifCmdHeader header;
	int	arglen;
	int	mode;
	char	arg[RESET_ARG_MAX + 1];
} ALIGNED(16);

int SifIopReset(const char *arg, int mode)
{
	struct _iop_reset_pkt reset_pkt;  /* Implicitly aligned. */
	struct t_SifDmaTransfer dmat;

	SifExitRpc(); // exit RPC in case it's already been init'd

	SifStopDma();

	memset(&reset_pkt, 0, sizeof reset_pkt);

	reset_pkt.header.size = sizeof reset_pkt;
	reset_pkt.header.cid  = 0x80000003;

	reset_pkt.mode = mode;
	if (arg != NULL) {
		strncpy(reset_pkt.arg, arg, RESET_ARG_MAX);
		reset_pkt.arg[RESET_ARG_MAX] = '\0';

		reset_pkt.arglen = strlen(reset_pkt.arg) + 1;
	}

	dmat.src  = &reset_pkt;
	dmat.dest = (void *)SifGetReg(SIF_REG_SUBADDR);
	dmat.size = sizeof reset_pkt;
	dmat.attr = 0x40 | SIF_DMA_INT_O;
	SifWriteBackDCache(&reset_pkt, sizeof reset_pkt);

	if (!SifSetDma(&dmat, 1))
		return 0;

	SifSetReg(SIF_REG_SMFLAG, 0x10000);
	SifSetReg(SIF_REG_SMFLAG, 0x20000);
	SifSetReg(0x80000002, 0);
	SifSetReg(0x80000000, 0);

	return 1;
}
#endif

#ifdef F_SifResetIop
int SifResetIop()
{
	int ret;

	SifExitRpc();
	ret = SifIopReset(NULL, 0);
	if(ret != 0)
		while(!SifIopSync());

	return ret;
}
#endif

#ifdef F_SifIopIsAlive
int SifIopIsAlive()
{
	return ((SifGetReg(SIF_REG_SMFLAG) & 0x10000) != 0);
}
#endif

#ifdef F_SifIopSync
int SifIopSync()
{
	if (SifGetReg(SIF_REG_SMFLAG) & 0x40000) {
		SifSetReg(SIF_REG_SMFLAG, 0x40000);
		return 1;
	}
	return 0;
}
#endif
