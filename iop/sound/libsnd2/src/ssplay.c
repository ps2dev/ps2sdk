/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

void SsSeqPlay(s16 sep_no, char play_mode, s16 l_count)
{
	Snd_SetPlayMode(sep_no, 0, play_mode, l_count);
}

void SsSepPlay(s16 sep_no, s16 seq_no, char play_mode, s16 l_count)
{
	Snd_SetPlayMode(sep_no, seq_no, play_mode, l_count);
}
