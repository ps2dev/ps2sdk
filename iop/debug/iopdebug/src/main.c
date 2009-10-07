/*

IOPDEBUG - IOP debugging library.

main.c - startup code

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
    // install IOP debug system.
    if(iop_dbg_install() != 0)
    {
        //printf("Failed installing IOP debug system!\n");
        return(1); // return "non-resident"
    }

    // register our IRX exports.
    if(RegisterLibraryEntries(&_exp_iopdebug) != 0)
    {
        iop_dbg_remove();

        //printf("Error registering library!\n");
        return(1);
    }

    FlushIcache();
    FlushDcache();

    //printf("IOPDEBUG installed!\n");

    return(0); // return "resident"
}
// export 2, called when the module is being unloaded(like when IOP is being "rebooted").
int _stop(int argc, char *argv[])
{
    // remove IOP debug system.
    if(iop_dbg_remove() != 0)
    {
        //printf("Failed removing IOP debug handlers!\n");
    }

    return(1); // return "non-resident"
}

