#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <string.h>

#include "slib.h"
#include "smod.h"

#include "common.h"

#define JMP(addr)	(0x08000000|(0x3ffffff&((addr)>>2)))
#define JAL(addr)	(0x0c000000 | (0x3ffffff & ((addr) >> 2)))

int sbv_patch_fileio(void)
{
	/* This patch is a fix for FILEIO on the IOP:
		The handler of rpc_fioremove doesn't exit right after file is
		removed due to the break being missing, which ends up
		calling the mkdir RPC handler in succession.
		A folder with the same name as the deleted file will be created.
		We'll search for FILEIO text section start and patch the rpc
		handler to jump to a subroutine to correct this.

		The RPC handlers for getstat() and dread() do not suspend interrupts
		before invoking sceSifSetDma(), which may result in a race condition
		that destroys the internal SIF data structure within SIFMAN.	*/

	smod_mod_info_t mod_info;
	SifDmaTransfer_t dmat;
	static u32 new_fileio[20] ALIGNED(16)={
		//sceFioRemove fix
		0x0c0001ce,	// jal		+0x738 <- jal fio_remove
		0x00000000,	// nop
		0x0800033a,	// j		+0xce8 <- j rpc_handler_exit
		0x00000000,	// nop
		//sceFioGetstat()/sceFioDread() fix
		0x27bdfff0,	// addiu	sp,sp,-16
		0xafbf0000,	// sw		ra,0(sp)
		0xafa40004,	// sw		a0,4(sp)
		0xafa50008,	// sw		a1,8(sp)
		0x0c000423,	// jal		+0x108c <- jal CpuSuspendIntr
		0x27a4000c,	// addiu	a0,sp,12
		0x8fa40004,	// lw		a0,4(sp)
		0x0c000430,	// jal		+0x10c0 <- jal sceSifSetDma
		0x8fa50008,	// lw		a1,8(sp)
		0x8fa4000c,	// lw		a0,12(sp)
		0x0c000425,	// jal		+0x1094 <- jal CpuResumeIntr
		0xafa20004,	// sw		v0,4(sp)
		0x8fbf0000,	// lw		ra,0(sp)
		0x8fa20004,	// lw		v0,4(sp)
		0x03e00008,	// jr		ra
		0x27bd0010,	// addiu	sp,sp,16
	};
	u32 *p_new_fileio;
	u32 new_jump_op;
	void *patch_addr;

	memset(&mod_info, 0, sizeof(mod_info));
	int ret = smod_get_mod_by_name("FILEIO_service", &mod_info);
	if ((!ret) || (mod_info.version != 0x101))
		return -1;

	SifInitIopHeap();
	if((patch_addr = SifAllocIopHeap(sizeof(new_fileio))) == NULL)
		return -1;

	/* setup our jump opcodes */
	p_new_fileio = UNCACHED_SEG(new_fileio);

	//For the sceFioRemove() patch
	p_new_fileio[0] += ((u32)mod_info.text_start >> 2);
	p_new_fileio[2] += ((u32)mod_info.text_start >> 2);

	//For the sceFioGetstat() and sceFioDread() patch
	p_new_fileio[8] += ((u32)mod_info.text_start >> 2);
	p_new_fileio[11] += ((u32)mod_info.text_start >> 2);
	p_new_fileio[14] += ((u32)mod_info.text_start >> 2);

	/* apply it */
	dmat.src=new_fileio;
	dmat.dest=patch_addr;
	dmat.size=sizeof(new_fileio);
	dmat.attr=0;
	SifSetDma(&dmat, 1);

	//For the dump to sceRemove()
	new_jump_op =  JMP((u32)patch_addr);
	smem_write_word((void *)mod_info.text_start + 0x0bb8, new_jump_op);
	new_jump_op =  JAL((u32)patch_addr + 16);
	//For the jumps to sceSifSetDma within sceGetstat() and sceDread():
	smem_write_word((void *)mod_info.text_start + 0x09cc, new_jump_op);
	smem_write_word((void *)mod_info.text_start + 0x0a58, new_jump_op);

	return 0;
}
