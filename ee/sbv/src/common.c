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
 * Internal, common functions and resources.
 */

#include <kernel.h>
#include <sifrpc.h>

#include "slib.h"
#include "smod.h"

#include "common.h"

struct smem_buf smem_buf ALIGNED(64);

/* Do not link to memcmp() from libc, so we only depend on libkernel. */
int __memcmp(const void *s1, const void *s2, unsigned int length)
{
	const char *a = s1;
	const char *b = s2;

	while (length--) {
		if (*a++ != *b++)
			return 1;
	}

	return 0;
}

int smem_write_word(void *address, u32 value)
{
	SifRpcReceiveData_t RData;
	void *pDestRounded;
	int result;
	SifDmaTransfer_t dmat;

	pDestRounded=(void*)(((u32)address)&0xFFFFFFC0);
	SyncDCache(&smem_buf, smem_buf.bytes+64);
	if((result=SifRpcGetOtherData(&RData, pDestRounded, &smem_buf, 64, 0))>=0)
	{
		*(u32*)UNCACHED_SEG((&smem_buf.bytes[((u32)address&0x3F)]))=value;

		dmat.src=&smem_buf;
		dmat.dest=pDestRounded;
		dmat.size=64;
		dmat.attr=0;
		SifSetDma(&dmat, 1);
	}

	return result;
}
