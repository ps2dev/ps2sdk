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
# SBV patch to enable LoadModuleBuffer() via RPC.
*/

#include "tamtypes.h"
#include "kernel.h"
#include "iopheap.h"
#include "string.h"

#include "smem.h"
#include "smod.h"
#include "slib.h"

#define JAL(addr)	(0x0c000000 | (0x3ffffff & ((addr) >> 2)))
#define HI16(addr)	(0x3c110000 | (((addr) >> 16) & 0xffff))	/* lui $s1, HI(addr) */
#define LO16(addr)	(0x36310000 | ((addr) & 0xffff))		/* ori $s1, LO(addr) */

/* This is the routine called by the loadfile RPC dispatcher for LoadModuleBuffer
   over RPC (call #6).  The bracketed operands are the ones patched by the real
   locations in IOP memory before being installed onto the IOP.  */
static u32 lmb_patch[32] = {
	0x27bdffd8,	/*	addiu	$sp, -40	*/
	0xafb00018,	/*	sw	$s0, 0x18($sp)	*/
	0xafbf0020,	/*	sw	$ra, 0x20($sp)	*/
	0x00808021,	/*	move	$s0, $a0	*/
	0x8c840000,	/*	lw	$a0, 0($a0)	*/
	0x0c000000,	/*	jal	[LoadModuleBuffer] */
	0xafb1001c,	/*	 sw	$s1, 0x1c($sp)	*/
	0x3c110000,	/*	lui	$s1, [HI16(result)] */
	0x04400008,	/*	bltz	$v0, 1f		*/
	0x36310000,	/*	 ori	$s1, [LO16(result)] */
	0x00402021,	/*	move	$a0, $v0	*/
	0x26250008,	/*	addiu	$a1, $s1, 8	*/
	0x8e060004,	/*	lw	$a2, 4($s0)	*/
	0x26070104,	/*	addiu	$a3, $s0, 0x104 */
	0x26280004,	/*	addiu	$t0, $s1, 4	*/
	0x0c000000,	/*	jal	[StartModule]	*/
	0xafa80010,	/*	 sw	$t0, 0x10($sp)	*/
	0xae220000,	/* 1:	sw	$v0, 0($s1)	*/
	0x02201021,	/*	move	$v0, $s1	*/
	0x8fbf0020,	/*	lw	$ra, 0x20($sp)	*/
	0x8fb1001c,	/*	lw	$s1, 0x1c($sp)	*/
	0x8fb00018,	/*	lw	$s0, 0x18($sp)	*/
	0x03e00008,	/*	jr	$ra		*/
	0x27bd0028,	/*	 addiu	$sp, 40		*/
	0x00000000, 0x00000000,
	0x7962424c, 0x00004545,	/* "LBbyEE" */
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

int sbv_patch_enable_lmb()
{
	u8 buf[256];
	slib_exp_lib_t *modload_lib = (slib_exp_lib_t *)buf;
	smod_mod_info_t *loadfile_info = (smod_mod_info_t *)buf;
	void *pStartModule, *pLoadModuleBuffer, *lf_text_start, *patch_addr;
	u32 lf_rpc_dispatch, lf_jump_table, result;
	int nexps, id, i;

	/* Locate the modload export library - it must have at least 16 exports.  */
	if ((nexps = slib_get_exp_lib("modload", modload_lib)) < 16)
		return -1;

	pStartModule = modload_lib->exports[8];
	pLoadModuleBuffer = modload_lib->exports[10];

	/* Now we need to find the loadfile module.  */
	memset(buf, 0, sizeof(smod_mod_info_t));
	if (!(id = smod_get_mod_by_name("LoadModuleByEE", loadfile_info)))
		return -1;

	/* Locate the loadfile RPC dispatch code, where the first 4 instructions look like:
	
	   27bdffe8	addiu	$sp, -24
	   2c820006	sltiu	$v0, $a0, 6
	   14400003	bnez	$v0, +12
	   afbf0010	sw	$ra, 0x10($sp)
	*/
	lf_text_start = (void *)(loadfile_info->text_start + 0x400);
	smem_read(lf_text_start, buf, sizeof buf);

	for (i = 0; i < sizeof buf; i += 4) {
		if ((*(u32 *)(buf + i) == 0x27bdffe8) &&
				(*(u32 *)(buf + i + 4) == 0x2c820006) &&
				(*(u32 *)(buf + i + 8) == 0x14400003) &&
				(*(u32 *)(buf + i + 12) == 0xafbf0010))
			break;
	}
	/* This is a special case: if the IOP was reset with an image that contains a
	   LOADFILE that supports LMB, we won't detect the dispatch routine.  If we
	   even got this far in the code then we can return success.  */
	if (i >= sizeof buf)
		return 0;

	/* We need to extract the address of the jump table, it's only 40 bytes in. */
	lf_rpc_dispatch = (u32)lf_text_start + i;
	smem_read((void *)lf_rpc_dispatch, buf, 40);

	lf_jump_table = (*(u16 *)(buf + 0x1c) << 16) + *(s16 *)(buf + 0x24);

	/* Now we can patch our subversive LoadModuleBuffer RPC call.  */
	SifInitIopHeap();
	if (!(patch_addr = SifAllocIopHeap(sizeof lmb_patch)))
		return -1;

	/* result is where the RPC return structure is stored.  */
	result = (u32)patch_addr + 96;
	lmb_patch[5] = JAL((u32)pLoadModuleBuffer);
	lmb_patch[7] = HI16(result);
	lmb_patch[9] = LO16(result);
	lmb_patch[15] = JAL((u32)pStartModule);

	SyncDCache(lmb_patch, (void *)(lmb_patch + 24));
	smem_write(patch_addr, lmb_patch, sizeof lmb_patch);

	/* Finally.  The last thing to do is to patch the loadfile RPC dispatch routine
	   so that it will jump to entry #6 in it's jump table, and to patch the jump
	   table itself.  */
	ee_kmode_enter();
	*(u32 *)(SUB_VIRT_MEM + lf_rpc_dispatch + 4) = 0x2c820007;
	*(u32 *)(SUB_VIRT_MEM + lf_jump_table + 0x18) = (u32)patch_addr;
	ee_kmode_exit();

	return 0;
}
