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
# Sub-CPU RAM direct access.
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
