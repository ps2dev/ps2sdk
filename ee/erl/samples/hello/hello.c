/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Hello sample
*/

#include <stdio.h>
#include <erl.h>

char * erl_id = "hello";
char * erl_dependancies[] = {
    "libc",
    0
};

int main()
{   
    printf("Hello world!\n");
    
    return 0;
}
