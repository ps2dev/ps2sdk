/* smem.h - Sub-CPU memory interface.
 *
 * Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This code is licensed under the Academic Free License v2.0.
 * See the file LICENSE included with this distribution for licensing terms.
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
