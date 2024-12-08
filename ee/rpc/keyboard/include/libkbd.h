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
 * USB Keyboard Driver for PS2
 */

#ifndef __LIBKBD_H__
#define __LIBKBD_H__

#include <ps2kbd.h>

typedef kbd_rawkey PS2KbdRawKey;
typedef kbd_keymap PS2KbdKeyMap;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the keyboard library */
extern int PS2KbdInit(void);
/** Reads 1 character from the keyboard */
extern int PS2KbdRead(char *key);
/** Reads 1 raw character from the keyboard */
extern int PS2KbdReadRaw(PS2KbdRawKey *key);
/** Sets the read mode to normal or raw */
extern int PS2KbdSetReadmode(u32 readMode);
/** Sets the blocking mode on or off */
extern int PS2KbdSetBlockingMode(u32 readMode);
/** Sets the repeat rate in millseconds */
extern int PS2KbdSetRepeatRate(u32 repeat);
/** Sets all connected keyboards leds */
extern int PS2KbdSetLeds(u8 leds);
/** Sets the current keymap */
extern int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps);
/** Sets the control key mappings */
extern int PS2KbdSetCtrlmap(u8 *ctrlmap);
/** Sets the alt key mappings */
extern int PS2KbdSetAltmap(u8 *altmap);
/** Sets the special key mappings */
extern int PS2KbdSetSpecialmap(u8 *special);
/** Flushes the keyboard buffer */
extern int PS2KbdFlushBuffer(void);
/** Resets the keymap to the default US mapping */
extern int PS2KbdResetKeymap(void);
/** Close down the keyboard library */
extern int PS2KbdClose(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIBKBD_H__ */
