/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Special IOP reboot routines.
 */

#ifndef __IOPCONTROL_SPECIAL_H__
#define __IOPCONTROL_SPECIAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Resets IOP
 * Reboots the IOP with an encrypted UDNL module.
 * @param udnl Pointer to UDNL module to reboot with.
 * @param size Size of UDNL module in bytes.
 * @return 1 for success or 0 for failure.
 */
int SifIopRebootBufferEncrypted(const void *udnl, int size);
/** Resets IOP
 * Reboots the IOP with an IOPRP image. If the image contains an IOPBTCONF file,
 * the IOPBTCONF file will be automatically split off into its own image
 * @param ioprp Pointer to IOPRP image to reboot with.
 * @param size Size of the IOPRP image in bytes.
 * @return 1 for success or 0 for failure.
 */
int SifIopRebootBuffer(const void *ioprp, int size);

#ifdef __cplusplus
}
#endif

#endif /* __IOPCONTROL_SPECIAL_H__ */
