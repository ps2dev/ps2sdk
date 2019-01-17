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
 * Pad externals
 */

#ifndef __LIBPAD_H__
#define __LIBPAD_H__

/*
 * Button bits
 */
#ifndef __INPUT_H__
#define PAD_LEFT      0x0080
#define PAD_DOWN      0x0040
#define PAD_RIGHT     0x0020
#define PAD_UP        0x0010
#define PAD_START     0x0008
#define PAD_R3        0x0004
#define PAD_L3        0x0002
#define PAD_SELECT    0x0001
#define PAD_SQUARE    0x8000
#define PAD_CROSS     0x4000
#define PAD_CIRCLE    0x2000
#define PAD_TRIANGLE  0x1000
#define PAD_R1        0x0800
#define PAD_L1        0x0400
#define PAD_R2        0x0200
#define PAD_L2        0x0100
#endif
/*
 * Pad states
 */
#define PAD_STATE_DISCONN       0x00
#define PAD_STATE_FINDPAD       0x01
#define PAD_STATE_FINDCTP1      0x02
#define PAD_STATE_EXECCMD       0x05
#define PAD_STATE_STABLE        0x06
#define PAD_STATE_ERROR         0x07

/*
 * Pad request states
 */
#define PAD_RSTAT_COMPLETE      0x00
#define PAD_RSTAT_FAILED        0x01
#define PAD_RSTAT_BUSY          0x02

/*
 * Connected pad type
 */
#define PAD_TYPE_NEJICON    0x2
#define PAD_TYPE_KONAMIGUN  0x3
#define PAD_TYPE_DIGITAL    0x4
#define PAD_TYPE_ANALOG     0x5
#define PAD_TYPE_NAMCOGUN   0x6
#define PAD_TYPE_DUALSHOCK  0x7
#define PAD_TYPE_JOGCON     0xE
#define PAD_TYPE_EX_TSURICON 0x100
#define PAD_TYPE_EX_JOGCON  0x300
/*
 * padInfoMode values
 */
#define PAD_MODECURID   1
#define PAD_MODECUREXID 2
#define PAD_MODECUROFFS 3
#define PAD_MODETABLE   4

/*
 * padSetMainMode
 */
#define PAD_MMODE_DIGITAL   0
#define PAD_MMODE_DUALSHOCK 1

#define PAD_MMODE_UNLOCK    2
#define PAD_MMODE_LOCK      3

/*
 * padInfoAct cmds
 */
#define PAD_ACTFUNC     1
#define PAD_ACTSUB      2
#define PAD_ACTSIZE     3
#define PAD_ACTCURR     4

/** Button info */
struct padButtonStatus
{
    unsigned char ok;
    unsigned char mode;
    unsigned short btns;
    // joysticks
    unsigned char rjoy_h;
    unsigned char rjoy_v;
    unsigned char ljoy_h;
    unsigned char ljoy_v;
    // pressure mode
    unsigned char right_p;
    unsigned char left_p;
    unsigned char up_p;
    unsigned char down_p;
    unsigned char triangle_p;
    unsigned char circle_p;
    unsigned char cross_p;
    unsigned char square_p;
    unsigned char l1_p;
    unsigned char r1_p;
    unsigned char l2_p;
    unsigned char r2_p;
    unsigned char unkn16[12];
} __attribute__((packed));

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise libpad
 * @param mode Must be set to 0.
 * @return == 1 => OK
 */
int padInit(int mode);

/** Initialise pad ports. Automatically called by padInit(), there is no need to call this function directly.
 * @param mode Must be set to 0.
 * @return == 1 => OK
 *
 * Note: PADMAN from release 1.3.4 does not have this function implemented.
 * As a result, it is impossible to reinitialize libpad after calling padEnd().
 *
 * @return == 1 => OK
 */
int padPortInit(int mode);

