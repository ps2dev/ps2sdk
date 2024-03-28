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
 * Common definitions for librm on the EE and IOP
 */

#ifndef __LIBRM_COMMON_H__
#define __LIBRM_COMMON_H__

#include <tamtypes.h>

struct rmRpcPacket
{
    union
    {
        struct
        {
            s32 command;
            union
            {
                struct
                {
                    s32 port, slot;
                    s32 result;
                    void *data;
                } cmd1;
                struct
                {
                    s32 result;
                    void *data;
                } cmd2;
            } u;
        } cmd;
        u8 buffer[128];
    };
};

struct rmEEData
{
    u8 data[32];
    u32 frame;
    u32 unused;
    u32 connected;
    u32 state;
};

struct rmEEData2
{
    u8 data[32];
    u32 connected;
    u32 state;
    u32 unused;
    u32 frame;
};

#define RMMAN_RPC_ID 0x80000C00
#define RMMAN2_RPC_ID 0x80000C01
#define RMMANX_RPC_ID 0x80000C02

enum RMMAN_RPCFUNC {
    RMMAN_RPCFUNC_END  = 1,
    RMMAN_RPCFUNC_INIT = 3,
    RMMAN_RPCFUNC_CLOSE,
    RMMAN_RPCFUNC_OPEN,
    RMMAN_RPCFUNC_VERSION = 7
};

enum RMMAN2_RPCFUNC {
    RMMAN2_RPCFUNC_END  = 1,
    RMMAN2_RPCFUNC_INIT,
    RMMAN2_RPCFUNC_CLOSE,
    RMMAN2_RPCFUNC_OPEN,
    RMMAN2_RPCFUNC_VERSION,
    RMMAN2_RPCFUNC_REMOTE2_6,
};

enum RM_RSTATE {
    RM_RSTATE_COMPLETE = 0,
    RM_RSTATE_FAILED,
    RM_RSTATE_BUSY
};

enum RM_STATE {
    RM_STATE_DISCONN = 0,
    RM_STATE_FINDRM,
    RM_STATE_EXECCMD,
    RM_STATE_STABLE
};

#endif /* _LIBRM_COMMON_H_ */
