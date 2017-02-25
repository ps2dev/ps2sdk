/**
 * @file
 * Patch user memory clear on EE restart.
 */

#include <tamtypes.h>
#include <kernel.h>

int sbv_patch_user_mem_clear(void *start)
{
	int ret = -1;
	u32 *p;

	DI();
	ee_kmode_enter();

	for (p = (unsigned int*)0x80001000; p < (unsigned int*)0x80080000; p++) {
		/*
		 * Search for function call and patch $a0
		 *  lui  $a0, 0x0008
		 *  jal  InitializeUserMemory
		 *  ori  $a0, $a0, 0x2000
		 */
		if (p[0] == 0x3c040008 && (p[1] & 0xfc000000) == 0x0c000000 && p[2] == 0x34842000) {
			p[0] = 0x3c040000 | ((unsigned int)start >> 16);
			p[2] = 0x34840000 | ((unsigned int)start & 0xffff);
			ret = 0;
			break;
		}
	}

	ee_kmode_exit();
	EI();

	return ret;
}
