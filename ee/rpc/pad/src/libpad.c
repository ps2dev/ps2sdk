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
 * Pad library functions
 * Quite easy rev engineered from util demos..
 * Find any bugs? Mail me: pukko@home.se
 */

#include <tamtypes.h>
#include <kernel.h>
#include <stdio.h>
#include <string.h>
#include <sifrpc.h>
#include <sifcmd.h>
#include "libpad.h"

/*
 * Defines
 */

#define PAD_BIND_RPC_ID1_NEW 0x80000100
#define PAD_BIND_RPC_ID2_NEW 0x80000101

#define PAD_BIND_RPC_ID1_OLD 0x8000010f
#define PAD_BIND_RPC_ID2_OLD 0x8000011f

#define PAD_RPCCMD_OPEN_NEW         0x01
#define PAD_RPCCMD_SET_MMODE_NEW    0x06
#define PAD_RPCCMD_SET_ACTDIR_NEW   0x07
#define PAD_RPCCMD_SET_ACTALIGN_NEW 0x08
#define PAD_RPCCMD_GET_BTNMASK_NEW  0x09
#define PAD_RPCCMD_SET_BTNINFO_NEW  0x0A
#define PAD_RPCCMD_SET_VREF_NEW     0x0B
#define PAD_RPCCMD_GET_PORTMAX_NEW  0x0C
#define PAD_RPCCMD_GET_SLOTMAX_NEW  0x0D
#define PAD_RPCCMD_CLOSE_NEW        0x0E
#define PAD_RPCCMD_END_NEW          0x0F
#define PAD_RPCCMD_INIT             0x10
#define PAD_RPCCMD_GET_MODVER       0x12

#define PAD_RPCCMD_OPEN_OLD         0x80000100
#define PAD_RPCCMD_INFO_ACT         0x80000102
#define PAD_RPCCMD_INFO_COMB_OLD    0x80000103
#define PAD_RPCCMD_INFO_MODE        0x80000104
#define PAD_RPCCMD_SET_MMODE_OLD    0x80000105
#define PAD_RPCCMD_SET_ACTDIR_OLD   0x80000106
#define PAD_RPCCMD_SET_ACTALIGN_OLD 0x80000107
#define PAD_RPCCMD_GET_BTNMASK_OLD  0x80000108
#define PAD_RPCCMD_SET_BTNINFO_OLD  0x80000109
#define PAD_RPCCMD_SET_VREF_OLD     0x8000010a
#define PAD_RPCCMD_GET_PORTMAX_OLD  0x8000010b
#define PAD_RPCCMD_GET_SLOTMAX_OLD  0x8000010c
#define PAD_RPCCMD_CLOSE_OLD        0x8000010d
#define PAD_RPCCMD_END_OLD          0x8000010e

/*
 * Types
 */

struct pad_data_new
{
    u8 data[32];
    u32 actDirData[2];
    u32 actAlignData[2];
    u8 actData[8][4];
    u16 modeTable[4];
    u32 frame;
    u32 findPadRetries;
    u32 length;
    u8 modeConfig;
    u8 modeCurId;
    u8 model;
    u8 buttonDataReady;
    u8 nrOfModes;
    u8 modeCurOffs;
    u8 nrOfActuators;
    u8 numActComb;
    u8 val_c6;
    u8 mode;
    u8 lock;
    u8 actDirSize;
    u8 state;
    u8 reqState;
    u8 currentTask;
    u8 runTask;
    u8 stat70bit;
    u8 padding[11];
};

// rom0:padman has only 64 byte of pad data
struct pad_data_old
{
    u32 frame;
    u8 state;
    u8 reqState;
    u8 ok;
    u8 unkn7;
    u8 data[32];
    u32 length;
    u8 request;
    /** CTP=1/no config; CTP=2/config, acts... */
    u8 CTP;
    /** 1, 2 or 3 */
    u8 model;
    /** the data in the buffer is already corrected */
    u8 correction;
    u8 errorCount;
    u8 unk49[15];
};

union pad_data_u
{
    struct pad_data_old oldPadData[2];
    struct pad_data_new newPadData[2];
};

