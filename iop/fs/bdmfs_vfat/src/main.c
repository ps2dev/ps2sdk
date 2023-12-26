#include <bdm.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>
#include <sysmem.h>

#include "fat_driver.h"

// #define DEBUG  //comment out this line when not debugging
#include "mass_debug.h"

#define MAJOR_VER 1
#define MINOR_VER 1

IRX_ID("bdmvfat", MAJOR_VER, MINOR_VER);

extern int InitFAT(void);
extern int InitFS(void);

static struct file_system g_fs = {
    .priv          = NULL,
    .name          = "vfat",
    .connect_bd    = fat_mount,
    .disconnect_bd = fat_forceUnmount,
};

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("BDM VFAT driver (FAT12/16/32) v%d.%d\n", MAJOR_VER, MINOR_VER);

    // initialize the FAT driver
    if (InitFAT() != 0) {
        M_DEBUG("Error initializing FAT driver!\n");
        return MODULE_NO_RESIDENT_END;
    }

    // initialize the file system driver
    if (InitFS() != 0) {
        M_DEBUG("Error initializing FS driver!\n");
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
