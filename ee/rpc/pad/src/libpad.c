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
#include <string.h>
#include <sifrpc.h>
#include <sifcmd.h>
#include "libpad.h"

/*
 * Defines
 */
#define PAD_BIND_RPC_ID1 0x8000010f
#define PAD_BIND_RPC_ID2 0x8000011f

#define PAD_RPCCMD_OPEN         0x80000100
#define PAD_RPCCMD_INFO_ACT     0x80000102
#define PAD_RPCCMD_INFO_COMB    0x80000103
#define PAD_RPCCMD_INFO_MODE    0x80000104
#define PAD_RPCCMD_SET_MMODE    0x80000105
#define PAD_RPCCMD_SET_ACTDIR   0x80000106
#define PAD_RPCCMD_SET_ACTALIGN 0x80000107
#define PAD_RPCCMD_GET_BTNMASK  0x80000108
#define PAD_RPCCMD_SET_BTNINFO  0x80000109
#define PAD_RPCCMD_SET_VREF     0x8000010a
#define PAD_RPCCMD_GET_PORTMAX  0x8000010b
#define PAD_RPCCMD_GET_SLOTMAX  0x8000010c
#define PAD_RPCCMD_CLOSE        0x8000010d
#define PAD_RPCCMD_END          0x8000010e

/*
 * Types
 */

struct pad_state
{
    int open;
    unsigned int port;
    unsigned int slot;
    struct pad_data *padData;
    unsigned char *padBuf;
};

// rom0:padman has only 64 byte of pad data
struct pad_data
{
    unsigned int frame;
    unsigned char state;
    unsigned char reqState;
    unsigned char ok;
    unsigned char unkn7;
    unsigned char data[32];
    unsigned int length;
    unsigned char request;
    /** CTP=1/no config; CTP=2/config, acts... */
    unsigned char CTP;
    /** 1, 2 or 3 */
    unsigned char model;
    /** the data in the buffer is already corrected */
    unsigned char correction;
    unsigned char errorCount;
    unsigned char unk49[15];
};

extern int _iop_reboot_count;
/*
 * Pad variables etc.
 */

static const char padStateString[8][16] = {"DISCONNECT", "FINDPAD",
                                           "FINDCTP1", "", "", "EXECCMD",
                                           "STABLE", "ERROR"};
static const char padReqStateString[3][16] = {"COMPLETE", "FAILED", "BUSY"};

static int padInitialised = 0;

// pad rpc call
static SifRpcClientData_t padsif[2] __attribute__((aligned(64)));
static union {
	s32 command;
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
} buffer __attribute__((aligned(16)));

/** Port state data */
static struct pad_state PadState[2][8];


/*
 * Local functions
 */

/** Common helper */
static struct pad_data*
padGetDmaStr(int port, int slot)
{
    struct pad_data *pdata;

    pdata = PadState[port][slot].padData;
    SyncDCache(pdata, (u8 *)pdata + 256);

    if(pdata[0].frame < pdata[1].frame) {
        return &pdata[1];
    }
    else {
        return &pdata[0];
    }
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
padReset()
{
    padInitialised = 0;
    padsif[0].server = NULL;
    padsif[1].server = NULL;
    return 0;
}

int
padInit(int a)
{
    // Version check isn't used by default
    // int ver;
    int i;
    static int _rb_count = 0;

    if (_rb_count != _iop_reboot_count)
    {
        _rb_count = _iop_reboot_count;
        padReset();
    }

    if(padInitialised)
        return 0;

    padsif[0].server = NULL;
    padsif[1].server = NULL;

    do {
        if (SifBindRpc(&padsif[0], PAD_BIND_RPC_ID1, 0) < 0) {
            return -1;
        }
        nopdelay();
    } while(!padsif[0].server);

    do {
        if (SifBindRpc(&padsif[1], PAD_BIND_RPC_ID2, 0) < 0) {
            return -3;
        }
        nopdelay();
    } while(!padsif[1].server);

    // If you require a special version of the padman, check for that here (uncomment)
    // ver = padGetModVersion();

    for(i = 0; i<8; i++)
    {
        PadState[0][i].open = 0;
        PadState[0][i].port = 0;
        PadState[0][i].slot = 0;
        PadState[1][i].open = 0;
        PadState[1][i].port = 0;
        PadState[1][i].slot = 0;
    }

    padInitialised = 1;
    return 0;

}

int
padEnd()
{

    int ret;


    buffer.command=PAD_RPCCMD_END;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    ret = buffer.padResult.result;
    if (ret == 1) {
        padInitialised = 0;
    }

    return ret;
}

int
padPortOpen(int port, int slot, void *padArea)
{

    int i;
    struct pad_data *dma_buf = (struct pad_data *)padArea;

    // Check 64 byte alignment
    if((u32)padArea & 0x3f) {
        //        scr_printf("dmabuf misaligned (%x)!!\n", dma_buf);
        return 0;
    }

    for (i=0; i<2; i++) {                // Pad data is double buffered
        memset(dma_buf[i].data, 0xff, 32); // 'Clear' data area
        dma_buf[i].frame = 0;
        dma_buf[i].length = 0;
        dma_buf[i].state = PAD_STATE_EXECCMD;
        dma_buf[i].reqState = PAD_RSTAT_BUSY;
        dma_buf[i].ok = 0;
        dma_buf[i].length = 0;
    }


	buffer.padOpenArgs.command = PAD_RPCCMD_OPEN;
	buffer.padOpenArgs.port = port;
    buffer.padOpenArgs.slot = slot;
    buffer.padOpenArgs.padArea = padArea;

    if(SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
    {
        return 0;
    }

    PadState[port][slot].open = 1;
    PadState[port][slot].padData = padArea;
    PadState[port][slot].padBuf = buffer.padOpenResult.padBuf;

    return buffer.padOpenResult.result;
}

int
padPortClose(int port, int slot)
{

    int ret;

	buffer.padCloseArgs.command = PAD_RPCCMD_CLOSE;
	buffer.padCloseArgs.port = port;
	buffer.padCloseArgs.slot = slot;
	buffer.padCloseArgs.mode = 1;

    ret = SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL);

    if(ret < 0)
        return ret;
    else {
        PadState[port][slot].open = 0;
        return buffer.padResult.result;
    }
}

unsigned char
padRead(int port, int slot, struct padButtonStatus *data)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);

    memcpy(data, pdata->data, pdata->length);
    return pdata->length;
}