struct pad_state
{
    int open;
    unsigned int port;
    unsigned int slot;
    union pad_data_u *padData;
    unsigned char *padBuf;
};

struct open_slot
{
    u32 frame;
    u32 openSlots[2];
    u8 padding[116];
};

extern int _iop_reboot_count;
/*
 * Pad variables etc.
 */

static const char *padStateString[] = {
    "DISCONNECT",
    "FINDPAD",
    "FINDCTP1",
    "",
    "",
    "EXECCMD",
    "STABLE",
    "ERROR",
};
static const char *padReqStateString[] = {
    "COMPLETE",
    "FAILED",
    "BUSY",
};

static int padInitialised = 0;

// pad rpc call
static SifRpcClientData_t padsif[2] __attribute__((aligned(64)));
static union {
	s32 command;
    struct {
        s32 command;
        s32 unused[3];
        void *statBuf;
    } padInitArgs;
	struct {
		s32 unknown[3];
		s32 result;
		s32 unknown2;
		void *padBuf;
	} padOpenResult;
	struct {
		s32 unknown[3];
		s32 result;
	} padResult;
	struct {
		s32 command;
		s32 port, slot;
		s32 unknown;
		void *padArea;
	} padOpenArgs;
	struct {
		s32 command;
		s32 port, slot;
		s32 unknown;
		s32 mode;
	} padCloseArgs;
	struct {
		s32 command;
		s32 port;
	} padSlotMaxArgs;
	struct {
		s32 command;
		s32 port, slot;
		s32 infoMode;
		s32 index;
	} padInfoModeArgs;
	struct {
		s32 command;
		s32 port, slot;
		s32 mode;
		s32 lock;
	} padSetMainModeArgs;
	struct {
		s32 unknown[5];
		s32 result;
	} padModeResult;
	struct {
		s32 command;
		s32 port, slot;
	} padGetButtonMaskArgs;
	struct {
		s32 command;
		s32 port, slot;
		s32 buttonInfo;
	} padSetButtonInfoArgs;
	struct {
		s32 unknown[4];
		s32 result;
	} padSetButtonInfoResult;
	struct {
		s32 command;
		s32 port, slot;
		s32 actuator;
		s32 act_cmd;
	} padInfoActArgs;
	struct {
		s32 command;
		s32 port, slot;
		s8 align[6];
	} padActDirAlignArgs;
	char buffer[128];
}
buffer __attribute__((aligned(64)));

/** Port state data */
static struct open_slot openSlot[2] __attribute__((aligned(64)));
static struct pad_state PadState[2][8];


/*
 * Local functions
 */

/** Common helper */
static struct pad_data_new*
padGetDmaStrNew(int port, int slot)
{
    struct pad_data_new *pdata;

    pdata = PadState[port][slot].padData->newPadData;
    SyncDCache(pdata, (u8 *)pdata + 256);

    return (pdata[0].frame < pdata[1].frame) ? &pdata[1] : &pdata[0];
}

static struct pad_data_old*
padGetDmaStrOld(int port, int slot)
{
    struct pad_data_old *pdata;

    pdata = PadState[port][slot].padData->oldPadData;
    SyncDCache(pdata, (u8 *)pdata + 256);

    return (pdata[0].frame < pdata[1].frame) ? &pdata[1] : &pdata[0];
}

/**
 * Returns the data for pad (opened) status.
 * This seems to have been removed from the official SDKs, very early during the PS2's lifetime.
 */
static struct open_slot*
padGetConnDmaStr(void)
{
   SyncDCache(openSlot, (u8*)openSlot + sizeof(openSlot));

   return (openSlot[0].frame < openSlot[1].frame) ? &openSlot[1] : &openSlot[0];
}

/*
 * Global functions
 */

/*
 * Functions not implemented here
 * padGetFrameCount() <- dunno if it's useful for someone..
 * padInfoComb() <- see above
 * padSetVrefParam() <- dunno
 */