/** Ends all pad communication
  * Note: PADMAN from release 1.3.4 does not have padPortInit implemented.
  * As a result, it is impossible to reinitialize libpad after calling padEnd().
  * This was known as padClose in the really early official SDK releases.
  *
  * @return == 1 => OK
  */
int padEnd(void);

/**
 * @param port Port to open
 * @param slot Slot to open
 * @param padArea The address of the buffer for storing the pad status. Must be a 256-byte region (2xsizeof(struct pad_data).
 *                Must be a 64-byte aligned address. For the old libpad, at least 16-bytes alignment.
 * @return != 0 => OK
 */
int padPortOpen(int port, int slot, void *padArea);

/**
 * Closes an opened port.
 *
 * @param port Port to close
 * @param slot Slot to close
 * @return != 0 => OK
 */
int padPortClose(int port, int slot);

/** Read pad data
 * @param port Port number of the pad to get the status for.
 * @param slot Slot number of the pad to get the status for.
 * @param data A pointer to a 32 byte array where the result is stored
 * @return != 0 => OK
 */
unsigned char padRead(int port, int slot, struct padButtonStatus *data);

/** Get current pad state
 * Wait until state == 6 (Ready) before trying to access the pad
 */
int padGetState(int port, int slot);

/** Get pad request state*/
unsigned char padGetReqState(int port, int slot);

/** Set pad request state (after a param setting)
 * No need to export this one perhaps..
 */
int padSetReqState(int port, int slot, int state);

/*
 * Debug print functions
 */
void padStateInt2String(int state, char buf[16]);
void padReqStateInt2String(int state, char buf[16]);

/** Returns # slots on the PS2 (usally 2)
 */
int padGetPortMax(void);

/** Returns # slots the port has (usually 1)
 * probably 4 if using a multi tap (not tested)
 */
int padGetSlotMax(int port);

/** Returns the padman.irx version
 * NOT SUPPORTED on module rom0:padman
 */
int padGetModVersion();

/** Get pad info (digital (4), dualshock (7), etc..)
 *
 * @return 3 - KONAMI GUN; 4 - DIGITAL PAD; 5 - JOYSTICK; 6 - NAMCO GUN; 7 - DUAL SHOCK
 */
int padInfoMode(int port, int slot, int infoMode, int index);

/**
 * mode = 1, -> Analog/dual shock enabled; mode = 0 -> Digital
 * lock = 3 -> Mode not changeable by user
 */
int padSetMainMode(int port, int slot, int mode, int lock);

/** Check if the pad has pressure sensitive buttons */
int padInfoPressMode(int port, int slot);

/** Pressure sensitive mode ON */
int padEnterPressMode(int port, int slot);

/** Check for newer version
 * Pressure sensitive mode OFF
 */
int padExitPressMode(int port, int slot);

/*
 * Dunno if these need to be exported
 */
int padGetButtonMask(int port, int slot);
int padSetButtonInfo(int port, int slot, int buttonInfo);

/** Get actuator status for this controller
 * If padInfoAct(port, slot, -1, 0) != 0, the controller has actuators
 * (i think ;) )
 */
unsigned char padInfoAct(int port, int slot, int word, int byte);

/** Initalise actuators. On dual shock controller:
 * act_align[0] = 0 enables 'small' engine
 * act_align[1] = 1 enables 'big' engine
 * set act_align[2-5] to 0xff (disable)
 */
int padSetActAlign(int port, int slot, char act_align[6]);

/** Set actuator status on dual shock controller,
 * act_align[0] = 0/1 turns off/on 'small' engine
 * act_align[1] = 0-255 sets 'big' engine speed
 */
int padSetActDirect(int port, int slot, char act_align[6]);

/** Returns whether the device at port,slot is connected (1 = connected)
 * Appears to have been removed very early during the PS2's lifetime.
 * If possible, use the documented padGetState instead.
 *
 * NOT SUPPORTED with module rom0:padman
 */
int padGetConnection(int port, int slot);

#ifdef __cplusplus
}
#endif

#endif /* __LIBPAD_H__ */
