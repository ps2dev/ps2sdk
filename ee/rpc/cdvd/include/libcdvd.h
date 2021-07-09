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
 * Definitions for libcdvd on the EE
 */

#ifndef __LIBCDVD_H__
#define __LIBCDVD_H__

#include <libcdvd-common.h>

#ifdef __cplusplus
extern "C" {
#endif

//EE-only libcdvd function prototypes.

/** read data to iop memory
 * 
 * @param lbn sector location to read from
 * @param sectors number of sectors to read
 * @param buf buffer to read to (in iop memory)
 * @param mode read mode
 * @return 1 if successful, 0 if error
 */
int sceCdReadIOPMem(u32 lbn, u32 sectors, void *buf, sceCdRMode *mode);

/** wait for disc to finish all n-commands
 * (shouldnt really need to call this yourself)
 * 
 * @return SCECdNotReady (6) if busy; SCECdComplete (2) if ready; 0 if error
 */
int sceCdNCmdDiskReady(void);

/** send an s-command by function number
 * 
 * @param command number
 * @param input buffer  (can be null)
 * @param size of input buffer  (0 - 16 bytes)
 * @param output buffer (can be null)
 * @param size of output buffer (0 - 16 bytes)
 * @return 1 if successful, 0 if error
 */
int sceCdApplySCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

/** send an n-command by function number
 * 
 * @param cmdNum command number
 * @param inBuff input buffer  (can be null)
 * @param inBuffSize size of input buffer  (0 - 16 bytes)
 * @param outBuff output buffer (can be null)
 * @param outBuffSize size of output buffer (0 - 16 bytes)
 * @return 1 if successful, 0 if error
 */
int sceCdApplyNCmd(u8 cmdNum, const void* inBuff, u16 inBuffSize, void* outBuff, u16 outBuffSize);

/** Controls spindle speed? Not sure what it really does.
 * SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
 *
 * @param speed Speed mode.
 * @return 1 on success, 0 on failure.
 */
int sceCdSpinCtrlEE(u32 speed);

#ifdef __cplusplus
}
#endif

#endif /* __LIBCDVD_H__ */
