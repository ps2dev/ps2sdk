/* smem.c - Sub-CPU RAM direct access.
 *
 * Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
 *
 * This code is licensed under the Academic Free License v2.0.
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "kernel.h"
#include "string.h"

#include "smem.h"

u32 smem_read(void *addr, void *buf, u32 size)
{
	DI();
	ee_kmode_enter();

	memcpy(buf, addr + SUB_VIRT_MEM, size);

	ee_kmode_exit();
	EI();

	return size;
}

u32 smem_write(void *addr, void *buf, u32 size)
{
	DI();
	ee_kmode_enter();

	memcpy(addr + SUB_VIRT_MEM, buf, size);

	ee_kmode_exit();
	EI();

	return size;
}
