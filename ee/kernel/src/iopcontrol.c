/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP reset and status routines.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <string.h>
#include <stdio.h>

#include <iopcontrol.h>

#ifdef F___iop_control_internals
int _iop_reboot_count = 0;
#endif

extern int _iop_reboot_count;

#ifdef F_SifIopReset

int SifIopReset(const char *arg, int mode)
{
	static SifCmdResetData_t reset_pkt __attribute__((aligned(64)));
	struct t_SifDmaTransfer dmat;
	int arglen;

	_iop_reboot_count++; // increment reboot counter to allow RPC clients to detect unbinding!

	SifStopDma();	//Stop DMA transfers across SIF0 (IOP -> EE).

	/*	The original did not null-terminate, had no bounds-checking and counted the characters as it copied.
		The IOP side will only copy up to arglen characters. */
	if(arg != NULL)
	{
		for(arglen = 0; arg[arglen] != '\0'; arglen++)
			reset_pkt.arg[arglen] = arg[arglen];
	}
	else
	{	//While NULL was not officially supported, do this for backward-compatibility with old homebrew projects.
		arglen = 0;
	}

	reset_pkt.header.psize = sizeof reset_pkt;	//dsize is not initialized (and not processed, even on the IOP).
	reset_pkt.header.cid  = SIF_CMD_RESET_CMD;
	reset_pkt.arglen = arglen;
	reset_pkt.mode = mode;

	dmat.src  = &reset_pkt;
	dmat.dest = (void *)SifGetReg(SIF_SYSREG_SUBADDR);
	dmat.size = sizeof(reset_pkt);
	dmat.attr = SIF_DMA_ERT | SIF_DMA_INT_O;
	SifWriteBackDCache(&reset_pkt, sizeof(reset_pkt));

	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_BOOTEND);

	if (!SifSetDma(&dmat, 1))
		return 0;

	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_SIFINIT);
	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_CMDINIT);
	SifSetReg(SIF_SYSREG_RPCINIT, 0);
	SifSetReg(SIF_SYSREG_SUBADDR, (int)NULL);

	return 1;
}
#endif

#ifdef F_SifIopReboot
int SifIopReboot(const char* arg)
{
	char param_str[RESET_ARG_MAX+1];

	if(strlen(arg) + 11 > RESET_ARG_MAX)
	{
		printf("too long parameter \'%s\'\n", arg);
		return 0;
	}

	SifInitRpc(0);
	SifExitRpc();

	strcpy(param_str, "rom0:UDNL ");
	strcat(param_str, arg);

	return SifIopReset(param_str, 0);
}
#endif

#ifdef F_SifIopIsAlive
int SifIopIsAlive(void)
{
	return ((SifGetReg(SIF_REG_SMFLAG) & SIF_STAT_SIFINIT) != 0);
}
#endif

#ifdef F_SifIopSync
int SifIopSync()
{
	return((SifGetReg(SIF_REG_SMFLAG) & SIF_STAT_BOOTEND) != 0);
}
#endif