int
padInit(int mode)
{
    // Version check isn't used by default
    // int ver;
    static int _rb_count = 0;
    int rpc_init_next;

    if (_rb_count != _iop_reboot_count)
    {
        _rb_count = _iop_reboot_count;
        padInitialised = 0;
    }

    if (padInitialised)
        return 0;

    padInitialised = 0xFFFFFFFF;
    rpc_init_next = 0;

    padsif[0].server = NULL;
    padsif[1].server = NULL;

    {
        unsigned int i;
        SifRpcClientData_t rpciftmp[2] __attribute__((aligned(64)));
        static const int rpc_ids[] = {
            PAD_BIND_RPC_ID1_NEW,
            PAD_BIND_RPC_ID1_OLD,
        };

        padsif[0].server = NULL;
        for (i = 0; i < (sizeof(rpc_ids)/sizeof(rpc_ids[0])); i += 1)
            rpciftmp[i].server = NULL;

        for (;;)
        {
            for (i = 0; i < (sizeof(rpc_ids)/sizeof(rpc_ids[0])); i += 1)
            {
                if ((sceSifBindRpc(&rpciftmp[i], rpc_ids[i], 0) < 0))
                    return -1;
                if (rpciftmp[i].server != NULL)
                {
                    switch (rpc_ids[i])
                    {
                    case PAD_BIND_RPC_ID1_NEW:
                        {
                            padInitialised = 2;
                            rpc_init_next = PAD_BIND_RPC_ID2_NEW;
                            break;
                        }
                    case PAD_BIND_RPC_ID1_OLD:
                        {
                            padInitialised = 1;
                            rpc_init_next = PAD_BIND_RPC_ID2_OLD;
                            break;
                        }
                    default:
                        break;
                    }
                    memcpy(&padsif[0], &rpciftmp[i], sizeof(padsif[0]));
                    break;
                }
            }
            if (padsif[0].server != NULL)
                break;
            nopdelay();
        }
    }

    if (!rpc_init_next)
        return -1;

    while (!padsif[1].server)
    {
        if (sceSifBindRpc(&padsif[1], rpc_init_next, 0) < 0)
            return -3;
        nopdelay();
    }

    // If you require a special version of the padman, check for that here (uncomment)
    // ver = padGetModVersion();

    //At v1.3.4, this used to return 1 (and padPortInit is not used). But we are interested in the better design from release 1.4.
    return padPortInit(mode);
}

int padPortInit(int mode)
{
    unsigned int i;
    unsigned int j;

    (void)mode;

    for (i = 0; i < (sizeof(PadState)/sizeof(PadState[0])); i += 1)
    {
        for (j = 0; j < (sizeof(PadState[0])/sizeof(PadState[0][0])); j += 1)
        {
            PadState[i][j].open = 0;
            PadState[i][j].port = 0;
            PadState[i][j].slot = 0;
        }
    }

    switch (padInitialised)
    {
    case 1:
        {
#if 0
            int ret;
            //Unofficial: libpad EE client from v1.3.4 has this RPC function implemented, but is not implemented within its PADMAN module.
            buffer.command = PAD_RPCCMD_OPEN_OLD;
            ret = sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL);
#endif
            return 1;
        }
    case 2:
        {
            int ret;

            buffer.command = PAD_RPCCMD_INIT;
            buffer.padInitArgs.statBuf = openSlot;
            ret = sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL);

            return ret >= 0 ? buffer.padResult.result : 0;
        }
    default:
        return 1;
    }
}

int
padEnd(void)
{
    int ret;

    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_END_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_END_NEW;
            break;
        }
    default:
        return -1;
    }

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return -1;

    ret = buffer.padResult.result;
    if (ret == 1)
        padInitialised = 0;

    return ret;
}

