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
# Pad library functions for the new PADMAN module.
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
#define PAD_BIND_RPC_ID1 0x80000100
#define PAD_BIND_RPC_ID2 0x80000101

#define PAD_RPCCMD_OPEN         0x01
#define PAD_RPCCMD_SET_MMODE    0x06
#define PAD_RPCCMD_SET_ACTDIR   0x07
#define PAD_RPCCMD_SET_ACTALIGN 0x08
#define PAD_RPCCMD_GET_BTNMASK  0x09
#define PAD_RPCCMD_SET_BTNINFO  0x0A
#define PAD_RPCCMD_SET_VREF     0x0B
#define PAD_RPCCMD_GET_PORTMAX  0x0C
#define PAD_RPCCMD_GET_SLOTMAX  0x0D
#define PAD_RPCCMD_CLOSE        0x0E
#define PAD_RPCCMD_END          0x0F
#define PAD_RPCCMD_INIT         0x10
#define PAD_RPCCMD_GET_CONNECT  0x11
#define PAD_RPCCMD_GET_MODVER   0x12

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

struct pad_data
{
    u8 data[32];
	u32 actDirData[2];
	u32 actAlignData[2];
    u8  actData[32];
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

struct open_slot
{
	u32 frame;
	u32 openSlots[2];
	u8  padding[116];
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
		s32 command;
		void *buffer;
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
		s8 align[6];
	} padActDirAlignArgs;
	char buffer[128];
} buffer __attribute__((aligned(64)));

static struct open_slot openSlot[2];

/* Port state data */
static struct pad_state PadState[2][8];


/*
 * Local functions
 */

/*
 * Common helper
 */
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


/*
 * Initialise padman
 * a = 0 should work..
 */
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

	buffer.padInitArgs.command = PAD_RPCCMD_INIT;
	buffer.padInitArgs.buffer = openSlot;
    if (SifCallRpc( &padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    padInitialised = 1;
    return 0;

}


/*
 * Ends all pad communication.
 */
int
padEnd()
{

    int ret;

	buffer.command = PAD_RPCCMD_END;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    ret = buffer.padResult.result;
    if (ret == 1) {
        padInitialised = 0;
    }

    return ret;
}


/*
 * The user should provide a pointer to a 256 byte (2xsizeof(struct pad_data))
 * 64 byte aligned pad data area for each pad port opened
 *
 * return != 0 => OK
 */
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
        dma_buf[i].currentTask = 0;
        dma_buf[i].length = 0;
        dma_buf[i].buttonDataReady = 0; // Should be cleared in newer padman
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


/*
 * Read pad data
 * Result is stored in 'data' which should point to a 32 byte array
 */
unsigned char
padRead(int port, int slot, struct padButtonStatus *data)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);

    memcpy(data, pdata->data, pdata->length);
    return pdata->length;
}


/*
 * Get current pad state
 * Wait until state == 6 (Ready) before trying to access the pad
 */
int
padGetState(int port, int slot)
{
    struct pad_data *pdata;
    unsigned char state;


    pdata = padGetDmaStr(port, slot);

    state = pdata->state;

    if (state == PAD_STATE_ERROR)
    {
        if (pdata->findPadRetries)
        {
            return PAD_STATE_FINDPAD;
        }
    }

    if (state == PAD_STATE_STABLE) { // Ok
        if (padGetReqState(port, slot) == PAD_RSTAT_BUSY) {
            return PAD_STATE_EXECCMD;
        }
    }
    return state;
}


/*
 * Get pad request state
 */
unsigned char
padGetReqState(int port, int slot)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);
    return pdata->reqState;
}


/*
 * Set pad request state (after a param setting)
 */
int
padSetReqState(int port, int slot, int state)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);
    pdata->reqState = state;
    return 1;
}


/*
 * Debug print functions
 */
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


/*
 * Returns # slots on the PS2 (usally 2)
 */
