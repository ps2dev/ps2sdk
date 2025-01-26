/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "srxfixup_internal.h"

#if defined(__has_include)
#if __has_include(<endian.h>)
// NOLINTNEXTLINE(misc-include-cleaner)
#include <endian.h>
#if _BYTE_ORDER != _LITTLE_ENDIAN
#define SWAPMEM_IS_NOT_BIG_ENDIAN
#endif
#endif
#endif

#ifdef SWAPMEM_IS_NOT_BIG_ENDIAN
#include <stdint.h>
#include <string.h>
#endif

void swapmemory(void *aaddr, const char *format, unsigned int times)
{
#ifdef SWAPMEM_IS_NOT_BIG_ENDIAN
	unsigned int i;
	size_t j;
	void *aaddr_cur;
	size_t format_len;

	format_len = strlen(format);

	aaddr_cur = aaddr;
	for ( i = 0; i < times; i += 1 )
	{
		for ( j = 0; j < format_len; j += 1 )
		{
			switch ( format[j] )
			{
				case 'c':
					aaddr_cur = (void *)(((uint8_t *)aaddr_cur) + 1);
					break;
				case 's':
					*(uint16_t *)aaddr_cur = bswap16(*(uint16_t *)aaddr_cur);
					aaddr_cur = (void *)(((uint8_t *)aaddr_cur) + 2);
					break;
				case 'l':
					*(uint32_t *)aaddr_cur = bswap32(*(uint32_t *)aaddr_cur);
					aaddr_cur = (void *)(((uint8_t *)aaddr_cur) + 4);
					break;
				default:
					break;
			}
		}
	}
#else
	(void)aaddr;
	(void)format;
	(void)times;
#endif
}
