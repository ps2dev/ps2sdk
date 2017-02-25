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

#include <tamtypes.h>
#include <fileio.h>
#include "libkbd.h"

extern int _iop_reboot_count;
static int kbd_iop = -1;
static int kbd_fd = -1;
static int curr_blockmode = PS2KBD_NONBLOCKING;
static int curr_readmode = PS2KBD_READMODE_NORMAL;

int PS2KbdInit(void)
{
  if (kbd_iop != _iop_reboot_count)
    {
      kbd_iop = _iop_reboot_count;
      kbd_fd = -1;
    }

  if(kbd_fd >= 0) /* Already initialised */
    {
      return 2;
    }

  kbd_fd = fioOpen(PS2KBD_DEVFILE, O_RDONLY);
  if(kbd_fd < 0)
    {
      return 0;
    }

  return 1;
}

int PS2KbdRead(char *key)
{
  if((kbd_fd >= 0) && (curr_readmode == PS2KBD_READMODE_NORMAL))
    {
      return fioRead(kbd_fd, key, 1);
    }

  return 0;
}

int PS2KbdReadRaw(PS2KbdRawKey *key)
{
  if((kbd_fd >= 0) && (curr_readmode == PS2KBD_READMODE_RAW))
    {
      return fioRead(kbd_fd, key, 2) / 2;
    }

  return 0;
}

int PS2KbdSetReadmode(u32 readmode)
{
  if((kbd_fd >= 0) && (curr_readmode != readmode))
    {
      curr_readmode = readmode;
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETREADMODE, &readmode);
    }
  return 0;
}

int PS2KbdSetBlockingMode(u32 blockmode)
{
  if((kbd_fd >= 0) && (curr_blockmode != blockmode))
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETBLOCKMODE, &blockmode);
    }

  return 0;
}

int PS2KbdSetRepeatRate(u32 repeat)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETREPEATRATE, &repeat);
    }
  return 0;
}

int PS2KbdSetLeds(u8 leds)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETLEDS, &leds);
    }
  return 0;
}

int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETKEYMAP, keymaps);
    }
  return 0;
}

int PS2KbdSetCtrlmap(u8 *ctrlmap)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETCTRLMAP, ctrlmap);
    }
  return 0;
}

int PS2KbdSetAltmap(u8 *altmap)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETALTMAP, altmap);
    }
  return 0;
}

int PS2KbdSetSpecialmap(u8 *special)
{
  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_SETSPECIALMAP, special);
    }
  return 0;
}

int PS2KbdFlushBuffer(void)
{
  int dummy;

  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_FLUSHBUFFER, &dummy);
    }
  return 0;
}

int PS2KbdResetKeymap(void)
{
  int dummy;

  if(kbd_fd >= 0)
    {
      return fioIoctl(kbd_fd, PS2KBD_IOCTL_RESETKEYMAP, &dummy);
    }
  return 0;
}

int PS2KbdClose(void)
{
  if(kbd_fd >= 0)
    {
      fioClose(kbd_fd);
      kbd_fd = -1;
    }

  return 1;
}
