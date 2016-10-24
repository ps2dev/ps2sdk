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
# Pad library functions
# Quite easy rev engineered from util demos..
# Find any bugs? Mail me: pukko@home.se
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

#define PAD_RPCCMD_OPEN 0x80000100
#define PAD_RPCCMD_INFO_ACT 0x80000102
#define PAD_RPCCMD_INFO_COMB 0x80000103
#define PAD_RPCCMD_INFO_MODE 0x80000104
#define PAD_RPCCMD_SET_MMODE 0x80000105
#define PAD_RPCCMD_SET_ACTDIR 0x80000106
#define PAD_RPCCMD_SET_ACTALIGN 0x80000107
#define PAD_RPCCMD_GET_BTNMASK 0x80000108
#define PAD_RPCCMD_SET_BTNINFO 0x80000109
#define PAD_RPCCMD_SET_VREF 0x8000010a
#define PAD_RPCCMD_GET_PORTMAX 0x8000010b
#define PAD_RPCCMD_GET_SLOTMAX 0x8000010c
#define PAD_RPCCMD_CLOSE 0x8000010d
#define PAD_RPCCMD_END 0x8000010e

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
    unsigned char CTP;         //CTP=1/no config; CTP=2/config, acts...
    unsigned char model;       //1, 2 or 3
    unsigned char correction;  //the data in the buffer is already corrected
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
static char buffer[128] __attribute__((aligned(16)));

/* Port state data */
static struct pad_state PadState[2][8];


/*
 * Local functions
 */

/*
 * Common helper
 */
