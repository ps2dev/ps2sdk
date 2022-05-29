/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE kernel update setup functions
 */

#include <kernel.h>

#ifdef F_kCopy
int kCopy(void *dest, const void *src, int size)
{
	if(size/4 != 0)
	{
		const u32 *pSrc;
		u32 *pDest;
		int i;

		pDest = (u32*)dest;
		pSrc = (const u32*)src;
		for(i=0; i<size; i+=4,pDest++,pSrc++)
			*pDest = *pSrc;
	}

	return 0;
}
#endif

#ifdef F_kCopyBytes
int kCopyBytes(void *dest, const void *src, int size)
{
	if(size != 0)
	{
		const u8 *pSrc;
		u8 *pDest;
		int i;

		pDest = (u8*)dest;
		pSrc = (const u8*)src;
		for(i=0; i<size; i++,pDest++,pSrc++)
			*pDest = *pSrc;
	}

	return 0;
}
#endif
