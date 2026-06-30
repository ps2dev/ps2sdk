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
 * PS2 ROM0 Info.
 * Some useful method for extracting info from ROM0
 */

#ifndef __ROM0_INFO_H__
#define __ROM0_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int (*open)(const char *name, int flags, ...);
    int (*close)(int fd);
    int (*read)(int fd, void *buf, int nbyte);
    int openFlags;
} _io_driver;

extern void SetupRomInfo(void);

/** check whether the PlayStation 2 is actually a DESR-XXXX machine
 *
 * @return 1 if DESR-XXXX machine; 0 if not
 */
extern int IsDESRMachine(void);
static inline int IsDESRMachineWithIODriver(const _io_driver *driver) { return IsDESRMachine(); }

/** check whether the PlayStation 2 is actually a TOOL DTL-T10000(H)
 *
 * @return 1 if DTL-T10000(H); 0 if not
 */
extern int IsT10K(void);
static inline int IsT10KWithIODriver(const _io_driver *driver) { return IsT10K(); }

/** gets the romname from the current ps2
 * 14 chars - doesnt set a null terminator
 *
 * @param romname buffer to hold romname (14 chars long)
 * @return pointer to buffer containing romname
 */
extern char *GetRomName(char *romname);
static inline char *GetRomNameWithIODriver(char *romname, const _io_driver *driver) { return GetRomName(romname); }

#ifdef __cplusplus
}
#endif

#endif /* __ROM_INFO_H__ */
