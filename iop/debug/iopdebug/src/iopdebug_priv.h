/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: iopdebug_priv.h $
#
# IOPDEBUG - IOP debugging library.
#
# iopdebug_priv.h - "private" header for internal definitions for the IOPDEBUG project.
#
*/

#ifndef _IOP_DEBUG_PRIV_H
#define _IOP_DEBUG_PRIV_H

#include <tamtypes.h>
#include <intrman.h>

#ifdef __cplusplus
extern "C" {
#endif

// Enter a critical state(interrupts are disabled)
static inline void EnterCritical(u32 *old_state) { CpuSuspendIntr((int *) old_state); }

// Exit a critical state(interrupts are resumed)
static inline void ExitCritical(u32 old_state) { CpuResumeIntr((int) old_state); }

#ifdef __cplusplus
}
#endif

#endif // #ifndef _IOP_DEBUG_PRIV_H