int
padPortOpen(int port, int slot, void *padArea)
{
    union pad_data_u *dma_buf = (union pad_data_u *)padArea;

    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (padInitialised)
    {
    case 1:
        {
            // Check 16 byte alignment
            if ((u32)padArea & 0xf)
            {
                printf("Address is not 16-byte aligned.\n");
                return 0;
            }
            break;
        }
    case 2:
        {
            // Check 64 byte alignment
            if ((u32)padArea & 0x3f)
            {
                printf("Address is not 64-byte aligned.\n");
                return 0;
            }
            break;
        }
    default:
        return 0;
    }

    
    // Pad data is double buffered
    switch (padInitialised)
    {
    case 1:
        {
            unsigned int i;

            for (i = 0; i < (sizeof(dma_buf->oldPadData)/sizeof(dma_buf->oldPadData[0])); i += 1)
            {
                struct pad_data_old *pdata;

                pdata = &dma_buf->oldPadData[i];
                memset(pdata->data, 0xff, sizeof(pdata->data)); // 'Clear' data area
                pdata->frame = 0;
                pdata->length = 0;
                pdata->state = PAD_STATE_EXECCMD;
                pdata->reqState = PAD_RSTAT_BUSY;
                pdata->length = 0;
                pdata->ok = 0;
            }
            break;
        }
    case 2:
        {
            unsigned int i;

            for (i = 0; i < (sizeof(dma_buf->newPadData)/sizeof(dma_buf->newPadData[0])); i += 1)
            {
                struct pad_data_new *pdata;

                pdata = &dma_buf->newPadData[i];
                memset(pdata->data, 0xff, sizeof(pdata->data)); // 'Clear' data area
                pdata->frame = 0;
                pdata->length = 0;
                pdata->state = PAD_STATE_EXECCMD;
                pdata->reqState = PAD_RSTAT_BUSY;
                pdata->length = 0;
                pdata->currentTask = 0;
                pdata->buttonDataReady = 0; // Should be cleared in newer padman
            }
            break;
        }
    default:
        return 0;
    }

    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_OPEN_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_OPEN_NEW;
            break;
        }
    default:
        return 0;
    }
	buffer.padOpenArgs.port = port;
    buffer.padOpenArgs.slot = slot;
    buffer.padOpenArgs.padArea = padArea;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    PadState[port][slot].open = padInitialised;
    PadState[port][slot].padData = padArea;
    PadState[port][slot].padBuf = buffer.padOpenResult.padBuf;

    return buffer.padOpenResult.result;
}

int
padPortClose(int port, int slot)
{
    int ret;

    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return -1;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return -1;

    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_CLOSE_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_CLOSE_NEW;
            break;
        }
    default:
        return -1;
    }

	buffer.padCloseArgs.port = port;
	buffer.padCloseArgs.slot = slot;
	buffer.padCloseArgs.mode = 1;

    ret = sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL);

    if (ret < 0)
        return ret;
    PadState[port][slot].open = 0;
    return buffer.padResult.result;
}

unsigned char
padRead(int port, int slot, struct padButtonStatus *data)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        {
            struct pad_data_old *pdata;

            pdata = padGetDmaStrOld(port, slot);

            memcpy(data, pdata->data, pdata->length);
            return pdata->length;
        }
    case 2:
        {
            struct pad_data_new *pdata;

            pdata = padGetDmaStrNew(port, slot);

            memcpy(data, pdata->data, pdata->length);
            return pdata->length;
        }
    default:
        return 0;
    }
}

int
padGetState(int port, int slot)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        {
            const struct pad_data_old *pdata;
            unsigned char state;

            pdata = padGetDmaStrOld(port, slot);
            state = pdata->state;

            if (state == PAD_STATE_STABLE && padGetReqState(port, slot) == PAD_RSTAT_BUSY) // Ok
                return PAD_STATE_EXECCMD;
            return state;
        }
    case 2:
        {
            const struct pad_data_new *pdata;
            unsigned char state;

            pdata = padGetDmaStrNew(port, slot);
            state = pdata->state;

            if (state == PAD_STATE_ERROR && pdata->findPadRetries)
                return PAD_STATE_FINDPAD;

            if (state == PAD_STATE_STABLE && padGetReqState(port, slot) == PAD_RSTAT_BUSY) // Ok
                return PAD_STATE_EXECCMD;
            return state;
        }
    default:
        return 0;
    }
}

