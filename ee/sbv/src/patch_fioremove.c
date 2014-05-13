#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <string.h>

#include "smod.h"

/* from common.c */
extern u8 smem_buf[];
extern int smem_write_word(void *address, u32 value);

#define JMP(addr)	(0x08000000|(0x3ffffff&((addr)>>2)))

int sbv_patch_fioremove(void)
{
	/* This patch is a fix for FILEIO on IOP: 				*/
	/* the handler of rpc_fioremove doesn't exit just after file is		*/
	/* removed due to a bug, and chain with rpc_fiomkdir which have for	*/
	/* effect to create a folder of the same name that the deleted file...	*/
	/* We'll search for FILEIO text section start and patch the rpc		*/
	/* handler to jump to a subroutine to correct this.			*/
	/*									*/
	/* text + 0x1448: patched with the patch given below (16 bytes, this is	*/
	/* 		  our new rpc_fioremove handler)			*/
	/* text + 0x0bb8: patched with 0x0c000512(+text_start) (4 bytes, this	*/
	/*		  is the modified jump to our handler)			*/
	/*									*/
	/* Here is what the 16 bytes patch look like:				*/
	/* 0x0c0001ce(+text_start) 	// jal fio_remove			*/
	/* 0x00000000			// nop					*/
	/* 0x0800033a(+text_start)	// j rpc_handler_exit			*/
	/* 0x00000000			// nop					*/

	smod_mod_info_t mod_info;
	SifDmaTransfer_t dmat;
	static u32 new_fioremove[4] ALIGNED(16)={
		0x0c0001ce,	// jal fio_remove
		0x00000000,	// nop
		0x0800033a,	// j rpc_handler_exit
		0x00000000	// nop
	};
	u32 *p_new_fioremove;
	u32 j_new_fioremove;
	void *patch_adddr;

	memset(&mod_info, 0, sizeof(mod_info));
	int ret = smod_get_mod_by_name("FILEIO_service", &mod_info);
	if ((!ret) || (mod_info.version != 0x101))
		return -1;

	SifInitIopHeap();
	if((patch_adddr = SifAllocIopHeap(sizeof(new_fioremove))) == NULL)
		return -1;

	/* setup our jump opcodes */
	p_new_fioremove = UNCACHED_SEG(new_fioremove);
	p_new_fioremove[0] += ((u32)mod_info.text_start >> 2);
	p_new_fioremove[2] += ((u32)mod_info.text_start >> 2);

	/* apply it */
	dmat.src=new_fioremove;
	dmat.dest=patch_adddr;
	dmat.size=sizeof(new_fioremove);
	dmat.attr=0;
	SifSetDma(&dmat, 1);

	j_new_fioremove =  JMP((u32)patch_adddr);
	smem_write_word((void *)mod_info.text_start + 0x0bb8, j_new_fioremove);

	return 0;
}
