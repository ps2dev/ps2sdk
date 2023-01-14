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
 * @brief RPC Interface for PS2 Remote Control Driver (RMMAN)
 */

#ifndef __LIBRM_H__
#define __LIBRM_H__

#include <librm-common.h>

struct remote_data
{
    u32 status;
    u32 button;
};

/**
 *
 * @name Status values (guess)
 */
/** @{ */
#define RM_INIT          0x000012FF
#define RM_READY         0x005A12FF
#define RM_KEYPRESSED    0x145A12FF
#define RM_NOREMOTE      0xFFFFFFFF
/** @} */

/**
 * @name Status button Values
 */
/** @{ */
#define RM_RELEASED      0x00000000
#define RM_IDLE          0x00FFFFFF
/** @}*/

/**
 * @name DVD Player buttons
 */
/** @{ */
#define RM_DVD_ONE       0x0000D049 /* 0 */
#define RM_DVD_TWO       0x0010D049
#define RM_DVD_THREE     0x0020D049
#define RM_DVD_FOUR      0x0030D049
#define RM_DVD_FIVE      0x0040D049
#define RM_DVD_SIX       0x0050D049
#define RM_DVD_SEVEN     0x0060D049
#define RM_DVD_EIGHT     0x0070D049
#define RM_DVD_NINE      0x0080D049
#define RM_DVD_ZERO      0x0090D049
#define RM_DVD_ENTER     0x00B0D049 /* 11 */
#define RM_DVD_BROWSE    0x00C0D049
#define RM_DVD_SET       0x00D0D049
#define RM_DVD_RETURN    0x00E0D049
#define RM_DVD_CLEAR     0x00F0D049
#define RM_DVD_SOURCE    0x0020D149 /* 18 */
#define RM_DVD_CHUP      0x0030D149
#define RM_DVD_CHDOWN    0x0040D149
#define RM_DVD_REC       0x0090D149 /* 25 */
#define RM_DVD_TITLE     0x00A0D149
#define RM_DVD_MENU      0x00B0D149
#define RM_DVD_PROGRAM   0x00F0D149 /* 31 */
#define RM_DVD_TIME      0x0080D249 /* 40 */
#define RM_DVD_ATOB      0x00A0D249 /* 42 */
#define RM_DVD_REPEAT    0x00C0D249 /* 44 */
#define RM_DVD_PREV      0x0000D349 /* 48 */
#define RM_DVD_NEXT      0x0010D349
#define RM_DVD_PLAY      0x0020D349
#define RM_DVD_SCAN_BACK 0x0030D349
#define RM_DVD_SCAN_FORW 0x0040D349
#define RM_DVD_SHUFFLE   0x0050D349
#define RM_DVD_STOP      0x0080D349 /* 56 */
#define RM_DVD_PAUSE     0x0090D349
#define RM_DVD_DISPLAY   0x0040D549 /* 84 */
#define RM_DVD_SLOW_BACK 0x0000D649 /* 96 */
#define RM_DVD_SLOW_FORW 0x0010D649
#define RM_DVD_SUBTITLE  0x0030D649
#define RM_DVD_AUDIO     0x0040D649 /* 100 */
#define RM_DVD_ANGLE     0x0050D649
#define RM_DVD_UP        0x0090D749 /* 122 */
#define RM_DVD_DOWN      0x00A0D749
#define RM_DVD_LEFT      0x00B0D749
#define RM_DVD_RIGHT     0x00C0D749
/** @}*/

/** @name Commands that works only on Dragon models
 */
/** @{ */
#define RM_PS2_RESET     0x0050D1DA /* 21 */
#define RM_PS2_EJECT     0x0060D1DA
#define RM_PS2_POWERON   0x00E0D2DA /* 46 */
#define RM_PS2_POWEROFF  0x00F0D2DA
/** @}*/

/** @name PS2 Controller buttons
 */
/** @{ */
#define RM_PS2_SELECT    0x0000D5DA /* 80 */
#define RM_PS2_L3        0x0010D5DA
#define RM_PS2_R3        0x0020D5DA
#define RM_PS2_START     0x0030D5DA
#define RM_PS2_UP        0x0040D5DA
#define RM_PS2_RIGHT     0x0050D5DA
#define RM_PS2_DOWN      0x0060D5DA
#define RM_PS2_LEFT      0x0070D5DA
#define RM_PS2_L2        0x0080D5DA
#define RM_PS2_R2        0x0090D5DA
#define RM_PS2_L1        0x00A0D5DA
#define RM_PS2_R1        0x00B0D5DA
#define RM_PS2_TRIANGLE  0x00C0D5DA
#define RM_PS2_CIRCLE    0x00D0D5DA
#define RM_PS2_CROSS     0x00E0D5DA
#define RM_PS2_SQUARE    0x00F0D5DA
/** @} */

#ifdef _cplusplus
extern "C" {
#endif

/**
 *  @brief Initialise librm
 *  @return == 1 => OK
 */
int RMMan_Init(void);

/**
 *  @brief Ends all remote communication
 *  @return == 1 => OK
 */
int RMMan_End(void);

/**
 *  @param [in] port Port number to open (0 or 1)
 *  @param [in] slot Slot to open (0 or 1: multitap not supported)
 *  @param [out] pData The address of the buffer for storing the pad status.
 *               Must be a 256-byte region (2xsizeof(struct pad_data).
 *               Must be a 64-byte aligned address.
 *  @return != 0 => OK
 */
int RMMan_Open(int port, int slot, void *pData);

/**
 *  @brief Closes an opened port.
 *
 *  @param port Port to close
 *  @param slot Slot to close
 *  @return != 0 => OK
 */
int RMMan_Close(int port, int slot);

/**
 *  @brief Read remote data
 *  @param [in] port Port number to get the status for.
 *  @param [in] slot Slot number to get the status for.
 *  @param [out] data A pointer to a 32 byte array where the result is stored
 *  @return != 0 => OK
 */
void RMMan_Read(int port, int slot, struct remote_data *data);

/**
 *  @brief Returns the rmman.irx version
 */
u32 RMMan_GetModuleVersion(void);

#ifdef _cplusplus
}
#endif

#endif /* __LIBRM_H__ */
