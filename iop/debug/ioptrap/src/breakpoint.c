/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "ioptrap.h"
#include <mipscopaccess.h>

void set_dba(u32 v)
{
    set_mips_cop_reg(0, 5, v);
}

void set_dbam(u32 v)
{
    set_mips_cop_reg(0, 9, v);
}

void set_dcic(u32 v)
{
    set_mips_cop_reg(0, 7, v);
}

u32 get_dba()
{
    u32 v;
    v = get_mips_cop_reg(0, 5);
    return v;
}

u32 get_dbam()
{
    u32 v;
    v = get_mips_cop_reg(0, 9);
    return v;
}

u32 get_dcic()
{
    u32 v;
    v = get_mips_cop_reg(0, 7);
    return v;
}
