/*
 * libkbd.h - USB Keyboard Driver for PS2
 *
 * (c) 2003 TyRaNiD <tiraniddo@hotmail.com>
 *
 * This library provides access functions for the keyboard driver
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef __LIBKBD_H__
#define __LIBKBD_H__

#include <ps2kbd.h>

typedef kbd_rawkey PS2KbdRawKey;
typedef kbd_keymap PS2KbdKeyMap;

#ifdef __cplusplus
extern "C" {
#endif

int PS2KbdInit(void);
/* Initialise the keyboard library */
int PS2KbdRead(char *key);
/* Reads 1 character from the keyboard */
int PS2KbdReadRaw(PS2KbdRawKey *key);
/* Reads 1 raw character from the keyboard */
int PS2KbdSetReadmode(u32 readMode);
/* Sets the read mode to normal or raw */
int PS2KbdSetBlockingMode(u32 readMode);
/* Sets the blocking mode on or off */
int PS2KbdSetRepeatRate(u32 repeat);
/* Sets the repeat rate in millseconds */
int PS2KbdSetLeds(u8 leds);
/* Sets all connected keyboards leds */
int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps);
/* Sets the current keymap */
int PS2KbdSetCtrlmap(u8 *ctrlmap);
/* Sets the control key mappings */
int PS2KbdSetAltmap(u8 *altmap);
/* Sets the alt key mappings */
int PS2KbdSetSpecialmap(u8 *special);
/* Sets the special key mappings */
int PS2KbdFlushBuffer(void);
/* Flushes the keyboard buffer */
int PS2KbdResetKeymap(void);
/* Resets the keymap to the default US mapping */
int PS2KbdClose(void);
/* Close down the keyboard library */

#ifdef __cplusplus
}
#endif

#endif
