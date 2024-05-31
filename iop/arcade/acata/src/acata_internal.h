/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACATA_INTERNAL_H
#define _ACATA_INTERNAL_H

#include <acata.h>
#include <irx_imports.h>

struct ata_softc
{
	acQueueHeadData requestq;
	acUint32 active;
	acInt32 thid;
	struct ac_ata_h *atah;
	acUint16 cprio;
	acUint16 prio;
	acTimerData timer;
};

struct ata_dma
{
	acDmaData ad_dma;
	acAtaT ad_ata;
	acInt32 ad_thid;
	acInt32 ad_result;
	acUint32 ad_state;
};

struct atapi_dma
{
	acDmaData ad_dma;
	acAtapiT ad_atapi;
	acInt32 ad_thid;
	acInt32 ad_result;
	acUint32 ad_state;
};

struct atapi_sense
{
	// cppcheck-suppress unusedStructMember
	acUint8 s_valid;
	// cppcheck-suppress unusedStructMember
	acUint8 s_segnum;
	acUint8 s_key;
	// cppcheck-suppress unusedStructMember
	acUint8 s_info[4];
	// cppcheck-suppress unusedStructMember
	acUint8 s_aslen;
	// cppcheck-suppress unusedStructMember
	acUint8 s_csi[4];
	acUint8 s_asc;
	acUint8 s_ascq;
	// cppcheck-suppress unusedStructMember
	acUint8 s_fruc;
	// cppcheck-suppress unusedStructMember
	acUint8 s_sks[3];
};

extern int ata_request(struct ac_ata_h *atah, int (*wakeup)(int thid));
extern int ata_probe(acAtaReg atareg);
extern int acAtaModuleStart(int argc, char **argv);
extern int acAtaModuleStop();
extern int acAtaModuleRestart(int argc, char **argv);
extern int acAtaModuleStatus();

#endif
