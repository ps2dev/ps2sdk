/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# taken from MX4SIO driver for simplicity.
# all credits go to maximus32
*/

#include "ioplib.h"
#include <intrman.h>

int ioplib_iterateByName(const char *name, ioplib_libiterate_cb_t cb, void *userdata)
{
    iop_library_t *libptr;
    int i;
    int count;

    count = 0;
    // Get first loaded library
    libptr = GetLoadcoreInternalData()->let_next;
    // Loop through all loaded libraries
    while (libptr != NULL) {
        // Compare library name only
        for (i = 0; i < 8; i++) {
            if (libptr->name[i] != name[i])
                break;
        }

        // Call callback if match
        if (i == 8) {
            count += 1;
            // Return early if requested
            if (cb(libptr, userdata))
                break;
        }

        // Next library
        libptr = libptr->prev;
    }

    return count;
}

unsigned int ioplib_getTableSize(iop_library_t *lib)
{
    void **exp;
    unsigned int size;

    exp = (lib != NULL) ? lib->exports : NULL;
    size = 0;

    if (exp != NULL)
        while (*exp++ != NULL)
            size++;

    return size;
}

void *ioplib_hookSameExportEntries(iop_library_t *lib, unsigned int entry, void *func)
{
    int table_size;
    int oldstate;
    void *oldfunc;
    unsigned int i;

    table_size = ioplib_getTableSize(lib);
    if (entry >= table_size)
        return NULL;

    CpuSuspendIntr(&oldstate);
    oldfunc = lib->exports[entry];
    for (i = 0; i < table_size; i += 1)
        if (lib->exports[i] == oldfunc)
            lib->exports[i] = func;
    CpuResumeIntr(oldstate);

    return oldfunc;
}

void ioplib_relinkExports(iop_library_t *lib)
{
    struct irx_import_table *table;
    struct irx_import_stub *stub;

    // go through each table that imports the library
    for (table = lib->caller; table != NULL; table = table->next) {
        // go through each import in the table
        for (stub = (struct irx_import_stub *)table->stubs; stub->jump != 0; stub++) {
            // patch the stub to jump to the address specified in the library export table for "fno"
            stub->jump = 0x08000000 | (((u32)lib->exports[stub->fno] << 4) >> 6);
        }
    }
}
