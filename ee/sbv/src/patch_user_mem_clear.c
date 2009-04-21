#include <tamtypes.h>
#include <kernel.h>

/**
 * sbv_patch_user_mem_clear - Patch user memory clear on EE restart.
 * @start: address above which all user memory is cleared
 * @return: 0: success, -1: error
 *
 * LoadExecPS2() wipes all user-space memory above 0x82000.  With this patch,
 * you can define a different start address to prevent your data from being
 * overwritten.  In order to completely disable the memory clear, simply pass
 * 0x02000000 to it.
 */
int sbv_patch_user_mem_clear(u32 start)
{
	int ret = -1;
	u32 *p;

	DI();
	ee_kmode_enter();

	for (p = (u32*)0x80001000; p < (u32*)0x80080000; p++) {
		/*
		 * Search for function call and patch $a0
		 *  lui  $a0, 0x0008
		 *  jal  InitializeUserMemory
		 *  ori  $a0, $a0, 0x2000
		 */
		if (p[0] == 0x3c040008 && (p[1] & 0xfc000000) == 0x0c000000 && p[2] == 0x34842000) {
			p[0] = 0x3c040000 | (start >> 16);
			p[2] = 0x34840000 | (start & 0xffff);
			ret = 0;
			break;
		}
	}

	ee_kmode_exit();
	EI();

	return ret;
}
