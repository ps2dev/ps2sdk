/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, James Lee (jbit<at>jbit<dot>net)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
*/

#ifndef __PS2SND_MOD_H
#define __PS2SND_MOD_H 1

#define MODNAME "ps2snd"
#define BANNER  MODNAME " %d.%d\n"
#define VER_MAJOR 0
#define VER_MINOR 1

#define OUT_ERROR    0
#define OUT_WARNING  1
#define OUT_INFO     2
#define OUT_DEBUG    3

extern int debug_level;

#define ALIGNED(x) __attribute__((aligned((x))))


#define dprintf(level, format, args...) \
do \
{ \
	if (level<=debug_level)  \
		printf(MODNAME "(%s): " format, __FUNCTION__, ## args); \
} while(0)

#endif /* __PS2SND_MOD_H */

