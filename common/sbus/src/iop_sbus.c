/*

IOP-specifc code for PS2 SBUS library.

This file contains the IRX glue, etc needed for an SBUS driver on IOP.

*/

#include <tamtypes.h>
#include <ps2_sbus.h>
#include <loadcore.h>
#include <stdio.h>
#include <thbase.h>
#include "sbus_priv.h"

IRX_ID("sbus_driver", 1, 0);

extern struct irx_export_table _exp_sbus;

extern int __local_sbus_irq_handler(void);

void SBUS_check_intr(void)
{
    if(!(*R_IOP_I_MASK & IOP_I_STAT_SBUS)) { *R_IOP_I_MASK |= IOP_I_STAT_SBUS; }

    if(*R_IOP_I_STAT & IOP_I_STAT_SBUS)
    {
        __local_sbus_irq_handler();
        *R_IOP_I_STAT &= ~(IOP_I_STAT_SBUS);
    }
}

int SBUS_interrupt_remote(int irq)
{
    // wait for remote to clear "irq" bit
    while(SBUS_get_reg(PS2_SBUS_LR_FLAG) & (1 << irq));

    // set the SBUS "irq" bit for remote interrupt.
    SBUS_set_reg(PS2_SBUS_LR_FLAG, 1 << irq);

    // cause SBUS interrupt on remote.
    *R_IOP_IRQ_CTRL |= (1 << PS2_IRQ_SBUS);

    // clear the SBUS interrupt on remote.
    *R_IOP_IRQ_CTRL &= ~(1 << PS2_IRQ_SBUS);

    return(irq);
}

int _start(int argc, char *argv[])
{
    if(SBUS_init() != 0) { printf("IOP: SBUS_init() failed!\n"); return(1); }
    if(SIF2_init() != 0) { printf("IOP: SIF2_init() failed!\n"); return(1); }
    if(SIF2_init_cmd() != 0) { printf("IOP: SIF2_init_cmd() failed!\n"); return(1); }
    if(RegisterLibraryEntries(&_exp_sbus) != 0) { printf("IOP: Error registering SBUS library!\n"); return(1); }

    printf("IOP: SBUS driver loaded!\n");

    return(0);
}

int _stop(int argc, char *argv[])
{
    ReleaseLibraryEntries(&_exp_sbus);

    SBUS_deinit();
    SIF2_deinit();
    return(1);
}
