#define MAJOR_VER 1
#define MINOR_VER 1
#define MODNAME "IEEE1394_bd"

#include "sbp2_disk.h"
#include "scsi.h"
#include <irx.h>
#include <loadcore.h>
#include <stdio.h>

#include "module_debug.h"

IRX_ID(MODNAME, MAJOR_VER, MINOR_VER);

int _start(int argc, char** argv)
{
    M_PRINTF("IEEE1394 Driver v%d.%d\n", MAJOR_VER, MINOR_VER);

    // initialize the SCSI driver
    if (scsi_init() != 0) {
        M_PRINTF("ERROR: initializing SCSI driver!\n");
        return MODULE_NO_RESIDENT_END;
    }

    // initialize the IEEE1394 driver
    init_ieee1394DiskDriver();

    return MODULE_RESIDENT_END;
}
