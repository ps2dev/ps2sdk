// This include will allow us to avoid reincluding other headers
#include "irx_imports.h"

#define MODNAME "hello"
IRX_ID("IOP_skeleton", 1, 1);

extern struct irx_export_table _exp_hello;


// This contain our prototype for our export, and we need it to call it inside our _start
#include "hello.h"


// This is a bit like a "main" for IRX files.
int _start(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
    
#ifndef NO_MODULE_UNLOAD_SUPPORT
    if (argc < 0) _stop(argc, argv, startaddr, mi);
#endif
    if (RegisterLibraryEntries(&_exp_hello) != 0) {
        printf("Failed to RegisterLibraryEntries()\n");
        return MODULE_NO_RESIDENT_END;
    }

    hello();

#ifdef MODULE_UNLOAD_ORIGINAL_APPROACH
        return MODULE_REMOVABLE_END;
#else
#ifndef NO_MODULE_UNLOAD_SUPPORT
        if (mi && ((mi->newflags & 2) != 0))
            mi->newflags |= 0x10;
#endif
        return MODULE_RESIDENT_END;
#endif
}

// Here you have to deinit devices, free memory, release exports,
// or whatever cleanup is required depending on what is your module doing 
#ifndef NO_MODULE_UNLOAD_SUPPORT
int _stop(int argc, char *argv[], void *startaddr, ModuleInfo_t *mi)
{
    printf("Unloading %s module\n", MODNAME);
    ReleaseLibraryEntries(&_exp_hello);
    
    return MODULE_NO_RESIDENT_END; //if the deinit process fails for whatever reason. you should return MODULE_REMOVABLE_END
}
#endif
