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
# USB driver for PS2
# This file contains the exported interface and usbd functionality
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

int hcd_alloc_data(void);
int hcd_setup(void);

static u8 *mem_pool;

static dev_data_t *devs;
static dev_data_t *free_devs;
static dev_data_t *used_devs;

extern u32 g_num_devices;
extern u32 g_conf_size;
extern int g_lock_sema;

/* Define a root ldd, this is never used as a target */
/* Just makes managing the list easier :) */
UsbDriver g_root_ldd = { 
    NULL, NULL,
    "Root",
    NULL,
    NULL,
    NULL,
    { 0 },
    0
};

static void usbd_dump_dev(dev_data_t *dev)

{
   printf("Device %p, devno %d, dev_ldd %p, next %p, prev %p, desc %p\n", 
	 dev, dev->devno, dev->dev_ldd, dev->next_dev, dev->prev_dev, dev->static_desc);
}

void usbd_debug_print_devs(void)

{
   dev_data_t *dev;

   dev = used_devs;

   printf("used_devs %p, free_devs %p\n", used_devs, free_devs);

   printf("Used devices\n");
   while(dev)
   {
      usbd_dump_dev(dev);
      dev = dev->next_dev;
   }

   printf("Free devices\n");
   dev = free_devs;
   while(dev)
   {
      usbd_dump_dev(dev);
      dev = dev->next_dev;
   }
}

/* Returns the device structure */
dev_data_t *usbd_alloc_dev(void)

{
   dev_data_t *dev;

   if(free_devs == NULL)
   {
      M_PRINTF("No free devices left\n");
      return NULL;
   }

   LOCK(g_lock_sema, NULL);

   /* Get next device and unlink from free chain */
   dev = free_devs;
   if(dev->next_dev != NULL)
   {
      dev->next_dev->prev_dev = NULL;
   }
   
   /* Free list will point to NULL if this was the last device */
   free_devs = dev->next_dev;

   if(used_devs)
   {
      used_devs->prev_dev = dev;
   }

   dev->next_dev = used_devs;
   dev->prev_dev = NULL;
   used_devs = dev;

   UNLOCK(g_lock_sema);

   return dev;
}

/* Connect the device, starts off a callback chain to get the static data etc. */
/* Will return almost immediately irrespective of whether the connect was successful */
int usbd_connect_dev(dev_data_t *dev)

{
   if(dev == NULL)
   {
      M_PRINTF("Cannot connect a NULL device\n");
      return -1;
   }

   LOCK(g_lock_sema, -1);

   /* Setup device, read in static descriptors etc. */
   /* Run through LDD chain and call in sequence */

   UNLOCK(g_lock_sema);

   return 0;
}

int usbd_free_dev(dev_data_t *dev)

{
   if(dev == NULL)
   {
      M_PRINTF("Cannot free a NULL device\n");
      return -1;
   }

   LOCK(g_lock_sema, -1);

   if(dev->next_dev)
   {
      dev->next_dev->prev_dev = dev->prev_dev;
   }

   if(dev->prev_dev == NULL)
   {
      used_devs = dev->next_dev;
   }
   else
   {
      dev->prev_dev->next_dev = dev->next_dev;
   }

   /* Relink at head of free list */
   dev->next_dev = free_devs;
   if(dev->next_dev)
   {
      dev->next_dev->prev_dev = dev;
   }

   dev->prev_dev = NULL;
   free_devs = dev;

   UNLOCK(g_lock_sema);

   return 0;
}

void *usbd_alloc_sysmem(u32 size)

{
   return AllocSysMemory(ALLOC_FIRST, size, NULL);
}

int usbd_alloc_data(void)

{
   int data_size;
   int dev_loop;
   u8 *curr_pool;

   g_num_devices += 1; /* Take into account the root hub */

   data_size = g_num_devices * sizeof(dev_data_t) + g_conf_size * g_num_devices;
   mem_pool = usbd_alloc_sysmem(data_size); 
   if(mem_pool == NULL)
   {
      return -1;
   }

   memset(mem_pool, 0, data_size);
   devs = (dev_data_t *) mem_pool;
   curr_pool = &mem_pool[g_num_devices * sizeof(dev_data_t)];

   for(dev_loop = 0; dev_loop < g_num_devices; dev_loop++)
   {
      devs[dev_loop].devno = dev_loop; /* Statically allocate devices */
      devs[dev_loop].next_dev = &devs[dev_loop + 1];
      devs[dev_loop].prev_dev = &devs[dev_loop - 1];
      devs[dev_loop].static_desc = curr_pool;
      curr_pool += g_conf_size;
   }

   /* Fix linked list */
   devs[0].prev_dev = NULL;
   devs[g_num_devices - 1].next_dev = NULL;

   used_devs = NULL;
   free_devs = &devs[0];

   return 0;
}

