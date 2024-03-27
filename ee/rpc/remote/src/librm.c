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
static int rmman_type = 0;

struct port_state
{
    int opened;
    struct rmEEData *rmData;
};

static struct port_state ports[2];

static struct rmEEData *rmGetDmaStr(int port, int slot)
{
    struct rmEEData *pdata;

    (void)slot;

    pdata = ports[port].rmData;
    SyncDCache(pdata, (u8 *)pdata + 256);

    if (pdata[0].frame < pdata[1].frame)
    {
        return &pdata[1];
    }
    else
    {
        return &pdata[0];
    }
}

int RMMan_Init(void)
{
    int i;

    if (rmman_type)
    {
        printf("RMMan Library already initialised\n");
        return 0;
    }

    {
        SifRpcClientData_t rpciftmp[3] __attribute__((aligned(64)));
        int rpc_ids[3] = {
            RMMANX_RPC_ID,
            RMMAN2_RPC_ID,
            RMMAN_RPC_ID,
        };

        rmmanif.server = NULL;
        for (i = 0; i < (int)(sizeof(rpc_ids)/sizeof(rpc_ids[0])); i += 1)
        {
            rpciftmp[i].server = NULL;
        }

        for (;;)
        {
            for (i = 0; i < (int)(sizeof(rpc_ids)/sizeof(rpc_ids[0])); i += 1)
            {
                if ((SifBindRpc(&rpciftmp[i], rpc_ids[i], 0) < 0))
                {
                    return -1;
                }
                if (rpciftmp[i].server != NULL)
                {
                    switch (rpc_ids[i])
                    {
                        case RMMANX_RPC_ID:
                        case RMMAN2_RPC_ID:
                        {
                            rmman_type = 2;
                            break;
                        }
                        case RMMAN_RPC_ID:
                        {
                            rmman_type = 1;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    memcpy(&rmmanif, &rpciftmp[i], sizeof(rmmanif));
                    break;
                }
            }
            if (rmmanif.server != NULL)
            {
                break;
            }
            nopdelay();
        }
    }

    if (rmman_type == 0)
    {
        return -1;
    }

    buffer.cmd.command = (rmman_type == 2) ? RMMAN2_RPCFUNC_INIT : RMMAN_RPCFUNC_INIT;

    if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        rmman_type = 0;
        return -1;
    }

    for (i = 0; i < (int)(sizeof(ports)/sizeof(ports[0])); i += 1)
    {
        ports[i].opened = 0;
        ports[i].rmData = NULL;
    }

    switch (rmman_type)
    {
        case 1:
            return buffer.cmd.u.cmd1.result;
        case 2:
            return buffer.cmd.u.cmd2.result;
        default:
            return 0;
    }
}

u32 RMMan_GetModuleVersion(void)
{
    buffer.cmd.command = (rmman_type == 2) ? RMMAN2_RPCFUNC_VERSION : RMMAN_RPCFUNC_VERSION;

    if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        return 0;
    }

    switch (rmman_type)
    {
        case 1:
            return buffer.cmd.u.cmd1.result;
        case 2:
            return buffer.cmd.u.cmd2.result;
        default:
            return 0;
    }
}

int RMMan_Open(int port, int slot, void *pData)
{
    if ((port < 0) || (port > 1) || (slot != 0))
    {
        printf("Error, port must be 0 or 1 and slot set to 0\n");
        return 0;
    }

    if ((u32)pData & 0x3F)
    {
        printf("Error, pData not aligned to 64byte boundary");
        return 0;
    }

    if (rmman_type != 1 && port != 0)
    {
        port = 0;
    }

    buffer.cmd.command = (rmman_type == 2) ? RMMAN2_RPCFUNC_OPEN : RMMAN_RPCFUNC_OPEN;
    switch (rmman_type)
    {
        case 1:
        {
            buffer.cmd.u.cmd1.port    = port;
            buffer.cmd.u.cmd1.slot    = slot;
            buffer.cmd.u.cmd1.data    = pData;
            break;
        }
        case 2:
        {
            buffer.cmd.u.cmd2.data    = pData;
            break;
        }
        default:
        {
            break;
        }
    }

    if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        return 0;
    }

    ports[port].opened = 1;
    ports[port].rmData = pData;

    switch (rmman_type)
    {
        case 1:
            return buffer.cmd.u.cmd1.result;
        case 2:
            return buffer.cmd.u.cmd2.result;
        default:
            return 0;
    }
}

int RMMan_End(void)
{
    buffer.cmd.command = (rmman_type == 2) ? RMMAN2_RPCFUNC_END : RMMAN_RPCFUNC_END;

    if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        return 0;
    }

    switch (rmman_type)
    {
        case 1:
            return buffer.cmd.u.cmd1.result;
        case 2:
            return buffer.cmd.u.cmd2.result;
        default:
            return 0;
    }
}

int RMMan_Close(int port, int slot)
{
    if ((port < 0) || (port > 1) || (slot != 0))
    {
        printf("Error, port must be 0 or 1 and slot set to 0\n");
        return 0;
    }

    if (rmman_type != 1 && port != 0)
    {
        port = 0;
    }

    if (!ports[port].opened)
    {
        return 0;
    }

    buffer.cmd.command = (rmman_type == 2) ? RMMAN2_RPCFUNC_CLOSE : RMMAN_RPCFUNC_CLOSE;
    switch (rmman_type)
    {
        case 1:
        {
            buffer.cmd.u.cmd1.port    = port;
            buffer.cmd.u.cmd1.slot    = slot;
            break;
        }
        default:
        {
            break;
        }
    }

    if (SifCallRpc(&rmmanif, 0, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        return 0;
    }

    switch (rmman_type)
    {
        case 1:
            return buffer.cmd.u.cmd1.result;
        case 2:
            return buffer.cmd.u.cmd2.result;
        default:
            return 0;
    }
}

void RMMan_Read(int port, int slot, struct remote_data *data)
{
    const struct rmEEData *pdata;

    if ((port < 0) || (port > 1) || (slot != 0))
    {
        printf("Error, port must be 0 or 1 and slot set to 0\n");
        return;
    }

    if (rmman_type != 1 && port != 0)
    {
        port = 0;
    }

    pdata = rmGetDmaStr(port, slot);

    if (rmman_type == 2)
    {
        int status;
        int button;
        status = RM_READY;
        button = 0;
        if (pdata->data[0] == 0x14)
        {
            status = RM_KEYPRESSED;
            button = pdata->data[1] | (pdata->data[2] << 8) | (pdata->data[3] << 16);
        }
        data->status = status;
        data->button = button;
    }
    else
    {
        memcpy(data, pdata->data, 8);
    }
}
