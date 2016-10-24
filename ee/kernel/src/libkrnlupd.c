/*
	libkrnlupd	- Kernel updates library.

	Contains updates for the Playstation 2's "Protokernel" EE kernel.

	The only known consoles to have a "Protokernel" are the SCPH-10000 and SCPH-15000. Both contain either boot ROM v1.00 or v1.01.

	Note that these kernels are not necessarily buggy, but were based on an older set of specifications. This file contains patches that will "modernize" these kernels until a hard reset.
	(Code was based on the official Sony "libosd" patch)
*/

#include <kernel.h>
#include <syscallnr.h>
#include <osd_config.h>

#include "libkrnlupd.h"

// aka SetSyscall - Sony uses it to patch locations in the kernel.
int Copy(unsigned int *dest, const unsigned int *src, int size);
void setup(int syscall_num, void *handler);
void *GetEntryAddress(int syscall);

static int kCopy(unsigned int *dest, const unsigned int *src, int size);
static int PatchIsNeeded(void);

extern unsigned char osdsrc[];
extern unsigned int size_osdsrc;

struct SyscallData
{
    int syscall_num;
    void *function;
};

struct SyscallData SyscallPatchEntries[] = {
    {0x5A,
     &kCopy},
    {0x5B,
     (void *)0x80074000},
    {             // This uses the Setup (aka "SetSyscall") syscall to patch the kernel. The address is relative to the start of the syscall table, and is in units of 32-bit pointers (4 bytes).
     0xFFFFC402,  // 0x80011F80+(-15358*4)=0x80002F88, where 0x80011F80 is the start of the syscall table.
     NULL},
    {__NR_SetOsdConfigParam,
     NULL},
    {__NR_GetOsdConfigParam,
     NULL},
    {__NR_SetOsdConfigParam2,
     NULL},
    {__NR_GetOsdConfigParam2,
     NULL},
};

static int kCopy(unsigned int *dest, const unsigned int *src, int size)
{
    unsigned int i;

    if (size >> 2) {
        for (i = 0; i < size; i += 4, dest++, src++) {
            *dest = *src;
        }
    }

    return 0;
}

static inline int PatchIsNeeded(void)
{
    ConfigParam original_config, config;

    GetOsdConfigParam(&original_config);
    config = original_config;
    config.version = 1;  //Protokernels cannot retain values set in this field.
    SetOsdConfigParam(&config);
    GetOsdConfigParam(&config);
    SetOsdConfigParam(&original_config);

    return (config.version < 1);
}

void InitKernel(void)
{
    unsigned int i;

    if (PatchIsNeeded()) {
        setup(SyscallPatchEntries[0].syscall_num, SyscallPatchEntries[0].function);
        Copy((unsigned int *)0x80074000, (unsigned int *)osdsrc, size_osdsrc);

        FlushCache(0);
        FlushCache(2);
        setup(SyscallPatchEntries[1].syscall_num, SyscallPatchEntries[1].function);

        /*	Unlike the Sony libosd patch, patch the whole kernel fully. The Sony patch has two functions, one for taking care of ExecPS2() and the other for the OsdConfig functions.
			Obviously, the PatchIsNeeded() test will always deem the kernel as a Protokernel for as long as the OsdConfig functions are not updated. */
        for (i = 2; i < 7; i++) {
            setup(SyscallPatchEntries[i].syscall_num, GetEntryAddress(SyscallPatchEntries[i].syscall_num));
        }
    }
}
