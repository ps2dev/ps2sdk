/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <errno.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <stdio.h>

#include "libpwroff.h"

static void poweroffCallback(void *arg)
{
    printf("\'hi\' from Poweroff callback!\n");

    //Close all files and unmount all partitions.
    //close(fd);

    //If you use PFS, close all files and unmount all partitions.
    //fileXioDevctl("pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0)

    //Shut down DEV9, if you used it.
    //while(fileXioDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0){};

    printf("Shutdown!");
    poweroffShutdown();
}

int main(int argc, char *argv[])
{
    int result;

    SifInitRpc(0);

    //Load modules.
    SifLoadFileInit();
    if ((result = SifLoadModule("host:poweroff.irx", 0, NULL)) < 0)
    {
        printf("Could not load \"host:poweroff.irx\": %d\n", result);
        return ENOENT;
    }

    printf("Welcome to the poweroff sample!\n\n");

    /*  Poweroff is a library that substitutes for the missing poweroff
        functionality in rom0:CDVDMAN, which is offered by newer CDVDMAN
        modules (i.e. sceCdPoweroff). Other than allowing the PlayStation 2
        to be switched off via software means, this is used to safeguard
        against the user switching off/resetting the console before data
        can be completely written to disk.
        */

    //Initialize poweroff library
    poweroffInit();

    //If necessary, change thread priority.
    //poweroffChangeThreadPriority(POWEROFF_THREAD_PRIORITY);

    //Set callback function
    poweroffSetCallback(&poweroffCallback, NULL);

    /*  The library has been initialized and a callback has been registered.
        The callback thread runs at priority POWEROFF_THREAD_PRIORITY by default,
        which can be changed with a call to poweroffChangeThreadPriority.
        */

    SleepThread();

    SifExitRpc();

    return 0;
}
