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

// Based on the module from SDK 3.1.0.

struct poffemu_param_stru
{
	int m_cdvdman_intr_efid;
	void (*m_cdvdman_poff_cb)(void *);
	void *m_cdvdman_poffarg;
	int m_sema_id;
};

static struct poffemu_param_stru sCdPtbl;

static unsigned int _sceCdPoffEmu(struct poffemu_param_stru *arg)
{
	Kprintf("PowerOff Simulation Start\n");
	iSetEventFlag(arg->m_cdvdman_intr_efid, 4);
	iSetEventFlag(arg->m_cdvdman_intr_efid, 0x10);
	if ( arg->m_cdvdman_poff_cb )
		arg->m_cdvdman_poff_cb(arg->m_cdvdman_poffarg);
	iSignalSema(arg->m_sema_id);
	return 0;
}

int _start(int ac, char **av)
{
	int unusedval;
	iop_sema_t semaparam;
	// Unofficial: the following variable has been made local stack
	iop_sys_clock_t sCdPoff_time;

	(void)ac;
	(void)av;

	semaparam.attr = 1;
	semaparam.initial = 0;
	semaparam.max = 1;
	semaparam.option = 0;
	sCdPtbl.m_sema_id = CreateSema(&semaparam);
	sCdPtbl.m_cdvdman_intr_efid = sceCdSC(0xFFFFFFF5, &unusedval);
	sCdPtbl.m_cdvdman_poff_cb = 0;
	if ( (unsigned int)sceCdSC(0xFFFFFFF7, &unusedval) < 0x222 )
	{
		Kprintf("This cdvdman.irx doesn't support the simulation of PowerOff_Callback of IOP\n");
	}
	else
	{
		sCdPtbl.m_cdvdman_poffarg = (void *)(uiptr)sceCdSC(0xFFFFFFE6, (int *)&sCdPtbl.m_cdvdman_poff_cb);
	}
	sCdPoff_time.hi = 0;
	sCdPoff_time.lo = 0x90000;
	SetAlarm(&sCdPoff_time, (unsigned int (*)(void *))_sceCdPoffEmu, &sCdPtbl);
	WaitSema(sCdPtbl.m_sema_id);
	DeleteSema(sCdPtbl.m_sema_id);
	return 1;
}
