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
# Standard ANSI string functions to complement the ASM funcs
*/

void _ps2sdk_alloc_init();
void _ps2sdk_alloc_deinit();
void _ps2sdk_stdio_init();
void _ps2sdk_stdio_deinit();

__attribute__((weak))
void _ps2sdk_libc_init()
{
    _ps2sdk_alloc_init();
    _ps2sdk_stdio_init();
}

__attribute__((weak))
void _ps2sdk_libc_deinit()
{
    _ps2sdk_stdio_deinit();
    _ps2sdk_alloc_deinit();
}
