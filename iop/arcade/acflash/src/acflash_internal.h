/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACFLASH_INTERNAL_H
#define _ACFLASH_INTERNAL_H

#include <acflash.h>
#include <irx_imports.h>

typedef acUint16 flash_data_t;
typedef acUint32 flash_addr_t;
typedef volatile flash_data_t *flash_ptr_t;

struct flash_ops
{
	// cppcheck-suppress unusedStructMember
	char *fo_name;
	acUint32 fo_bsize;
	acUint32 fo_blocks;
	// cppcheck-suppress unusedStructMember
	int fo_padding;
	int (*fo_erase)(flash_addr_t addr);
	int (*fo_program)(flash_addr_t addr, const flash_data_t *buf, int size);
	int (*fo_reset)(flash_addr_t addr);
	int (*fo_status)(flash_addr_t addr);
};

typedef struct flash_ops *flash_ops_t;

struct flash_softc
{
	acInt32 status;
	acInt32 size;
	flash_ops_t ops;
	acUint32 padding;
};

typedef flash_ops_t (*flash_probe_t)(flash_addr_t addr);

extern flash_ops_t flash_probe_i28f640f5(flash_addr_t addr);
extern flash_ops_t flash_probe_mbm29f033c(flash_addr_t addr);

#endif