static struct pad_data *
padGetDmaStr(int port, int slot)
{
    struct pad_data *pdata;

    pdata = PadState[port][slot].padData;
    SyncDCache(pdata, (u8 *)pdata + 256);

    if (pdata[0].frame < pdata[1].frame) {
        return &pdata[1];
    } else {
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

/*
 * Since padEnd() further below doesn't work right, a pseudo function is needed
 * to allow recovery after IOP reset. This function has nothing to do with the
 * functions of the IOP modules. It merely resets variables for the EE routines.
 */
int padReset()
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
int padInit(int a)
{
    // Version check isn't used by default
    // int ver;
    int i;
    static int _rb_count = 0;

    if (_rb_count != _iop_reboot_count) {
        _rb_count = _iop_reboot_count;
        padReset();
    }

    if (padInitialised)
        return 0;

    padsif[0].server = NULL;
    padsif[1].server = NULL;

    do {
        if (SifBindRpc(&padsif[0], PAD_BIND_RPC_ID1, 0) < 0) {
            return -1;
        }
        nopdelay();
    } while (!padsif[0].server);

    do {
        if (SifBindRpc(&padsif[1], PAD_BIND_RPC_ID2, 0) < 0) {
            return -3;
        }
        nopdelay();
    } while (!padsif[1].server);

    // If you require a special version of the padman, check for that here (uncomment)
    // ver = padGetModVersion();

    for (i = 0; i < 8; i++) {
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


/*
 * Ends all pad communication
 */
int padEnd()
{

    int ret;


    *(u32 *)(&buffer[0]) = PAD_RPCCMD_END;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return -1;

    ret = *(int *)(&buffer[12]);
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
int padPortOpen(int port, int slot, void *padArea)
{

    int i;
    struct pad_data *dma_buf = (struct pad_data *)padArea;

    // Check 64 byte alignment
    if ((u32)padArea & 0x3f) {
        //        scr_printf("dmabuf misaligned (%x)!!\n", dma_buf);
        return 0;
    }

    for (i = 0; i < 2; i++) {               // Pad data is double buffered
        memset(dma_buf[i].data, 0xff, 32);  // 'Clear' data area
        dma_buf[i].frame = 0;
        dma_buf[i].length = 0;
        dma_buf[i].state = PAD_STATE_EXECCMD;
        dma_buf[i].reqState = PAD_RSTAT_BUSY;
        dma_buf[i].ok = 0;
        dma_buf[i].length = 0;
    }


    *(u32 *)(&buffer[0]) = PAD_RPCCMD_OPEN;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[16]) = (u32)padArea;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0) {
        return 0;
    }

    PadState[port][slot].open = 1;
    PadState[port][slot].padData = padArea;
    PadState[port][slot].padBuf = *(char **)(&buffer[20]);

    return *(u32 *)(&buffer[12]);
}

int padPortClose(int port, int slot)
{

    int ret;

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_CLOSE;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[16]) = 1;

    ret = SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL);

    if (ret < 0)
        return ret;
    else {
        PadState[port][slot].open = 0;
        return *(int *)(&buffer[12]);
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
int padGetState(int port, int slot)
{
    struct pad_data *pdata;
    unsigned char state;


    pdata = padGetDmaStr(port, slot);

    state = pdata->state;

    if (state == PAD_STATE_STABLE) {  // Ok
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
int padSetReqState(int port, int slot, int state)
{

    struct pad_data *pdata;

    pdata = padGetDmaStr(port, slot);
    pdata->reqState = state;
    return 1;
}


/*
 * Debug print functions
 */
void padStateInt2String(int state, char buf[16])
{

    if (state < 8) {
        strcpy(buf, padStateString[state]);
    }
}

void padReqStateInt2String(int state, char buf[16])
{
    if (state < 4)
        strcpy(buf, padReqStateString[state]);
}


/*
 * Returns # slots on the PS2 (usally 2)
 */
int padGetPortMax(void)
{

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_GET_PORTMAX;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return -1;

    return *(int *)(&buffer[12]);
}


/*
 * Returns # slots the port has (usually 1)
 * probably 4 if using a multi tap (not tested)
 */
int padGetSlotMax(int port)
{

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_GET_SLOTMAX;
    *(u32 *)(&buffer[4]) = port;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return -1;

    return *(int *)(&buffer[12]);
}


/*
 * Returns the padman version
 * NOT SUPPORTED on module rom0:padman
 */
int padGetModVersion()
{
    return 1;  // Well.. return a low version #
}

/*
 * Get pad info (digital (4), dualshock (7), etc..)
 * ID: 3 - KONAMI GUN
 *     4 - DIGITAL PAD
 *     5 - JOYSTICK
 *     6 - NAMCO GUN
 *     7 - DUAL SHOCK
 */
int padInfoMode(int port, int slot, int infoMode, int index)
{
    *(u32 *)(&buffer[0]) = PAD_RPCCMD_INFO_MODE;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[12]) = infoMode;
    *(u32 *)(&buffer[16]) = index;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    if (*(int *)(&buffer[20]) == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return *(int *)(&buffer[20]);
}


/*
 * mode = 1, -> Analog/dual shock enabled; mode = 0 -> Digital
 * lock = 3 -> Mode not changeable by user
 */
int padSetMainMode(int port, int slot, int mode, int lock)
{

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_SET_MMODE;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[12]) = mode;
    *(u32 *)(&buffer[16]) = lock;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    if (*(int *)(&buffer[20]) == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return *(int *)(&buffer[20]);
}


/*
 * Check if the pad has pressure sensitive buttons
 */
int padInfoPressMode(int port, int slot)
{
    int mask;

    mask = padGetButtonMask(port, slot);

    if (mask ^ 0x3ffff) {
        return 0;
    } else {
        return 1;
    }
}


/*
 * Pressure sensitive mode ON
 */
int padEnterPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0xFFF);
}


/*
 * Check for newer version
 * Pressure sensitive mode OFF
 */
int padExitPressMode(int port, int slot)
{
    return padSetButtonInfo(port, slot, 0);
}


/*
 *
 */
int padGetButtonMask(int port, int slot)
{

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_GET_BTNMASK;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    return *(int *)(&buffer[12]);
}


/*
 *
 */
int padSetButtonInfo(int port, int slot, int buttonInfo)
{
    int val;

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_SET_BTNINFO;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[12]) = buttonInfo;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    val = *(int *)(&buffer[16]);

    if (val == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return *(int *)(&buffer[16]);
}


/*
 * Get actuator status for this controller
 * If padInfoAct(port, slot, -1, 0) != 0, the controller has actuators
 * (i think ;) )
 */
unsigned char
padInfoAct(int port, int slot, int actuator, int cmd)
{
    *(u32 *)(&buffer[0]) = PAD_RPCCMD_INFO_ACT;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;
    *(u32 *)(&buffer[12]) = actuator;
    *(u32 *)(&buffer[16]) = cmd;

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    if (*(int *)(&buffer[20]) == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return *(int *)(&buffer[20]);
}


/*
 * Initalise actuators. On dual shock controller:
 * actAlign[0] = 0 enables 'small' engine
 * actAlign[1] = 1 enables 'big' engine
 * set actAlign[2-5] to 0xff (disable)
 */
int padSetActAlign(int port, int slot, char actAlign[6])
{
    int i;
    char *ptr;

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_SET_ACTALIGN;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;

    ptr = (char *)(&buffer[12]);
    for (i = 0; i < 6; i++)
        ptr[i] = actAlign[i];

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    if (*(int *)(&buffer[20]) == 1) {
        padSetReqState(port, slot, PAD_RSTAT_BUSY);
    }
    return *(int *)(&buffer[20]);
}


/*
 * Set actuator status
 * On dual shock controller,
 * actAlign[0] = 0/1 turns off/on 'small' engine
 * actAlign[1] = 0-255 sets 'big' engine speed
 */
int padSetActDirect(int port, int slot, char actAlign[6])
{
    int i;
    char *ptr;

    *(u32 *)(&buffer[0]) = PAD_RPCCMD_SET_ACTDIR;
    *(u32 *)(&buffer[4]) = port;
    *(u32 *)(&buffer[8]) = slot;

    ptr = (char *)(&buffer[12]);
    for (i = 0; i < 6; i++)
        ptr[i] = actAlign[i];

    if (SifCallRpc(&padsif[0], 1, 0, buffer, 128, buffer, 128, NULL, NULL) < 0)
        return 0;

    return *(int *)(&buffer[20]);
}


/*
 * Dunno about this one.. always returns 1?
 * I guess it should've returned if the pad was connected..
 *
 * NOT SUPPORTED with module rom0:padman
 */
int padGetConnection(int port, int slot)
{
    return 1;
}
