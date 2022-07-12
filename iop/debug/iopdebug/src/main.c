/**
 * @file
 * IOPDEBUG - IOP debugging library.
 * startup code
 */

#include <tamtypes.h>
#include <stdio.h>
#include <loadcore.h>
#include <thbase.h>
#include "iopdebug.h"

#include "iopdebug_priv.h"

IRX_ID("iopdebug", 1, 0);

extern struct irx_export_table _exp_iopdebug;
int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // install IOP debug system.
    if(iop_dbg_install() != 0)
    {
        //printf("Failed installing IOP debug system!\n");
        return MODULE_NO_RESIDENT_END;
    }

    // register our IRX exports.
    if(RegisterLibraryEntries(&_exp_iopdebug) != 0)
    {
        iop_dbg_remove();

        //printf("Error registering library!\n");
        return MODULE_NO_RESIDENT_END;
    }

    FlushIcache();
    FlushDcache();

    //printf("IOPDEBUG installed!\n");

    return MODULE_RESIDENT_END;
}
// export 2, called when the module is being unloaded(like when IOP is being "rebooted").
int _stop(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    // remove IOP debug system.
    if(iop_dbg_remove() != 0)
    {
        //printf("Failed removing IOP debug handlers!\n");
    }

    return MODULE_NO_RESIDENT_END;
}