void usbd_dealloc_data(void)

{
   FreeSysMemory(mem_pool);
}

int usbd_init(void)

{
   if(hcd_alloc_data() < 0)
   {
      M_PRINTF("Failed to allocate hcd data\n");
      return -1;
   }

   if(usbd_alloc_data() < 0)
   {
      M_PRINTF("Failed to allocate usbd data\n");
   }

   if(hcd_setup() < 0)
   {
      return -1;
   }

   return 0;
}

s32 UsbChangeThreadPriority(s32 p1, s32 p2)

{
   return 0;
}

s32 UsbCloseEndpoint(s32 pipeid)

{
   return 0;
}

int UsbGetDeviceLocation(s32 devid, u8 *locs)

{
   return 0;
}

void *UsbGetDevicePrivateData(s32 devid)

{
   void *data = NULL;

   if(devid < g_num_devices)
   {
      LOCK(g_lock_sema, NULL);
      data = devs[devid].priv_data;
      UNLOCK(g_lock_sema);
   }

   return data;
}

s32 UsbGetReportDescriptor(s32 devid, s32 config, s32 interface, void **desc)

{
   return 0;
}

s32 UsbOpenEndpoint(s32 devid, UsbEndpointDescriptor *desc)

{
   return 0;
} 

s32 UsbOpenEndpointAligned(s32 devid, UsbEndpointDescriptor *desc)

{
   return 0;
}

s32 UsbRegisterDriver(UsbDriver *ldd)

{
   UsbDriver *curr_ldd;
   u32 curr_gp = 0;

   /* Grab $gp register */
   asm __volatile__ ( 
	" add %0, $0, $28\n"
        : "=r"(curr_gp)
   ); 
   printf("Register Ldd gp = %08X\n", curr_gp);

   if((ldd == NULL) || (ldd->next) || (ldd->prev) || (ldd->name == NULL))
   {
      return USBD_ERROR_LDD_INVALID;
   }

   LOCK(g_lock_sema, USBD_ERROR_UNLOCKED);
   
   curr_ldd = &g_root_ldd;

   while(curr_ldd->next)
   {
      curr_ldd = curr_ldd->next; 
   }

   /* Found end of chain */
   curr_ldd->next = ldd;
   ldd->prev = curr_ldd;
   ldd->gp = curr_gp;

   UNLOCK(g_lock_sema);
   
   return 0;
}

s32 UsbGetStaticDescriptor(s32 devid, void *data, u8 type)

{
   return 0;
}

s32 UsbSetDevicePrivateData(s32 devid, void *data)

{
   if(devid < g_num_devices)
   {
      LOCK(g_lock_sema, USBD_ERROR_UNLOCKED);
      devs[devid].priv_data = data;
      UNLOCK(g_lock_sema);
   }
   else
   {
      return -1;
   }

   return 0;
}

s32 UsbTransfer(s32 pipe, void *data, s32 len, void *option, UsbTransferDoneCallBack func, void *cb_arg)

{
   return 0;
}

s32 UsbUnregisterDriver(UsbDriver *ldd)

{
   if(ldd == NULL)
   {
      return USBD_ERROR_LDD_INVALID;
   }

   /* Flush out all waiting transfers for this ldd */

   /* Grab a sema before modifying the ldd list */
   LOCK(g_lock_sema, USBD_ERROR_UNLOCKED);

   /* Unlink ldd from chain */
   if(ldd->prev)
   {
      ldd->prev->prev = ldd->next;
   }

   if(ldd->next)
   {
      ldd->next->prev = ldd->prev;
   }

   UNLOCK(g_lock_sema);

   return 0;
}

/* No idea what these two do, are exported from the irx I had though */
s32 UsbCall14(s32 param)

{
   return 0;
}

s32 UsbCall15(void)

{
   return 0;
}
