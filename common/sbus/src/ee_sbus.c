/*

EE-specific code for PS2 SBUS library.

This file contains the glue, etc needed for an SBUS library on EE.

*/

#include <ps2_reg_defs.h>
#include <tamtypes.h>
#include <kernel.h>
#include "ps2_sbus.h"
#include "sbus_priv.h"

extern int __local_sbus_irq_handler(void);

void SBUS_check_intr(void)
{
    if(*R_EE_I_STAT & EE_I_STAT_SBUS)
    {
        __local_sbus_irq_handler();
        *R_EE_I_STAT |= EE_I_STAT_SBUS;
    }
}

#if 0

int SBUS_interrupt_remote(int irq)
{
    int oldi;

    M_SuspendIntr(&oldi);

    // wait for remote to clear "irq" bit
    while(SBUS_get_reg(PS2_SBUS_LR_FLAG) & (1 << irq));

    // if clk is high, bring it low
    if(SBUS_get_reg(PS2_SBUS_REG4) & 0x00000100) { SBUS_set_reg(PS2_SBUS_REG4, 0x00000100); }

    // set the "irq" bit for the SBUS interrupt.
    SBUS_set_reg(PS2_SBUS_LR_FLAG, (1 << irq));

    // cause the actual SBUS interrupt on remote CPU

    // clk high
    *R_EE_SBUS(4) = 0x00000100;
    // clk low
    *R_EE_SBUS(4) = 0x00000100;
    // clk high
    *R_EE_SBUS(4) = 0x00000100;
    // clk low
    *R_EE_SBUS(4) = 0x00000100;
    // clk high
    *R_EE_SBUS(4) = 0x00000100;
    // clk low
    *R_EE_SBUS(4) = 0x00000100;
    // clk high
    *R_EE_SBUS(4) = 0x00000100;
    // clk low
    *R_EE_SBUS(4) = 0x00000100;
    // clk high
    *R_EE_SBUS(4) = 0x00000100;
    // clk low, intr high
    *R_EE_SBUS(4) = 0x00040100;

    M_ResumeIntr(oldi);

    return(irq);
}
#else
int SBUS_interrupt_remote(int irq)
{
    int oldi;

    if(irq >= 32) { return(-1); }

    M_SuspendIntr(&oldi);

    // wait for remote to clear irq bit
    while(SBUS_get_reg(PS2_SBUS_MS_FLAG) & (1 << irq));

    // set the "irq" bit for the SBUS interrupt.
    SBUS_set_reg(PS2_SBUS_MS_FLAG, (1 << irq));

    SBUS_set_reg(PS2_SBUS_REG4, 0x00040100);
    SBUS_set_reg(PS2_SBUS_REG4, 0x00000100);

    M_ResumeIntr(oldi);

    return(irq);
}
#endif

// this is a hack but the address of the function that sets the actual low-level interrupt handler pointer.
void _SetIntcHandler(int irq, void *handler)
{
    int i_state, u_state;

    i_state = DI();
    u_state = (ee_kmode_enter() >> 3) & 3;

    ((void (*)(int, void *)) (0x80000700))(irq, handler);

    if(u_state) { ee_kmode_exit(); }
    if(i_state) { EI(); }
}
