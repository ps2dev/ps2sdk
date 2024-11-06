/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "usbdpriv.h"

void *AllocSysMemoryWrap(int size)
{
	void *ptr;
	int state;

	CpuSuspendIntr(&state);
	ptr = AllocSysMemory(ALLOC_FIRST, size, NULL);
	CpuResumeIntr(state);
	return ptr;
}

int FreeSysMemoryWrap(void *ptr)
{
	int res;
	int state;

	CpuSuspendIntr(&state);
	res = FreeSysMemory(ptr);
	CpuResumeIntr(state);
	return res;
}

int usbdLock(void)
{
	return WaitSema(usbKernelResources.m_usbdSema);
}

int usbdUnlock(void)
{
	return SignalSema(usbKernelResources.m_usbdSema);
}
