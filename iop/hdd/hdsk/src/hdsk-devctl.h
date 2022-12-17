/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2SDK_HDSK_IOCTL_H__
#define __PS2SDK_HDSK_IOCTL_H__

enum HDSK_DEVCTL {
    HDSK_DEVCTL_GET_FREE = 0,
    HDSK_DEVCTL_GET_HDD_STAT,
    HDSK_DEVCTL_START,
    HDSK_DEVCTL_WAIT,
    HDSK_DEVCTL_POLL,
    HDSK_DEVCTL_GET_STATUS,
    HDSK_DEVCTL_STOP,
    HDSK_DEVCTL_GET_PROGRESS,
};

struct hdskStat
{
    u32 free;
    u32 total;
};

#endif
