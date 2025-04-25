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
 * DECI2
 */

#include "kernel.h"

// actual size?
static char userarea[56];

int sceDeci2Open(unsigned short protocol, void *opt,
                 void (*handler)(int event, int param, void *opt))
{

    u32 args[4];

    args[0] = protocol & 0xffff;
    args[1] = (u32)opt;
    args[2] = (u32)handler;
    args[3] = (u32)UNCACHED_SEG(userarea);

    return Deci2Call(1, args);
}

int sceDeci2Close(int s)
{
    u32 args[4];

    args[0] = s;

    return Deci2Call(2, args);
}

int sceDeci2ReqSend(int s, char dest)
{
    u32 args[4];

    args[0] = s;
    args[1] = dest;

    return Deci2Call(3, args);
}

void sceDeci2Poll(int s)
{
    u32 args[4];

    args[0] = s;

    Deci2Call(4, args);
}

int sceDeci2ExRecv(int s, void *buf, unsigned short len)
{
    u32 args[4];

    args[0] = s;
    args[1] = (u32)buf;
    args[2] = len & 0xffff;

    return Deci2Call(-5, args);
}

int sceDeci2ExSend(int s, void *buf, unsigned short len)
{
    u32 args[4];

    args[0] = s;
    args[1] = (u32)buf;
    args[2] = len & 0xffff;

    return Deci2Call(-6, args);
}

int sceDeci2ExReqSend(int s, char dest)
{
    u32 args[4];

    args[0] = s;
    args[1] = dest;

    return Deci2Call(-7, args);
}

int sceDeci2ExLock(int s)
{
    u32 args[4];

    args[0] = s;

    return Deci2Call(-8, args);
}

int sceDeci2ExUnLock(int s)
{
    u32 args[4];

    args[0] = s;

    return Deci2Call(-9, args);
}

int kputs(char *s)
{
    u32 args[4];

    args[0] = (u32)s;

    return Deci2Call(16, args);
}
