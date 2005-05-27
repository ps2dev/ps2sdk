/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# audsrv EE-side RPC code.
*/

#ifndef _AUDSRV_RPC_C_H_
#define _AUDSRV_RPC_C_H_

/**
 * \file audsrv_rpc.c.h
 * \author gawd (Gil Megidish)
 * \date 04-24-05
 */

/** intialization and destruction functions */
#define AUDSRV_INIT                 0x0000
#define AUDSRV_QUIT                 0x0001

/** intialization and destruction functions */
#define AUDSRV_INIT                 0x0000
#define AUDSRV_QUIT                 0x0001

/** audio control functions */
#define AUDSRV_FORMAT_OK            0x0002
#define AUDSRV_SET_FORMAT           0x0003
#define AUDSRV_PLAY_AUDIO           0x0004
#define AUDSRV_WAIT_AUDIO           0x0005
#define AUDSRV_STOP_AUDIO           0x0006
#define AUDSRV_SET_VOLUME           0x0007
#define AUDSRV_SET_THRESHOLD        0x0008

/** cdrom functions */
#define AUDSRV_PLAY_CD              0x0009
#define AUDSRV_STOP_CD              0x000a
#define AUDSRV_GET_CDPOS            0x000b
#define AUDSRV_GET_TRACKPOS         0x000c
#define AUDSRV_GET_NUMTRACKS        0x000d
#define AUDSRV_GET_TRACKOFFSET      0x000e
#define AUDSRV_PAUSE_CD             0x0011
#define AUDSRV_RESUME_CD            0x0012
#define AUDSRV_PLAY_SECTORS         0x0013
#define AUDSRV_GET_CD_STATUS        0x0014
#define AUDSRV_GET_CD_TYPE          0x0015
                                    
#define AUDSRV_FILLBUF_CALLBACK     0x0010
#define AUDSRV_CDDA_CALLBACK        0x0011

#endif
