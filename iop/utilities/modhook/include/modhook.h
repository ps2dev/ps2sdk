#ifndef __MODHOOK_H__
#define __MODHOOK_H__

#include <loadcore.h>

/**
 * @file modhook.c
 * @brief IOP module manipulation library for hooking exports.
 * @note depends on: `CpuSuspendIntr` `CpuResumeIntr` `GetLoadcoreInternalData`
 */

/**
 * @brief returns an iop library pointer for the specified IRX module
 * @returns NULL on error, else, a pointer to the struct
 */
iop_library_t *modhook_getModule(const char *name);

/**
 * @brief returns the size of the export table for the specified module
 * @param lib the library to obtain the export table size
 * @returns the ammount of exports registered for that module
 */
unsigned int modhook_getTableSize(iop_library_t *lib);

/**
 * @brief replaces the function called as a module export
 * @param lib The iop_library_t struct of the module to modify
 * @param entry the export number to be modified
 * @param func the function to replace the export with
 * @returns a pointer to the original function
 */
void *modhook_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func);
void modhook_relinkExports(iop_library_t *lib);


#endif