/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "kernel.h"
#include "syscallnr.h"

void InitDebug(void)
{
    void (*krnl_print)(const char* format, ...) = NULL;

    // Get the address of ResetEE so we can find where printf is located.
    u32* resetEEAddress = (u32*)GetSyscallHandler(__NR_ResetEE);

    // Find the first JAL instruction in ResetEE which should be a printf call.
    ee_kmode_enter();
    for (int i = 0; i < 15; i++)
    {
        // Check if the current instruction is a JAL.
        u32 jalPrintf = resetEEAddress[i];
        if ((jalPrintf & 0xFC000000) == 0xC000000)
        {
            // Get the call target which is the address of printf.
            krnl_print = (void(*)(const char*, ...))(0x80000000 + ((jalPrintf & 0x3FFFFFF) << 2));
            break;
        }
    }
    ee_kmode_exit();

    // If we found the printf function address re-enable the printf syscall.
    if (krnl_print != NULL)
        SetSyscall(__NR__print, krnl_print);
}