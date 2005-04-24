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
#define AUDSRV_INIT          0x0000
#define AUDSRV_QUIT          0x0001

/** audio control functions */
#define AUDSRV_FORMAT_OK     0x0002
#define AUDSRV_SET_FORMAT    0x0003
#define AUDSRV_PLAY_AUDIO    0x0004
#define AUDSRV_WAIT_AUDIO    0x0005
#define AUDSRV_STOP_AUDIO    0x0006
#define AUDSRV_SET_VOLUME    0x0007

/** cdrom functions */
#define AUDSRV_PLAY_CD       0x0008
#define AUDSRV_STOP_CD       0x0009
#define AUDSRV_SET_CDVOL     0x000a

#endif
