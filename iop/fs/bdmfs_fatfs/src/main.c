#include <bdm.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>
#include <sysmem.h>
#include <cdvdman.h>

#include "ff.h"

//#define DEBUG  //comment out this line when not debugging
#include "module_debug.h"

#define MAJOR_VER 1
#define MINOR_VER 4

IRX_ID("bdmff", MAJOR_VER, MINOR_VER);

extern int InitFAT(void);
extern int InitFS(void);
extern int connect_bd();
extern void disconnect_bd();

static struct file_system g_fs = {
    .priv = NULL,
    .name = "fatfs",
    .connect_bd = connect_bd,
    .disconnect_bd = disconnect_bd,
};

int _start(int argc, char *argv[])
{
    printf("BDM FatFs driver (FAT/exFAT) v%d.%d\n", MAJOR_VER, MINOR_VER);

    // initialize the FAT driver
    if (InitFAT() != 0) {
        M_DEBUG("Error initializing FatFs driver!\n");
        return MODULE_NO_RESIDENT_END;
    }

    // initialize the file system driver
    if (InitFS() != 0) {
        M_DEBUG("Error initializing FatFs driver!\n");
        return MODULE_NO_RESIDENT_END;
    }

    // Connect to block device manager
    bdm_connect_fs(&g_fs);

    // return resident
    return MODULE_RESIDENT_END;
}

void *malloc(int size)
{
    void *result;
    int OldState;

    CpuSuspendIntr(&OldState);
    result = AllocSysMemory(ALLOC_FIRST, size, NULL);
    CpuResumeIntr(OldState);

    return result;
}

void free(void *ptr)
{
    int OldState;

    CpuSuspendIntr(&OldState);
    FreeSysMemory(ptr);
    CpuResumeIntr(OldState);
}

DWORD get_fattime(void)
{
    // ps2 specific routine to get time and date
    int year, month, day, hour, minute, sec;
    sceCdCLOCK cdtime;

    if (sceCdReadClock(&cdtime) != 0 && cdtime.stat == 0) {
        sec = btoi(cdtime.second);
        minute = btoi(cdtime.minute);
        hour = btoi(cdtime.hour);
        day = btoi(cdtime.day);
        month = btoi(cdtime.month & 0x7F); // Ignore century bit (when an old CDVDMAN is used).
        year = btoi(cdtime.year) + 2000;
    } else {
        year = 2005;
        month = 1;
        day = 6;
        hour = 14;
        minute = 12;
        sec = 10;
    }

    /* Pack date and time into a DWORD variable */
    return ((DWORD)(year - 1980) << 25) | ((DWORD)month << 21) | ((DWORD)day << 16) | ((DWORD)hour << 11) | ((DWORD)minute << 5) | ((DWORD)sec >> 1);
}
