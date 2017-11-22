/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# The erl-tags support
*/

#include <erl.h>

#ifdef _XINPUT
char * erl_id = "libinputx";
#else
char * erl_id = "libinput";
#endif

char * erl_dependancies[] = {
    "libc",
#ifdef _XINPUT
    "libmtap",
#endif
    "libpad",
    0
};