unsigned char
padGetReqState(int port, int slot)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        return padGetDmaStrOld(port, slot)->reqState;
    case 2:
        return padGetDmaStrNew(port, slot)->reqState;
    default:
        return 0;
    }
}

int
padSetReqState(int port, int slot, int state)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        padGetDmaStrOld(port, slot)->reqState = state;
        return 1;
    case 2:
        padGetDmaStrNew(port, slot)->reqState = state;
        return 1;
    default:
        return 0;
    }
}

void
padStateInt2String(int state, char buf[16])
{
    if ((unsigned int)state < (sizeof(padStateString)/sizeof(padStateString[0])))
        strcpy(buf, padStateString[state]);
}

void
padReqStateInt2String(int state, char buf[16])
{
    if ((unsigned int)state < (sizeof(padReqStateString)/sizeof(padReqStateString[0])))
        strcpy(buf, padReqStateString[state]);
}

int
padGetPortMax(void)
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_GET_PORTMAX_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_GET_PORTMAX_NEW;
            break;
        }
    default:
        return -1;
    }

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}

int
padGetSlotMax(int port)
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_GET_SLOTMAX_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_GET_SLOTMAX_NEW;
            break;
        }
    default:
        return -1;
    }
    buffer.padSlotMaxArgs.port = port;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}

int
padGetModVersion()
{
    if (padInitialised != 2)
        return 1; // Well.. return a low version #
    buffer.command = PAD_RPCCMD_GET_MODVER;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}

int
padInfoMode(int port, int slot, int infoMode, int index)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_INFO_MODE;
            buffer.padInfoModeArgs.port = port;
            buffer.padInfoModeArgs.slot = slot;
            buffer.padInfoModeArgs.infoMode = infoMode;
            buffer.padInfoModeArgs.index = index;

            if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
                return 0;

            if (buffer.padModeResult.result == 1)
                padSetReqState(port, slot, PAD_RSTAT_BUSY);
            return buffer.padModeResult.result;
        }
    case 2:
        {
            struct pad_data_new *pdata;

            pdata = padGetDmaStrNew(port, slot);

            if (pdata->currentTask != 1)
                return 0;
            if (pdata->reqState == PAD_RSTAT_BUSY)
                return 0;

            switch (infoMode)
            {
            case PAD_MODECURID:
                return (pdata->modeCurId == 0xF3) ? 0 : (pdata->modeCurId >> 4);
            case PAD_MODECUREXID:
                return (pdata->modeConfig == pdata->currentTask || ((unsigned int)(pdata->modeCurOffs) >= (sizeof(pdata->modeTable)/sizeof(pdata->modeTable[0])))) ? 0 : pdata->modeTable[pdata->modeCurOffs];
            case PAD_MODECUROFFS:
                return (pdata->modeConfig == 0) ? 0 : pdata->modeCurOffs;
            case PAD_MODETABLE:
                if (pdata->modeConfig != 0) {
                    if (index == -1)
                        return pdata->nrOfModes;
                    else if (index < pdata->nrOfModes && ((unsigned int)index <= (sizeof(pdata->modeTable)/sizeof(pdata->modeTable[0]))))
                        return pdata->modeTable[index];
                }
                return 0;
            default:
                return 0;
            }
        }
    default:
        return 0;
    }
}

int
padSetMainMode(int port, int slot, int mode, int lock)
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_SET_MMODE_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_SET_MMODE_NEW;
            break;
        }
    default:
        return -1;
    }
	buffer.padSetMainModeArgs.port = port;
	buffer.padSetMainModeArgs.slot = slot;
	buffer.padSetMainModeArgs.mode = mode;
	buffer.padSetMainModeArgs.lock = lock;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1)
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    return buffer.padModeResult.result;
}

int
padInfoPressMode(int port, int slot)
{
    return (padGetButtonMask(port, slot) ^ 0x3ffff) ? 0 : 1;
}

int
padEnterPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0xFFF);
}

int
padExitPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0);
}

