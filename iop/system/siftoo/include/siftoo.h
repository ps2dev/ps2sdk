/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SIFToo driver.
*/

#ifndef IOP_SIFTOO_H
#define IOP_SIFTOO_H

#include "types.h"
#include "irx.h"

enum sif2_errors {
    SIF2_E_OK,		/* No error.  */

    SIF2_E_INIT = 0xd700, /* Initialization error.  */
};

typedef int (*sif2_pipe_handler_t)(u32, void *, u32);

#define siftoo_IMPORTS_start DECLARE_IMPORT_TABLE(siftoo, 1, 1)
#define siftoo_IMPORTS_end END_IMPORT_TABLE

int sif2_init(void);
#define I_sif2_init DECLARE_IMPORT(4, sif2_init)
int sif2_exit(void);
#define I_sif2_exit DECLARE_IMPORT(5, sif2_exit)

int sif2_mem_read(u32 addr, void *buf, u32 size);
#define I_sif2_mem_read DECLARE_IMPORT(6, sif2_mem_read)
int sif2_mem_write(u32 addr, void *buf, u32 size);
#define I_sif2_mem_write DECLARE_IMPORT(7, sif2_mem_write)

/* Pipe API.  */
int sif2_pipe_create(u32 id, void *buf, u32 size, u32 flags,
        sif2_pipe_handler_t phandler);
#define I_sif2_pipe_create DECLARE_IMPORT(10, sif2_pipe_create)
int sif2_pipe_open(u32 id);
#define I_sif2_pipe_open DECLARE_IMPORT(11, sif2_pipe_open)
int sif2_pipe_close(int pd);
#define I_sif2_pipe_close DECLARE_IMPORT(12, sif2_pipe_close)

int sif2_pipe_read(int pd, void *buf, u32 size);
#define I_sif2_pipe_read DECLARE_IMPORT(13, sif2_pipe_read)
int sif2_pipe_write(int pd, void *buf, u32 size);
#define I_sif2_pipe_write DECLARE_IMPORT(14, sif2_pipe_write)

#endif /* IOP_SIFTOO_H */
