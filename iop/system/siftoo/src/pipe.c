/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Simple pipe API for SIFToo.
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