int
padGetButtonMask(int port, int slot)
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_GET_BTNMASK_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_GET_BTNMASK_NEW;
            break;
        }
    default:
        return -1;
    }
	buffer.padGetButtonMaskArgs.port = port;
	buffer.padGetButtonMaskArgs.slot = slot;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    return buffer.padResult.result;
}

int
padSetButtonInfo(int port, int slot, int buttonInfo)
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_SET_BTNINFO_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_SET_BTNINFO_NEW;
            break;
        }
    default:
        return -1;
    }
	buffer.padSetButtonInfoArgs.port = port;
	buffer.padSetButtonInfoArgs.slot = slot;
	buffer.padSetButtonInfoArgs.buttonInfo = buttonInfo;

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    if (buffer.padSetButtonInfoResult.result == 1)
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    return buffer.padSetButtonInfoResult.result;
}

unsigned char
padInfoAct(int port, int slot, int actuator, int cmd)
{
    if ((unsigned int)port >= (sizeof(PadState)/sizeof(PadState[0])))
        return 0;
    if ((unsigned int)slot >= (sizeof(PadState[0])/sizeof(PadState[0][0])))
        return 0;

    switch (PadState[port][slot].open)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_INFO_ACT;
            buffer.padInfoActArgs.port = port;
            buffer.padInfoActArgs.slot = slot;
            buffer.padInfoActArgs.actuator = actuator;
            buffer.padInfoActArgs.act_cmd = cmd;

            if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
                return 0;

            if (buffer.padModeResult.result == 1)
                padSetReqState(port, slot, PAD_RSTAT_BUSY);
            return buffer.padModeResult.result;
        }
    case 2:
        {
            struct pad_data_new *pdata;

            pdata = padGetDmaStrNew(port, slot);

            if (pdata->currentTask != 1)
                return 0;
            if (pdata->modeConfig < 2)
                return 0;
            if (actuator >= pdata->nrOfActuators)
                return 0;

            if (actuator == -1)
                return pdata->nrOfActuators;   // # of acutators?

            if ((unsigned int)actuator >= (sizeof(pdata->actData)/sizeof(pdata->actData[0])))
                return 0;
            if ((unsigned int)cmd >= (sizeof(pdata->actData[0])/sizeof(pdata->actData[0][0])))
                return 0;

            return pdata->actData[actuator][cmd];
        }
    default:
        return 0;
    }
}

int
padSetActAlign(int port, int slot, const char actAlign[6])
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_SET_ACTALIGN_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_SET_ACTALIGN_NEW;
            break;
        }
    default:
        return -1;
    }
	buffer.padActDirAlignArgs.port = port;
	buffer.padActDirAlignArgs.slot = slot;

    memcpy(buffer.padActDirAlignArgs.align, actAlign, sizeof(buffer.padActDirAlignArgs.align));

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1)
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    return buffer.padModeResult.result;
}

int
padSetActDirect(int port, int slot, const char actAlign[6])
{
    switch (padInitialised)
    {
    case 1:
        {
            buffer.command = PAD_RPCCMD_SET_ACTDIR_OLD;
            break;
        }
    case 2:
        {
            buffer.command = PAD_RPCCMD_SET_ACTDIR_NEW;
            break;
        }
    default:
        return -1;
    }
	buffer.padActDirAlignArgs.port = port;
	buffer.padActDirAlignArgs.slot = slot;

    memcpy(buffer.padActDirAlignArgs.align, actAlign, sizeof(buffer.padActDirAlignArgs.align));

    if (sceSifCallRpc(&padsif[0], 1, 0, &buffer, sizeof(buffer), &buffer, sizeof(buffer), NULL, NULL) < 0)
        return 0;

    return buffer.padModeResult.result;
}

/*
 * This seems to have been removed from the official SDKs, very early during the PS2's lifetime.
 */
int
padGetConnection(int port, int slot)
{
    struct open_slot* oslt;

    if (padInitialised != 2)
        return 1;
    oslt = padGetConnDmaStr();
    if ((unsigned int)slot >= (sizeof(oslt->openSlots)/sizeof(oslt->openSlots[0])))
        return 1;
    return ((oslt->openSlots[port] >> slot) & 0x1);
}
