/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * PS2 ROM0 Info.
 * Some useful method for extracting info from ROM0
 */

#include <rom0_info.h>

#include <iopheap.h>
#include <string.h>
#include <sifrpc.h>
#include <kernel.h>
#include <syscallnr.h>

struct rom0_info_data
{
    /** stores romname of ps2 */
    char m_romver[14];
};

extern struct rom0_info_data g_rom0_info_data;

#ifdef F__info_internals
struct rom0_info_data g_rom0_info_data;
#endif

#ifdef F_SetupRomInfo
void SetupRomInfo(void)
{
    void *iop_addr;
    SifRpcReceiveData_t rdata;
    u8 buf[64] __attribute__((__aligned__(64)));

    /* ROMVER needs to be read from the IOP due to PPCIOP region patching. */
    /* only read in the romname the first time */
    if (g_rom0_info_data.m_romver[0])
        return;

    /* SYSMEM allocates in units of 256. */
    if (!(iop_addr = SifAllocIopHeap(256)))
        return;

    /* ROMVER is known to be 16 bytes. */
    if (SifLoadIopHeap("rom:ROMVER", iop_addr) < 0)
    {
        SifFreeIopHeap(iop_addr);
        return;
    }
    SyncDCache(buf, buf + sizeof(buf));
    if (sceSifGetOtherData(&rdata, iop_addr, buf, sizeof(buf), 0) < 0)
    {
        SifFreeIopHeap(iop_addr);
        return;
    }
    memcpy(g_rom0_info_data.m_romver, UNCACHED_SEG(buf), sizeof(g_rom0_info_data.m_romver));

    SifFreeIopHeap(iop_addr);
}
#endif

#ifdef F_GetRomName
char *GetRomName(char *romname)
{
    SetupRomInfo();
    /* Explicitly copy 14 bytes to the buffer, without NULL terminator */
    memcpy(romname, g_rom0_info_data.m_romver, sizeof(g_rom0_info_data.m_romver));
    return romname;
}
#endif

#ifdef F_IsDESRMachine
int IsDESRMachine(void)
{
    /* Use the existence of ExecPSX syscall as the determination. */
    return GetSyscallHandler(__NR_ExecPSX) ? 1 : 0;
}
#endif

#ifdef F_IsT10K
int IsT10K(void)
{
    SetupRomInfo();
    return (g_rom0_info_data.m_romver[4] == 'T' && g_rom0_info_data.m_romver[5] != 'Z') ? 1 : 0;
}
#endif
