/*
 * librm.c - RPC Interface for PS2 Remote Control Driver (RMMAN)
 *
 * (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include <stdio.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <string.h>
#include "librm.h"

static SifRpcClientData_t rmmanif __attribute__((aligned(64)));
static char buffer[128] __attribute__((aligned(64)));
static int rmman_init = 0;

#define RMMAN_BIND_RPC_ID 	0x80000C00
#define RMMAN_RPC_END     	1
#define RMMAN_RPC_INIT 	  	3
#define RMMAN_RPC_CLOSE   	4
#define RMMAN_RPC_OPEN    	5
#define RMMAN_RPC_GETMODULEVER 	7

struct rm_data
{
   /* Not sure what is in here yet */
   unsigned char data[32];
   unsigned int frame;
   unsigned char pad[92];
};

struct port_state
{
   int opened;
   struct rm_data *rmData;
};

static struct port_state ports[2];

static struct rm_data*
rmGetDmaStr(int port, int slot)

{
   struct rm_data *pdata;

   pdata = ports[port].rmData;    
   SyncDCache(pdata, (u8 *)pdata + 256);

   return pdata;

   if(pdata[0].frame < pdata[1].frame) {
      return &pdata[1];
   }
   else {
      return &pdata[0];
   }
}

int RMMan_Init(void)

{
   u32 *data = (u32 *) buffer;
   if(rmman_init)
    {
      printf("RMMan Library already initialised\n");
      return 0;
    }

   rmmanif.server = NULL;

   do {
      if (SifBindRpc(&rmmanif, RMMAN_BIND_RPC_ID, 0) < 0) {
	 return -1;
      }
      nopdelay();
   } while(!rmmanif.server);

   data[0] = RMMAN_RPC_INIT;

   if (SifCallRpc(&rmmanif, 0, 0, buffer, 128, buffer, 128, 0, 0) < 0)
       return -1;

   ports[0].opened = 0;
   ports[0].rmData = NULL;
   ports[1].opened = 1;
   ports[1].rmData = NULL;
   FlushCache(0);

   rmman_init = 1;

   return data[3];
}

u32 RMMan_GetModuleVersion(void)

{
   u32 *data = (u32 *) buffer;

   data[0] = RMMAN_RPC_GETMODULEVER;

   if (SifCallRpc(&rmmanif, 0, 0, buffer, 128, buffer, 128, 0, 0) < 0)
       return 0;

   FlushCache(0);

   return data[3];
}

int RMMan_Open(int port, int slot, void *pData)

{
   u32 *data = (u32 *) buffer;

   if((port < 0) || (port > 1) || (slot != 0))
   {
      printf("Error, port must be 0 or 1 and slot set to 0\n");
      return 0;
   }

   if((u32) pData & 0x3F)
   {
      printf("Error, pData not aligned to 64byte boundary");
      return 0;
   }

   data[0] = RMMAN_RPC_OPEN;
   data[1] = port;
   data[2] = slot;
   data[4] = (u32) pData;

   if (SifCallRpc(&rmmanif, 0, 0, buffer, 128, buffer, 128, 0, 0) < 0)
       return 0;

   FlushCache(0);

   ports[port].opened = 1;
   ports[port].rmData = pData;

   return data[3];
}

int RMMan_End(void)

{
   u32 *data = (u32 *) buffer;

   data[0] = RMMAN_RPC_END;

   if (SifCallRpc(&rmmanif, 0, 0, buffer, 128, buffer, 128, 0, 0) < 0)
       return 0;

   FlushCache(0);

   return data[3];
}

int RMMan_Close(int port, int slot)

{
   u32 *data = (u32 *) buffer;

   if((port < 0) || (port > 1) || (slot != 0))
   {
      printf("Error, port must be 0 or 1 and slot set to 0\n");
      return 0;
   }

   if(!ports[port].opened)
   {
      return 0;
   }

   data[0] = RMMAN_RPC_CLOSE;
   data[1] = port;
   data[2] = slot;

   if (SifCallRpc(&rmmanif, 0, 0, buffer, 128, buffer, 128, 0, 0) < 0)
       return 0;

   FlushCache(0);

   return data[3];
}

void RMMan_Read(int port, int slot, struct remote_data *data)
{
   struct rm_data *pdata;

   if((port < 0) || (port > 1) || (slot != 0))
   {
      printf("Error, port must be 0 or 1 and slot set to 0\n");
      return;
   }

   pdata = rmGetDmaStr(port, slot);

   memcpy(data, pdata->data, 8);
}
