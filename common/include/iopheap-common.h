/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Common definitions for iopheap between the client and server sides of the FILEIO protocol.
 */

#ifndef __IOPHEAP_COMMON_H__
#define __IOPHEAP_COMMON_H__

#include <tamtypes.h>

// iopheap common definitions

#define LIH_PATH_MAX 252

struct _iop_load_heap_arg
{
    union
    {
        void *addr;
        int result;
    } p;
    char path[LIH_PATH_MAX];
};

#endif /* __IOPHEAP_COMMON_H__ */
