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

#ifndef IOPLIB_H
#define IOPLIB_H


#include <loadcore.h>

typedef int (*ioplib_libiterate_cb_t)(iop_library_t *lib, void *userdata);

/** @brief Iterate for each library found by name
 * @param name Name of the module to search for
 * @param callback Callback called when the library name matches
 * @param userdata User data to pass to the callback
 * @return count of items found
 */
extern int ioplib_iterateByName(const char *name, ioplib_libiterate_cb_t cb, void *userdata);
extern unsigned int ioplib_getTableSize(iop_library_t *lib);
/** @brief Hook all IRX module exported functions that matches the specified entry.
 * @param lib Library object of the module to hook
 * @param entry Export number to be hooked
 * @param func Hook function
 * @return on error: NULL | on success: original function pointer
 */
extern void *ioplib_hookSameExportEntries(iop_library_t *lib, unsigned int entry, void *func);
extern void ioplib_relinkExports(iop_library_t *lib);


#endif
