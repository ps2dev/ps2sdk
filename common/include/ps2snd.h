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

#ifndef __PS2SND_H
#define __PS2SND_H 1

#include <libsd.h>

#define STREAM_STEREO 1
#define STREAM_END_CLOSE  0x0000
#define STREAM_END_REPEAT 0x1000

#define BINDID_PS2SND 0x80068000

#define PS2SND_Init                   4
#define PS2SND_SetParam               5
#define PS2SND_GetParam               6
#define PS2SND_SetSwitch              7
#define PS2SND_GetSwitch              8
#define PS2SND_SetAddr                9
#define PS2SND_GetAddr                10
#define PS2SND_SetCoreAttr            11
#define PS2SND_GetCoreAttr            12
#define PS2SND_Note2Pitch             13
#define PS2SND_Pitch2Note             14
#define PS2SND_ProcBatch              15
#define PS2SND_ProcBatchEx            16
#define PS2SND_VoiceTrans             17
#define PS2SND_BlockTrans             18
#define PS2SND_VoiceTransStatus       19
#define PS2SND_BlockTransStatus       20
#define PS2SND_SetTransCallback       21
#define PS2SND_SetIRQCallback         22
#define PS2SND_SetEffectAttr          23
#define PS2SND_GetEffectAttr          24
#define PS2SND_ClearEffectWorkArea    25
#define PS2SND_SetTransIntrHandler    26
#define PS2SND_SetSpu2IntrHandler     27

#define PS2SND_StreamOpen             64
#define PS2SND_StreamClose            65
#define PS2SND_StreamPlay             66
#define PS2SND_StreamPause            67
#define PS2SND_StreamSetPosition      68
#define PS2SND_StreamGetPosition      69
#define PS2SND_StreamSetVolume        70
 

#define PS2SND_QueryMaxFreeMemSize    99 /* XXX: Hack until i can figure out how to do it right */

#ifdef _EE
int sndLoadSample(void *buf, u32 spuaddr, int size);
u32 sndQueryMaxFreeMemSize(void);
#endif

int sndStreamOpen(char *file, u32 voices, u32 flags, u32 bufaddr, u32 bufsize);
int sndStreamClose(void);
int sndStreamPlay(void);
int sndStreamPause(void);
int sndStreamSetPosition(int block);
int sndStreamGetPosition(void);
int sndStreamSetVolume(int left, int right);
u32 sndQueryMaxFreeMemSize();

#endif /* __PS2SND_H */
