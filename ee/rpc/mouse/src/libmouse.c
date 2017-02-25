/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * USB Mouse Driver for PS2
 */

#include <stdio.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <string.h>
#include "libmouse.h"

static SifRpcClientData_t mouseif __attribute__((aligned(64)));
static union {
	char buffer[128];
	u32 mode;
	u32 accel;
	u32 thres;
	struct mbounds {
		s32 minx, maxx;
		s32 miny, maxy;
	} bounds;
	struct {
		s32 x, y;
	} pos;
	u32 data;
	u32 time;
} buffer __attribute__((aligned(64)));
static int mouse_init = 0;

int PS2MouseInit(void)

{
  if(mouse_init)
    {
      printf("PS2Mouse Library already initialised\n");
      return 0;
    }

  mouseif.server = NULL;

  do {
    if (SifBindRpc(&mouseif, PS2MOUSE_BIND_RPC_ID, 0) < 0) {
      return -1;
    }
    nopdelay();
  } while(!mouseif.server);

  mouse_init = 1;

  return 1;
}

int PS2MouseRead(PS2MouseData *data)

{
  u8* uncached = UNCACHED_SEG(buffer.buffer);

  if(!data)
    {
      return -1;
    }

  if (SifCallRpc(&mouseif, PS2MOUSE_READ, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;


  memcpy(data, uncached, sizeof(PS2MouseData));
  //  printf("MouseRead %d %d %d %d\n", data->x, data->y, data->wheel, data->buttons);

  return 1;
}

int PS2MouseSetReadMode(u32 readMode)

{
  buffer.mode = readMode;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETREADMODE, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetReadMode()

{
  u32 *uncached = (u32 *) UNCACHED_SEG(&buffer.mode);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETREADMODE, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return 0xFF;

  return uncached[0];
}

int PS2MouseSetThres(u32 thres)

{
  buffer.thres = thres;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETTHRES, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetThres()

{
  u32* uncached = (u32 *) UNCACHED_SEG(&buffer.thres);
  if (SifCallRpc(&mouseif, PS2MOUSE_GETTHRES, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return 0;

  return uncached[0];
}

int PS2MouseSetAccel(float accel)

{
  u32 accel_fixed;

  if(accel < 0) return -1;

  accel_fixed = (u32) (accel * 65536.0);
  buffer.accel = accel_fixed;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETACCEL, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

float PS2MouseGetAccel()

{
  u32* uncached = (u32 *) UNCACHED_SEG(&buffer.accel);
  u32 accel_fixed;

  if (SifCallRpc(&mouseif, PS2MOUSE_GETACCEL, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  accel_fixed = uncached[0];

  return ((float) accel_fixed) / 65536.0 ;
}

int PS2MouseSetBoundary(int minx, int maxx, int miny, int maxy)

{
  buffer.bounds.minx = minx;
  buffer.bounds.maxx = maxx;
  buffer.bounds.miny = miny;
  buffer.bounds.maxy = maxy;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETBOUNDARY, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

int PS2MouseGetBoundary(int *minx, int *maxx, int *miny, int *maxy)

{
  struct mbounds* uncached = (struct mbounds *) UNCACHED_SEG(&buffer.bounds);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETBOUNDARY, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  *minx = uncached->minx;
  *maxx = uncached->maxx;
  *miny = uncached->miny;
  *maxy = uncached->maxy;

  return 1;
}

int PS2MouseSetPosition(int x, int y)

{
  buffer.pos.x = x;
  buffer.pos.y = y;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETPOSITION, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

int PS2MouseReset()

{
  if (SifCallRpc(&mouseif, PS2MOUSE_RESET, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

u32 PS2MouseEnum()

{
  u32* uncached = (u32 *) UNCACHED_SEG(&buffer.data);

  if (SifCallRpc(&mouseif, PS2MOUSE_ENUM, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return uncached[0];
}

u32 PS2MouseGetVersion()

{
  u32* uncached = (u32 *) UNCACHED_SEG(&buffer.data);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETVERSION, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return uncached[0];
}

int PS2MouseSetDblClickTime(u32 msec)

{
  buffer.time = msec;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETDBLCLICKTIME, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetDblClickTIme()

{
  u32* uncached = (u32 *) UNCACHED_SEG(&buffer.time);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETDBLCLICKTIME, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    return -1;

  return uncached[0];
}

