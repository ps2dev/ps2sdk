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
 * SBV patch to enable LoadModuleBuffer() via RPC.
 */

#include <tamtypes.h>
#include <kernel.h>
#include <iopheap.h>
#include <sifrpc.h>
#include <string.h>

#include "smem.h"
#include "smod.h"
#include "slib.h"

#include "common.h"

/* from common.c */
extern struct smem_buf smem_buf;

/* from slib.c */
extern slib_exp_lib_list_t _slib_cur_exp_lib_list;

#define JAL(addr)	(0x0c000000 | (0x3ffffff & ((addr) >> 2)))
#define HI16(addr)	(0x3c110000 | (((addr) >> 16) & 0xffff))	/* lui $s1, HI(addr) */
#define LO16(addr)	(0x36310000 | ((addr) & 0xffff))		/* ori $s1, LO(addr) */

int sbv_patch_enable_lmb(void)
{
	/* This is the routine called by the loadfile RPC dispatcher for LoadModuleBuffer
	   over RPC (call #6).  The bracketed operands are the ones patched by the real
	   locations in IOP memory before being installed onto the IOP.  */
	static u32 lmb_patch[32] ALIGNED(64) = {
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
	u8 buf[256];
	SifRpcReceiveData_t RData;
	slib_exp_lib_t *modload_lib = (slib_exp_lib_t *)buf;
	smod_mod_info_t loadfile_info;
	void *pStartModule, *pLoadModuleBuffer, *patch_addr, *lf_rpc_dispatch, *lf_jump_table_end, *lf_fno_check;
	unsigned short int JumpTableOffset_hi, JumpTableOffset_lo;
	u32 result, *data;
	int id;
	SifDmaTransfer_t dmat;

	memset(&_slib_cur_exp_lib_list, 0, sizeof(slib_exp_lib_list_t));

	/* Locate the modload export library - it must have at least 16 exports.  */
	if (slib_get_exp_lib("modload", modload_lib) < 16)
		return -1;

	pStartModule = modload_lib->exports[8];
	pLoadModuleBuffer = modload_lib->exports[10];

	/* Now we need to find the loadfile module.  */
	if (!(id = smod_get_mod_by_name("LoadModuleByEE", &loadfile_info)))
		return -1;

	/*	In the Sony original, the whole text section of LOADFILE is scanned for the pattern.

		But that required a larger portion of EE RAM, which means that memory
		will have to be allocated. It will also mean that this library will become dependent on additional memory (i.e. 0x01e00000-0x01e80000).
		I think that it's fine to hardcode the address because the affected LOADFILE module is the same in all boot ROMs.
		If someday, somebody finds an unusual (perhaps even prototype) PlayStation 2 console that has an older LOADFILE module that needs this patch too,
		this patch can be revised.

		The original sbv library's LMB patch starts scanning at offset 0x400, in a mere 256-byte radius.
			Locate the loadfile RPC dispatch code, where the first 4 instructions look like:

			27bdffe8	addiu	$sp, -24
			2c820006	sltiu	$v0, $a0, 6
			14400003	bnez	$v0, +12
			afbf0010	sw	$ra, 0x10($sp)	*/

	if(loadfile_info.text_size < 0x4c4 + 128)
		return -1;

	lf_rpc_dispatch = (void *)(loadfile_info.text_start + 0x4c4);
	SyncDCache(&smem_buf, smem_buf.bytes+128);
	if(SifRpcGetOtherData(&RData, (void*)lf_rpc_dispatch, &smem_buf, 128, 0)>=0){
		data=smem_buf.words;
		if(data[0]==0x27bdffe8 && data[1]==0x2c820006 && data[2]==0x14400003 && data[3]==0xafbf0010 && data[5]==0x00001021 && data[6]==0x00041080){
			lf_fno_check = (void*)(lf_rpc_dispatch+4);

			/* We need to extract the address of the jump table. */
			JumpTableOffset_hi=*(unsigned short int*)&data[7];
			JumpTableOffset_lo=*(unsigned short int*)&data[9];

			lf_jump_table_end = (void*)((JumpTableOffset_hi<<16) + (short int)JumpTableOffset_lo + 0x18);

			/* Now we can patch our subversive LoadModuleBuffer RPC call.  */
			SifInitIopHeap();
			if ((patch_addr = SifAllocIopHeap(sizeof lmb_patch)) == NULL)
				return -1;

			/* result is where the RPC return structure is stored.  */
			result = (u32)patch_addr + 96;
			lmb_patch[5] = JAL((u32)pLoadModuleBuffer);
			lmb_patch[7] = HI16(result);
			lmb_patch[9] = LO16(result);
			lmb_patch[15] = JAL((u32)pStartModule);

			SyncDCache(lmb_patch, (void *)(lmb_patch + 24));

			dmat.src=lmb_patch;
			dmat.size=sizeof(lmb_patch);
			dmat.dest=patch_addr;
			dmat.attr=0;

			SifSetDma(&dmat, 1);

			/* Finally.  The last thing to do is to patch the loadfile RPC dispatch routine
			   so that it will jump to entry #6 in it's jump table, and to patch the jump
			   table itself.  */
			smem_write_word(lf_jump_table_end, (u32)patch_addr);
			smem_write_word(lf_fno_check, 0x2C820007);	//sltiu v0, a0, $0007

			return 0;
		}
	}

	return 1;
}
