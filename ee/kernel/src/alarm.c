/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE kernel update for alarm functions
 * ReleaseAlarm is unable to correctly release alarm in all CEX/DEX EE kernels.
 * This also includes code for dealing with the restriction on the COP0 EIE bit
 * becoming set to 0 when an interrupt occurs before interrupts can be disabled.
 */

#include <kernel.h>
#include <stdio.h>

struct SyscallData{
	int syscall;
	void *function;
};

static const struct SyscallData SysEntry[]={
	{0x5A, &kCopy},
	{0x5B, (void*)0x80076000},
	{0xFC, NULL},	//SetAlarm
	{0xFD, NULL},	//iSetAlarm
	{0xFE, NULL},	//ReleaseAlarm
	{0xFF, NULL},	//iReleaseAlarm
	{0x12C, NULL},	//Intc12Handler (overwrites INTC 12 handler entry)
	{0x08, NULL},	//ResumeIntrDispatch (Syscall #8)
};

extern unsigned char srcfile[];
extern unsigned int size_srcfile;

extern unsigned char eenull[];
extern unsigned int size_eenull;

void InitAlarm(void)
{
	int i;

	setup(SysEntry[0].syscall, SysEntry[0].function);
	Copy((void*)0x80076000, srcfile, size_srcfile);
	Copy((void*)0x00082000, eenull, size_eenull);
	FlushCache(0);
	FlushCache(2);
	setup(SysEntry[1].syscall, SysEntry[1].function);

	for(i=2; i<8; i++)
		setup(SysEntry[i].syscall, GetEntryAddress(SysEntry[i].syscall));
}