int
padGetPortMax(void)
{

    buffer.command = PAD_RPCCMD_GET_PORTMAX;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}


/*
 * Returns # slots the port has (usually 1)
 * probably 4 if using a multi tap (not tested)
 */
int
padGetSlotMax(int port)
{

	buffer.padSlotMaxArgs.command = PAD_RPCCMD_GET_SLOTMAX;
	buffer.padSlotMaxArgs.port = port;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}


/*
 * Returns the padman version
 */
int
padGetModVersion()
{
	buffer.command = PAD_RPCCMD_GET_MODVER;

    if (SifCallRpc(&padsif[0], 1, 0, &buffer, 128, &buffer, 128, NULL, NULL) < 0)
        return -1;

    return buffer.padResult.result;
}


/*
 * Get pad info (digital (4), dualshock (7), etc..)
 * ID: 3 - KONAMI GUN
 *     4 - DIGITAL PAD
 *     5 - JOYSTICK
 *     6 - NAMCO GUN
 *     7 - DUAL SHOCK
 */
int
padInfoMode(int port, int slot, int infoMode, int index)
{
    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);

    if (pdata->currentTask != 1)
        return 0;
    if (pdata->reqState == PAD_RSTAT_BUSY)
        return 0;

    switch(infoMode) {
    case PAD_MODECURID:
        if (pdata->modeCurId == 0xF3)
            return 0;
        else
            return (pdata->modeCurId >> 4);
        break;

    case PAD_MODECUREXID:
        if (pdata->modeConfig == pdata->currentTask)
            return 0;
        return pdata->modeTable[pdata->modeCurOffs];
        break;

    case PAD_MODECUROFFS:
        if (pdata->modeConfig != 0)
            return pdata->modeCurOffs;
        else
            return 0;
        break;

    case PAD_MODETABLE:
        if (pdata->modeConfig != 0) {
            if(index == -1) {
                return pdata->nrOfModes;
            }
            else if (index < pdata->nrOfModes) {
                return pdata->modeTable[index];
            }
            else {
                return 0;
            }
        }
        else
            return 0;
        break;
    }
    return 0;
}


/*
 * mode = 1, -> Analog/dual shock enabled; mode = 0 -> Digital
 * lock = 3 -> Mode not changeable by user
 */
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


/*
 * Check if the pad has pressure sensitive buttons
 */
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


/*
 * Pressure sensitive mode ON
 */
int
padEnterPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0xFFF);
}


/*
 * Check for newer version
 * Pressure sensitive mode OFF
 */
int
padExitPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0);

}


/*
 *
 */
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


/*
 *
 */
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


/*
 * Get actuator status for this controller
 * If padInfoAct(port, slot, -1, 0) != 0, the controller has actuators
 * (i think ;) )
 */
unsigned char
padInfoAct(int port, int slot, int actuator, int cmd)
{
    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);

    if (pdata->currentTask != 1)
        return 0;
    if (pdata->modeConfig < 2)
        return 0;
    if (actuator >= pdata->nrOfActuators)
        return 0;

    if (actuator == -1)
        return pdata->nrOfActuators;   // # of acutators?

    if (cmd >= 4)
        return 0;

    return pdata->actData[actuator*4+cmd];
}


/*
 * Initalise actuators. On dual shock controller:
 * actAlign[0] = 0 enables 'small' engine
 * actAlign[1] = 1 enables 'big' engine
 * set actAlign[2-5] to 0xff (disable)
 */
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


/*
 * Set actuator status
 * On dual shock controller,
 * actAlign[0] = 0/1 turns off/on 'small' engine
 * actAlign[1] = 0-255 sets 'big' engine speed
 */
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
	struct open_slot *oslot;

	if(openSlot[0].frame < openSlot[1].frame)
		oslot = &openSlot[1];
	else
		oslot = &openSlot[0];

	return ((oslot->openSlots[port] >> slot) & 0x1);
}
