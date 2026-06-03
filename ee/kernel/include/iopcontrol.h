/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * IOP reset and status routines.
 */

#ifndef __IOPCONTROL_H__
#define __IOPCONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Resets IOP
 * This is a high level wrapper for SifIopReset
 * @param arg a const character pointer for path to module that will be loaded afterwards or an empty string. NULL must not be specified.
 * @return 1 for success or 0 for failure.
 */
extern int SifIopReboot(const char *arg);
/** Resets IOP
 * @param arg a const character pointer for path to module that will be loaded afterwards or a blank. NULL is not officially supported, but is supported for backward-compatibility with old homebrew projects.
 * @param mode Bitmask for optional settings. 0x80000000 for verbose messages and 0x100 for magicgate.
 * @return 1 for success or 0 for failure.
 */
extern int SifIopReset(const char *arg, int mode);
/**
 * @return 1 for SIF initialized or 0 for not initialized.
 */
extern int SifIopIsAlive(void);
/**
 * @return 1 for bootup complete or 0 for incomplete.
 */
extern int SifIopSync(void);

/**
 * @return 1 if IOP reboot count has changed since last call or 0 if not
 */
static inline int HasIopRebootedSinceLastCall(void)
{
    static int _rb_count;
    extern int _iop_reboot_count;
    if (_rb_count != _iop_reboot_count) {
        _rb_count = _iop_reboot_count;
        return 1;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __IOPCONTROL_H__ */
