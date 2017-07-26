/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Sub-CPU memory interface.
 */

#ifndef __SMEM_H__
#define __SMEM_H__

#include <tamtypes.h>

/**
 * IOP RAM is mysteriously mapped into EE HW space at this address.  I first
 * noticed it was used by SCE in the PS2/Linux kernel, where it's used in the
 * USB core driver.  It's also used in a few places in the EE kernel.
 *
 * I'm not exactly sure how the mapping looks to the bus, reads and writes to
 * it are extremely fast, and data width hasn't been a problem for me yet (so
 * you can use memcpy(), and obviously 16-byte reads and writes work).
 *
 * For writes to IOP address space, the data cache MUST be first written back with SyncDCache().
 * I don't why why it is required, but writes do not always seem to take place properly without it.
 *
 * For the PlayStation 3, this window does not seem to be correctly emulated.
 * Although its EE kernel appears to still uses it, homebrew software cannot seem to use this window properly.
*/
#define SUB_VIRT_MEM	0xbc000000

#ifdef __cplusplus
extern "C" {
#endif

u32 smem_read(void *addr, void *buf, u32 size);
u32 smem_write(void *addr, void *buf, u32 size);

#ifdef __cplusplus
}
#endif

#endif /* __SMEM_H__ */
