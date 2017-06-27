/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <kernel.h>

int kCopy(void *dest, const void *src, int size)
{
	const u32 *pSrc;
	u32 *pDest;
	int i;

	if(size/4 != 0)
	{
		pDest = (u32*)dest;
		pSrc = (const u32*)src;
		for(i=0; i<size; i+=4,pDest++,pSrc++)
			*pDest = *pSrc;
	}

	return 0;
}
