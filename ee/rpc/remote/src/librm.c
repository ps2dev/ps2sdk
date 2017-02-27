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
 * RPC Interface for PS2 Remote Control Driver (RMMAN)
 */

#include <stdio.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <string.h>
#include "librm.h"

static SifRpcClientData_t rmmanif __attribute__((aligned(64)));
static struct rmRpcPacket buffer __attribute__((aligned(64)));
static int rmman_init = 0;

struct port_state
{
   int opened;
   struct rmEEData *rmData;
};

static struct port_state ports[2];

static struct rmEEData*
rmGetDmaStr(int port, int slot)

{
   struct rmEEData *pdata;

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
   if(rmman_init)
    {
      printf("RMMan Library already initialised\n");
      return 0;
    }

   rmmanif.server = NULL;

   do {
      if (SifBindRpc(&rmmanif, RMMAN_RPC_ID, 0) < 0) {
	 return -1;
      }
      nopdelay();
   } while(!rmmanif.server);

   buffer.cmd.command = RMMAN_RPCFUNC_INIT;

   if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
       return -1;

   ports[0].opened = 0;
   ports[0].rmData = NULL;
   ports[1].opened = 1;
   ports[1].rmData = NULL;

   rmman_init = 1;

   return buffer.cmd.result;
}

u32 RMMan_GetModuleVersion(void)

{
   buffer.cmd.command = RMMAN_RPCFUNC_VERSION;

   if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
       return 0;

   return buffer.cmd.result;
}

int RMMan_Open(int port, int slot, void *pData)

{
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

   buffer.cmd.command = RMMAN_RPCFUNC_OPEN;
   buffer.cmd.port = port;
   buffer.cmd.slot = slot;
   buffer.cmd.data = pData;

   if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
       return 0;

   ports[port].opened = 1;
   ports[port].rmData = pData;

   return buffer.cmd.result;
}

int RMMan_End(void)

{
   buffer.cmd.command = RMMAN_RPCFUNC_END;

   if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
       return 0;

   return buffer.cmd.result;
}

int RMMan_Close(int port, int slot)

{
   if((port < 0) || (port > 1) || (slot != 0))
   {
      printf("Error, port must be 0 or 1 and slot set to 0\n");
      return 0;
   }

   if(!ports[port].opened)
   {
      return 0;
   }

   buffer.cmd.command = RMMAN_RPCFUNC_CLOSE;
   buffer.cmd.port = port;
   buffer.cmd.slot = slot;

   if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
       return 0;

   return buffer.cmd.result;
}

void RMMan_Read(int port, int slot, struct remote_data *data)
{
   struct rmEEData *pdata;

   if((port < 0) || (port > 1) || (slot != 0))
   {
      printf("Error, port must be 0 or 1 and slot set to 0\n");
      return;
   }

   pdata = rmGetDmaStr(port, slot);

   memcpy(data, pdata->data, 8);
}
