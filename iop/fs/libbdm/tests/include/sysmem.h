#ifndef __SYSMEM_H__
#define __SYSMEM_H__

#include <stdlib.h>

#define ALLOC_FIRST 0
#define AllocSysMemory(mode, size, address) malloc(size)
#define FreeSysMemory(ptr) free(ptr)

#endif
