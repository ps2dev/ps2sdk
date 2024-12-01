#include "modhook.h"
#include <intrman.h>
#include <stdint.h>

iop_library_t *modhook_getModule(const char *name)
{
    iop_library_t *libptr;
    int i;

    // Get first loaded library
    libptr = GetLoadcoreInternalData()->let_next;
    // Loop through all loaded libraries
    while (libptr != NULL) {
        // Compare library name only
        for (i = 0; i < 8; i++) {
            if (libptr->name[i] != name[i])
                break;
        }

        // Return if match
        if (i == 8)
            return libptr;

        // Next library
        libptr = libptr->prev;
    }

    return NULL;
}

unsigned int modhook_getTableSize(iop_library_t *lib)
{
    void **exp;
    unsigned int size;

    exp = NULL;
    if (lib != NULL) {
        exp = lib->exports;
    }
    size = 0;

    if (exp != NULL)
        while (*exp++ != NULL)
            size++;

    return size;
}

void *modhook_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func)
{
    if (entry < modhook_getTableSize(lib)) {
        int oldstate;
        void **exp, *temp;

        exp = &lib->exports[entry];

        CpuSuspendIntr(&oldstate);
        temp = *exp;
        *exp = func;
        func = temp;
        CpuResumeIntr(oldstate);

        return func;
    }

    return NULL;
}

void modhook_relinkExports(iop_library_t *lib)
{
    struct irx_import_table *table;
    struct irx_import_stub *stub;

    // go through each table that imports the library
    for (table = lib->caller; table != NULL; table = table->next) {
        // go through each import in the table
        for (stub = (struct irx_import_stub *)table->stubs; stub->jump != 0; stub++) {
            // patch the stub to jump to the address specified in the library export table for "fno"
            stub->jump = 0x08000000 | (((uint32_t)lib->exports[stub->fno] << 4) >> 6);
        }
    }
}