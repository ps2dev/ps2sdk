/*
 * pipe.c - Simple pipe API for SIFToo.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * Licensed under the Academic Free License version 2.0.
 */

#include "types.h"
#include "defs.h"

#include "sbusintr.h"
#include "siftoo.h"

int sif2_pipe_create(u32 id, void *buf, u32 size, u32 flags,
		sif2_pipe_handler_t phandler)
{
	return 0;
}

int sif2_pipe_open(u32 id)
{
	return 0;
}

int sif2_pipe_close(int pd)
{
	return 0;
}

int sif2_pipe_read(int pd, void *buf, u32 size)
{
	return 0;
}

int sif2_pipe_write(int pd, void *buf, u32 size)
{
	return 0;
}
