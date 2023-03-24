#include <errno.h>
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>

#include "sbp2_disk.h"
#include "fat_driver.h"
#include "mass_debug.h"
IRX_ID(MODNAME, 0x01, 0x07);

int InitFS(void);

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    if (InitFAT() != 0) {
        printf("Error initializing FAT driver!\n");
        return MODULE_NO_RESIDENT_END;
    }

    init_ieee1394DiskDriver();
    InitFS();

    return MODULE_RESIDENT_END;
}
