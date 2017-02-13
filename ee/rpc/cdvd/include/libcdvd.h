/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Definitions for libcdvd on the EE
*/

#ifndef _LIBCDVD_H_
#define _LIBCDVD_H_

#include <libcdvd-common.h>

#ifdef __cplusplus
extern "C" {
#endif

//EE-only libcdvd function prototypes.

// read data to iop memory
//
// arguments:	sector location to read from
//		number of sectors to read
//		buffer to read to (in iop memory)
//		read mode
// returns:	1 if successful
//		0 if error
int sceCdReadIOPMem(u32 lbn, u32 sectors, void *buf, sceCdRMode *mode);

// wait for disc to finish all n-commands
// (shouldnt really need to call this yourself)
//
// returns:	SCECdNotReady (6) if busy
//		SCECdComplete (2) if ready
//		0 if error
int sceCdNCmdDiskReady(void);

// send an s-command by function number
//
// arguments:	command number
//		input buffer  (can be null)
//		size of input buffer  (0 - 16 bytes)
//		output buffer (can be null)
//		size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//		0 if error
int sceCdApplySCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

// send an n-command by function number
//
// arguments:	command number
//		input buffer  (can be null)
//		size of input buffer  (0 - 16 bytes)
//		output buffer (can be null)
//		size of output buffer (0 - 16 bytes)
// returns:	1 if successful
//		0 if error
int sceCdApplyNCmd(u8 cmdNum, const void *inBuff, u16 inBuffSize, void *outBuff, u16 outBuffSize);

// Opens a specified configuration block, within NVRAM. Each block is 15 bytes long.
//
// arguments:	Block number.
//		Mode (0 = read, 1 = write).
//		Number of blocks.
//		Result code.
// returns:	1 on success, 0 on failure.
int sceCdOpenConfig(int block, int mode, int NumBlocks, u32 *status);

// Controls spindle speed? Not sure what it really does.
// SUPPORTED IN XCDVDMAN/XCDVDFSV ONLY
//
// arguments:	Speed mode.
// returns:	1 on success, 0 on failure.
int sceCdSpinCtrlEE(u32 speed);

#ifdef __cplusplus
}
#endif

#endif // _LIBCDVD_H_
