/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Some routines to do some timer work
*/

#include <tamtypes.h>

#ifdef F_cpu_ticks
u32 cpu_ticks(void) {
    u32 out;
    
    asm("mfc0\t%0, $9\n" : "=r"(out));
    return out;
}
#endif
