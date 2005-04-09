/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Routines for accessing the EE's on-chip serial port.
*/

#include <kernel.h>

#define SYSCALL_1		0x80
#define SYSCALL_2		0x83
#define SYSCALL_1_ADDR 	0xF1EEA931
#define SYSCALL_2_ADDR 	0x83554146

#ifdef F_GetSyscall
static u32* g_pSyscallTable = NULL;

/** Initialise the syscall table address */
static void InitSyscallTable(void)
{
	u32 *pAddr;

	DIntr();
	ee_kmode_enter();
	pAddr = (u32 *) 0x800002f0;
	g_pSyscallTable = (u32 *) ((pAddr[0] << 16) | (pAddr[2] & 0xFFFF));
	ee_kmode_exit();
	EIntr();

}

/** Get the address of a syscall function.
    @param syscall - The syscall number.
	@return - The address of the syscall function (or NULL)
*/
void *GetSyscall(u32 syscall)
{
	u32 addr = 0;

	if(g_pSyscallTable == NULL)
	{
		InitSyscallTable();
	}

	if(g_pSyscallTable != NULL)
	{
		ee_kmode_enter();
		addr = g_pSyscallTable[syscall];
		ee_kmode_exit();
	}

	return (void *) addr;
}
#endif
