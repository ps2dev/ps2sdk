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

#include <tamtypes.h>
#include <sifcmd.h>
#include <kernel.h>
#include <sifrpc.h>
#include "sio.h"

int main()
{   
   SifInitRpc(0); 
/*
   init_scr();
   scr_printf("Hello, world!\n"); // hello world in the screen
*/
   printf("Hello, world!\n");
   nprintf("Hello, again, from Naplink RPC!\n");
   
   sio_init(115200, 0, 0, 0, 0);
   sio_printf("Hello from EE SIO!\n");

   /* Return to the bootloader or PS2 browser. */
   return 0;
}
