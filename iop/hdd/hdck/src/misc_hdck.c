/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <atad.h>
#include <cdvdman.h>
#include <errno.h>
#include <intrman.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>

#include "apa-opt.h"
#include "libapa.h"
#include "misc_hdck.h"

int HdckUnlockHdd(int unit)
{
    unsigned char id[32];
    int result;

    if ((result = apaGetIlinkID(id)) == 0) {
        result = ata_device_sce_sec_unlock(unit, id);
    }

    return result;
}
