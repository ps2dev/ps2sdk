/*
 *  hub.c - USB driver for PS2
 * 
 *  (c) 2004 TyRaNiD <tiraniddo at hotmail dot com>
 *
 *  This file contains the hub driver code.
 *
 *  See the file LICENSE included with this distribution for
 *  licensing terms.
 */

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "loadcore.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"
#include "intrman.h"
#include "stdio.h"
#include "sysclib.h"
#include "usbd.h"
#include "usbhw.h"
#include "sysmem.h"
#include "usbd_mod.h"

s32 hub_probe(s32 devid)
{
   return 0;
}

s32 hub_attach(s32 devid)
{
   return 0;
}

s32 hub_detach(s32 devid)
{
   return 0;
}

UsbDriver hub_ldd = {
   NULL, NULL,
   "Hub LDD",
   hub_probe,
   hub_attach,
   hub_detach,
   {0},
   0
};

void hub_register_driver(void)

{
   UsbRegisterDriver(&hub_ldd);
}
