/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# IOP reset and status routines.
*/

#ifndef IOPCONTROL_H
#define IOPCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 Resets IOP, arg can be NULL or a module that will be loaded afterwards.
 This is a high level wrapper for SifIopReset
 @param arg a const character pointer for path to module or an empty string. NULL must not be specified.
 @return 1 for success or 0 for failure.
 */
int SifIopReboot(const char *arg);
/*!
 Resets IOP, arg can be NULL or a module that will be loaded afterwards.
 @param arg a const character pointer for path to module or NULL
 @param mode 0x100 for magicgate anything else for no magicgate.
 @return 1 for success or 0 for failure.
 */
int SifIopReset(const char *arg, int mode);
/*!
 @return 1 for SIF initialized or 0 for not initialized.
 */
int SifIopIsAlive(void);
/*!
 @return 1 for bootup complete or 0 for incomplete.
 */
int SifIopSync(void);

#ifdef __cplusplus
}
#endif

#endif /* IOPCONTROL_H */
