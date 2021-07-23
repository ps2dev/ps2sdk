/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * MPEG decoding functions.
 */

#ifndef __LIBMPEG_H__
#define __LIBMPEG_H__

#include <tamtypes.h>

#define MPEG_CHROMA_FORMAT_420 1
#define MPEG_CHROMA_FORMAT_422 2
#define MPEG_CHROMA_FORMAT_444 3

#define MPEG_PROFILE_422        133
#define MPEG_PROFILE_SIMPLE       5
#define MPEG_PROFILE_MAIN_        4
#define MPEG_PROFILE_SNR_SCALABLE 3
#define MPEG_PROFILE_SPT_SCALABLE 2
#define MPEG_PROFILE_HIGH         1

#define MPEG_LEVEL_MAIN      8
#define MPEG_LEVEL_LOW      12
#define MPEG_LEVEL_HIGH1440  6
#define MPEG_LEVEL_HIGH      4

#define MPEG_VIDEO_FORMAT_COMPONENT 0
#define MPEG_VIDEO_FORMAT_PAL       1
#define MPEG_VIDEO_FORMAT_NTSC      2
#define MPEG_VIDEO_FORMAT_SECAM     3
#define MPEG_VIDEO_FORMAT_MAC       4
#define MPEG_VIDEO_FORMAT_UNSPEC    5

typedef struct MPEGSequenceInfo {
	int m_Width;
	int m_Height;
	int m_FrameCnt;
	int m_Profile;
	int m_Level;
	int m_ChromaFmt;
	int m_VideoFmt;
	int m_fEOF;
	int m_MSPerFrame;
} MPEGSequenceInfo;

#ifdef __cplusplus
extern "C" {
#endif

void MPEG_Initialize (  int ( * ) ( void* ), void*, void* ( * ) ( void*, MPEGSequenceInfo* ), void*, s64*  );
void MPEG_Destroy    ( void );
extern int  ( *MPEG_Picture ) ( void*, s64* );

#ifdef __cplusplus
}
#endif

#endif /* __LIBMPEG_H__ */
