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
# Sub-CPU memory interface.
*/

#ifndef SBV_SMEM_H
#define SBV_SMEM_H

#include "tamtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* IOP RAM is mysteriously mapped into EE HW space at this address.  I first
   noticed it was used by SCE in the PS2/Linux kernel, where it's used in the
   USB core driver.  It's also used in a few places in the EE kernel.

   I'm not exactly sure how the mapping looks to the bus, reads and writes to
   it are extremely fast, and data width hasn't been a problem for me yet (so
   you can use memcpy(), and obviously 16-byte reads and writes work).  */
#define SUB_VIRT_MEM	0xbc000000

u32 smem_read(void *addr, void *buf, u32 size);
u32 smem_write(void *addr, void *buf, u32 size);

#ifdef __cplusplus
}
#endif

#endif /* SBV_SMEM_H */
