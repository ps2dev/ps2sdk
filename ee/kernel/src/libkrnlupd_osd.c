/**
 * @file
 * libkrnlupd - Kernel updates library.
 *
 * Contains updates for the Playstation 2's "Protokernel" EE kernel.
 *
 * The only known consoles to have a "Protokernel" are the SCPH-10000 and SCPH-15000. Both contain either boot ROM v1.00 or v1.01.
 *
 * Note that these kernels are not necessarily buggy, but were based on an older set of specifications.  
 * This file contains patches that will "modernize" these kernels until a hard reset.  
 * The code within this file was based on the code from the HDD Browser, not libosd.  
 * Originally, this was the version that I linked crt0 to,
 *  but I realized that SONY had a reason for only fully patching the protokernels within the HDD Browser:
 *  Only the HDD browser can change the new language setting, while the protokernel browser is not aware of the new language field.
 *  By fully patching the kernel, then new software will use the new language setting.
 *  However, if the user is not using the HDD Browser, then the user becomes unable to change the language setting.
 */

#include <kernel.h>
#include <syscallnr.h>
#include <osd_config.h>

#include "libkrnlupd.h"

extern unsigned char osdsrc[];
extern unsigned int size_osdsrc;

struct SyscallData{
	int syscall_num;
	void *function;
};

static struct SyscallData SyscallPatchEntries[]={
	{
		0x5A,
		&kCopy
	},
	{
		0x5B,
		(void*)0x80074000
	},
	{
		// This uses the Setup (aka "SetSyscall") syscall to patch the kernel. The address is relative to the start of the syscall table, and is in units of 32-bit pointers (4 bytes).
		0xFFFFC402,	// 0x80011F80+(-15358*4)=0x80002F88, where 0x80011F80 is the start of the syscall table.
		NULL
	},
	{
		__NR_SetOsdConfigParam,
		NULL
	},
	{
		__NR_GetOsdConfigParam,
		NULL
	},
	{
		__NR_SetOsdConfigParam2,
		NULL
	},
	{
		__NR_GetOsdConfigParam2,
		NULL
	},
};

void InitKernelFull(void){
	unsigned int i;

	if(PatchIsNeeded()){
		setup(SyscallPatchEntries[0].syscall_num, SyscallPatchEntries[0].function);
		Copy((unsigned int*)0x80074000, (unsigned int*)osdsrc, size_osdsrc);

		FlushCache(0);
		FlushCache(2);
		setup(SyscallPatchEntries[1].syscall_num, SyscallPatchEntries[1].function);

		for(i=2; i<7; i++){
			setup(SyscallPatchEntries[i].syscall_num, GetEntryAddress(SyscallPatchEntries[i].syscall_num));
		}
	}
}
