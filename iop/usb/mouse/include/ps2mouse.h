/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB Mouse Driver for PS2
*/

#ifndef __PS2MOUSE_H__
#define __PS2MOUSE_H__

typedef struct _mouse_data 

{
  s32 x, y;
  s32 wheel;
  u32 buttons;
} mouse_data;

#define PS2MOUSE_READMODE_DIFF 0
#define PS2MOUSE_READMODE_ABS 1

/* Define mouse button constants */

#define PS2MOUSE_BTN1    1
#define PS2MOUSE_BTN2    2
#define PS2MOUSE_BTN3    4
#define PS2MOUSE_BTN1DBL (PS2MOUSE_BTN1 << 8)
#define PS2MOUSE_BTN2DBL (PS2MOUSE_BTN2 << 8)
#define PS2MOUSE_BTN3DBL (PS2MOUSE_BTN3 << 8)

/* RPC Defines */

#define PS2MOUSE_BIND_RPC_ID 0x500C001
#define PS2MOUSE_READ 0x1
#define PS2MOUSE_SETREADMODE 0x2
#define PS2MOUSE_GETREADMODE 0x3
#define PS2MOUSE_SETTHRES 0x4
#define PS2MOUSE_GETTHRES 0x5
#define PS2MOUSE_SETACCEL 0x6
#define PS2MOUSE_GETACCEL 0x7
#define PS2MOUSE_SETBOUNDARY 0x8
#define PS2MOUSE_GETBOUNDARY 0x9
#define PS2MOUSE_SETPOSITION 0xA
#define PS2MOUSE_RESET 0xB
#define PS2MOUSE_ENUM 0xC
#define PS2MOUSE_SETDBLCLICKTIME 0xD
#define PS2MOUSE_GETDBLCLICKTIME 0xE
#define PS2MOUSE_GETVERSION 0x20

#endif
