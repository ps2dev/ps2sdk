/*
 * libmouse.c - USB Mouse Driver for PS2
 *
 * (c) 2003 TyRaNiD <tiraniddo@hotmail.com>
 *
 * This module handles the setup and manipulation of USB mouse devices
 * on the PS2
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <string.h>
#include "libmouse.h"

static SifRpcClientData_t mouseif __attribute__((aligned(64)));
static char buffer[128] __attribute__((aligned(64)));
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
  u8* uncached = UNCACHED_SEG(buffer);

  if(!data)
    {
      return -1;
    }

  if (SifCallRpc(&mouseif, PS2MOUSE_READ, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;


  memcpy(data, uncached, sizeof(PS2MouseData));
  //  printf("MouseRead %d %d %d %d\n", data->x, data->y, data->wheel, data->buttons);
  
  return 1;
}

int PS2MouseSetReadMode(u32 readMode)

{
  *((u32 *) buffer) = readMode;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETREADMODE, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetReadMode()

{
  u32 *uncached = (u32 *) UNCACHED_SEG(buffer);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETREADMODE, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return 0xFF;

  return uncached[0];
}

int PS2MouseSetThres(u32 thres)

{
  *((u32 *) buffer) = thres;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETTHRES, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetThres()

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);
  if (SifCallRpc(&mouseif, PS2MOUSE_GETTHRES, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return 0;

  return uncached[0];
}

int PS2MouseSetAccel(float accel)

{
  u32 accel_fixed;

  if(accel < 0) return -1;

  accel_fixed = (u32) (accel * 65536.0);
  *((u32 *) buffer) = accel_fixed;
  if (SifCallRpc(&mouseif, PS2MOUSE_SETACCEL, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

float PS2MouseGetAccel()

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);
  u32 accel_fixed;

  if (SifCallRpc(&mouseif, PS2MOUSE_GETACCEL, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  accel_fixed = uncached[0];

  return ((float) accel_fixed) / 65536.0 ;
}

int PS2MouseSetBoundary(int minx, int maxx, int miny, int maxy)

{
  s32* data = (s32 *) buffer;

  data[0] = minx;
  data[1] = maxx;
  data[2] = miny;
  data[3] = maxy;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETBOUNDARY, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

int PS2MouseGetBoundary(int *minx, int *maxx, int *miny, int *maxy)

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETBOUNDARY, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  *minx = uncached[0];
  *maxx = uncached[1];
  *miny = uncached[2];
  *maxy = uncached[3];

  return 1;
}

int PS2MouseSetPosition(int x, int y)

{
  s32* data = (s32 *) buffer;
  
  data[0] = x;
  data[1] = y;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETPOSITION, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

int PS2MouseReset()

{
  if (SifCallRpc(&mouseif, PS2MOUSE_RESET, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

u32 PS2MouseEnum()

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);

  if (SifCallRpc(&mouseif, PS2MOUSE_ENUM, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return uncached[0];
}

u32 PS2MouseGetVersion()

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETVERSION, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return uncached[0];
}

int PS2MouseSetDblClickTime(u32 msec)

{
  u32 *data = (u32 *) buffer;

  data[0] = msec;

  if (SifCallRpc(&mouseif, PS2MOUSE_SETDBLCLICKTIME, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return 1;
}

u32 PS2MouseGetDblClickTIme()

{
  u32* uncached = (u32 *) UNCACHED_SEG(buffer);

  if (SifCallRpc(&mouseif, PS2MOUSE_GETDBLCLICKTIME, 0, buffer, 128, buffer, 128, 0, 0) < 0)
    return -1;

  return uncached[0];
}

