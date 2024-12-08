#ifndef IOPLIB_H
#define IOPLIB_H


#include <loadcore.h>


extern iop_library_t *ioplib_getByName(const char *name);
extern unsigned int ioplib_getTableSize(iop_library_t *lib);
extern void *ioplib_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func);
extern void ioplib_relinkExports(iop_library_t *lib);


#endif