int
padGetState(int port, int slot)
{
    struct pad_data *pdata;
    unsigned char state;


    pdata = padGetDmaStr(port, slot);

    state = pdata->state;

    if (state == PAD_STATE_STABLE) { // Ok
        if (padGetReqState(port, slot) == PAD_RSTAT_BUSY) {
            return PAD_STATE_EXECCMD;
        }
    }
    return state;
}

unsigned char
padGetReqState(int port, int slot)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);
    return pdata->reqState;
}

int
padSetReqState(int port, int slot, int state)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);
    pdata->reqState = state;
    return 1;
}

void
padStateInt2String(int state, char buf[16])
{

    if(state < 8) {
        strcpy(buf, padStateString[state]);
    }
}

void
padReqStateInt2String(int state, char buf[16])
{
    if(state < 4)
        strcpy(buf, padReqStateString[state]);
}

int
padGetPortMax(void)
{

	buffer.command = PAD_RPCCMD_GET_PORTMAX;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}

int
padGetSlotMax(int port)
{

    buffer.padSlotMaxArgs.command = PAD_RPCCMD_GET_SLOTMAX;
    buffer.padSlotMaxArgs.port = port;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}

int
padGetModVersion()
{
    return 1; // Well.. return a low version #
}

int
padInfoMode(int port, int slot, int infoMode, int index)
{
	buffer.padInfoModeArgs.command = PAD_RPCCMD_INFO_MODE;
	buffer.padInfoModeArgs.port = port;
	buffer.padInfoModeArgs.slot = slot;
	buffer.padInfoModeArgs.infoMode = infoMode;
	buffer.padInfoModeArgs.index = index;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return buffer.padModeResult.result;
}

int
padSetMainMode(int port, int slot, int mode, int lock)
{

	buffer.padSetMainModeArgs.command = PAD_RPCCMD_SET_MMODE;
	buffer.padSetMainModeArgs.port = port;
	buffer.padSetMainModeArgs.slot = slot;
	buffer.padSetMainModeArgs.mode = mode;
	buffer.padSetMainModeArgs.lock = lock;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return buffer.padModeResult.result;
}

int
padInfoPressMode(int port, int slot)
{
    int mask;

    mask = padGetButtonMask(port, slot);

    if (mask^0x3ffff) {
        return 0;
    }
    else {
        return 1;
    }
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

	buffer.padGetButtonMaskArgs.command = PAD_RPCCMD_GET_BTNMASK;
	buffer.padGetButtonMaskArgs.port = port;
	buffer.padGetButtonMaskArgs.slot = slot;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    return buffer.padResult.result;
}

int
padSetButtonInfo(int port, int slot, int buttonInfo)
{
    int val;

	buffer.padSetButtonInfoArgs.command = PAD_RPCCMD_SET_BTNINFO;
	buffer.padSetButtonInfoArgs.port = port;
	buffer.padSetButtonInfoArgs.slot = slot;
	buffer.padSetButtonInfoArgs.buttonInfo = buttonInfo;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    val = buffer.padSetButtonInfoResult.result;

    if (val == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return buffer.padSetButtonInfoResult.result;
}

unsigned char
padInfoAct(int port, int slot, int actuator, int cmd)
{
	buffer.padInfoActArgs.command = PAD_RPCCMD_INFO_ACT;
	buffer.padInfoActArgs.port = port;
	buffer.padInfoActArgs.slot = slot;
	buffer.padInfoActArgs.actuator = actuator;
	buffer.padInfoActArgs.act_cmd = cmd;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return buffer.padModeResult.result;
}

int
padSetActAlign(int port, int slot, char actAlign[6])
{
    int i;
    s8 *ptr;

	buffer.padActDirAlignArgs.command = PAD_RPCCMD_SET_ACTALIGN;
	buffer.padActDirAlignArgs.port = port;
	buffer.padActDirAlignArgs.slot = slot;

	ptr = buffer.padActDirAlignArgs.align;
    for (i=0; i<6; i++)
        ptr[i]=actAlign[i];

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    if (buffer.padModeResult.result == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return buffer.padModeResult.result;
}

int
padSetActDirect(int port, int slot, char actAlign[6])
{
    int i;
    s8 *ptr;

	buffer.padActDirAlignArgs.command = PAD_RPCCMD_SET_ACTDIR;
	buffer.padActDirAlignArgs.port = port;
	buffer.padActDirAlignArgs.slot = slot;

    ptr = buffer.padActDirAlignArgs.align;
    for (i=0; i<6; i++)
        ptr[i]=actAlign[i];

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return 0;

    return buffer.padModeResult.result;
}

int
padGetConnection(int port, int slot)
{
    return 1;
}
