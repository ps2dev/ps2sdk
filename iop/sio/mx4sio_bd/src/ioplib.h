#ifndef IOPLIB_H
#define IOPLIB_H


#include <loadcore.h>


iop_library_t *ioplib_getByName(const char *name);
unsigned int ioplib_getTableSize(iop_library_t *lib);
void *ioplib_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func);
void ioplib_relinkExports(iop_library_t *lib);


#endif
