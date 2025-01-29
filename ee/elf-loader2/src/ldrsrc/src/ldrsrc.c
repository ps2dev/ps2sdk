/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <elf_loader_common.h>
#include <kernel.h>
#include <ps2sdkapi.h>
#include <stdint.h>
#include <string.h>
#include <tamtypes.h>

#ifdef LOADER_ENABLE_DEBUG_COLORS
#define SET_GS_BGCOLOUR(colour)                                                                                        \
	do                                                                                                                   \
	{                                                                                                                    \
		*((volatile unsigned long int *)0x120000E0) = colour;                                                              \
	} while ( 0 )
#else
#define SET_GS_BGCOLOUR(colour)                                                                                        \
	do                                                                                                                   \
	{                                                                                                                    \
	} while ( 0 )
#endif

// Color status helper in BGR format
#define WHITE_BG 0xFFFFFF    // start main
#define CYAN_BG 0xFFFF00     // move memory
#define GREEN_BG 0x00FF00    // set memory to 0
#define RED_BG 0x0000FF      // never encountered execution command
#define MAGENTA_BG 0xFF00FF  // malformed loader info
#define BROWN_BG 0x2A2AA5    // before FlushCache
#define PURPLE_BG 0x800080   // before ExecPS2

//--------------------------------------------------------------
// Redefinition of init/deinit libc:
//--------------------------------------------------------------
// DON'T REMOVE, as it is for reducing binary size.
// These functions are defined as weak in /libc/src/init.c
//--------------------------------------------------------------
void _libcglue_init() {}
void _libcglue_deinit() {}
void _libcglue_args_parse(int argc, char **argv) {}

DISABLE_PATCHED_FUNCTIONS();
DISABLE_EXTRA_TIMERS_FUNCTIONS();
PS2_DISABLE_AUTOSTART_PTHREAD();

void __attribute__((section(".start"))) ldr_proc(void);
void ldr_proc(void)
{
	elf_loader_loaderinfo_t *ldrinfo;
	elf_loader_arginfo_t *arginfo;

	SET_GS_BGCOLOUR(WHITE_BG);
	ldrinfo = (void *)0x11004000;
	arginfo = (void *)0x00088000;

	{
		int i;
		for ( i = 0; i < (sizeof(ldrinfo->items) / sizeof(ldrinfo->items[0])); i += 1 )
		{
			elf_loader_loaderinfo_item_t *item;

			item = &(ldrinfo->items[i]);
			if ( item->dest_addr != NULL && item->src_addr != NULL && item->size != 0 )
			{
				SET_GS_BGCOLOUR(CYAN_BG);
				memmove(item->dest_addr, item->src_addr, item->size);
			}
			else if ( item->dest_addr != NULL && item->src_addr == NULL && item->size != 0 )
			{
				SET_GS_BGCOLOUR(GREEN_BG);
				memset(item->dest_addr, 0, item->size);
			}
			else if ( item->dest_addr != NULL && item->size == 0 )
			{
				SET_GS_BGCOLOUR(BROWN_BG);
				FlushCache(0);
				FlushCache(2);
				SET_GS_BGCOLOUR(PURPLE_BG);
				ExecPS2(item->dest_addr, item->src_addr, arginfo->argc, arginfo->argv);
			}
			else
			{
				SET_GS_BGCOLOUR(MAGENTA_BG);
				break;
			}
		}
	}
	SET_GS_BGCOLOUR(RED_BG);
	__builtin_trap();
}
