/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>

#include "internal.h"

static short int gs_db_draw_buffer=0;

int GsDbGetDrawBuffer(void)
{
	return gs_db_draw_buffer;
}

int GsDbGetDisplayBuffer(void)
{
	return(gs_db_draw_buffer? 0: 1);
}

void GsDbSwapBuffer(void)
{
	gs_db_draw_buffer = gs_db_draw_buffer? 0: 1;
}
