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

#include <rom0_info.h>

// We don't want kernel to depend newlib
#define NEWLIB_PORT_AWARE
#include "fileio.h"

#define defaultIODriver { (void *)fioOpen, fioClose, fioRead, FIO_O_RDONLY }

extern char g_RomName[];

#ifdef F__info_internals
/** stores romname of ps2 */
char g_RomName[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#ifdef F_GetRomNameWithIODriver
char *GetRomNameWithIODriver(char *romname, _io_driver *driver)
{
    int fd;

    fd = driver->open("rom0:ROMVER", driver->openFlags);
    driver->read(fd, romname, 14);
    driver->close(fd);
    return romname;
}
#endif

#ifdef F_GetRomName
char *GetRomName(char *romname)
{
    _io_driver driver = defaultIODriver;
    return GetRomNameWithIODriver(romname, &driver);
}
#endif

#ifdef F_IsDESRMachineWithIODriver
int IsDESRMachineWithIODriver(_io_driver *driver)
{
    int fd;

    fd = driver->open("rom0:PSXVER", driver->openFlags);
    if (fd > 0) {
        driver->close(fd);
        return 1;
    }

    return 0;
}
#endif

#ifdef F_IsDESRMachine
int IsDESRMachine(void)
{
    _io_driver driver = defaultIODriver;
    return IsDESRMachineWithIODriver(&driver);
}
#endif

#ifdef F_IsT10KWithIODriver
int IsT10KWithIODriver(_io_driver *driver)
{
    // only read in the romname the first time
    if (g_RomName[0] == 0)
        GetRomNameWithIODriver(g_RomName, driver);
    return (g_RomName[4] == 'T' && g_RomName[5] != 'Z') ? 1 : 0;
}
#endif

#ifdef F_IsT10K
int IsT10K(void)
{
    _io_driver driver = defaultIODriver;
    return IsT10KWithIODriver(&driver);
}
#endif
