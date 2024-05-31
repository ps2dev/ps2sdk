/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACMEME_INTERNAL_H
#define _ACMEME_INTERNAL_H

#include <acmeme.h>
#include <irx_imports.h>

typedef acUint32 acMemAddr;
struct ac_mem_vec
{
	acInt32 mv_result;
	acMemAddr mv_src;
	acMemAddr mv_dst;
	acUint32 mv_size;
};

typedef struct ac_mem_vec acMemVecData;
typedef acMemVecData *acMemVecT;

struct meme_ram
{
	acRamData mr_ram;
	int mr_thid;
	int mr_result;
};

struct ac_memsif_reply
{
	acInt32 error;
	acInt32 result;
	// cppcheck-suppress unusedStructMember
	acUint32 padding[2];
};

union ac_memsif_pkt
{
	struct ac_memsif_reply rpl;
};

struct meme_softc
{
	union ac_memsif_pkt pkt;
	acInt32 status;
	acInt32 thid;
	void *buf;
	acInt32 size;
	acMemVecData mvec[16];
};

typedef int (*meme_xfer_t)(acMemAddr addr, void *buf, int size);

typedef int (*meme_ops_t)(struct meme_softc *memec, struct ac_memsif_reply *rpl, const void *arg, int size);

struct ac_memsif_init
{
	// cppcheck-suppress unusedStructMember
	acUint32 padding[2];
	acUint32 start;
	acUint32 size;
};

struct ac_memsif_xfer
{
	acUint32 padding[2];
	acMemVecT mvec;
	acUint32 item;
};

#endif