/*

Common PS2 SBUS interrupt management API for both EE and IOP.

This file contains all common code for both EE and IOP SBUS interrupt management.

See also iop_sbus.c and ee_sbus.c for platform-specific glue functions.

*/

#include <tamtypes.h>
#include <ps2_reg_defs.h>
#include "ps2_sbus.h"
#include "sbus_priv.h"

// 32 local SBUS interrupt handlers.
SBUS_IrqHandler _sbus_irq_handlers[32];

static u32 _get_reg(u32 *reg)
{
    u32 v1, v2;

    v2 = *(vu32 *) (reg);
    do
    {
        v1 = v2;

#ifdef _EE
// EE needs 20 NOPs!
        __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
#endif

        v2 = *(vu32 *) reg;
    } while(v1 != v2);

    return(v1);
}

static void _set_reg(u32 *reg, u32 val)
{
    *(vu32 *) (reg) = val;
    _get_reg(reg);
}

// API call
u32 SBUS_get_reg(int reg_no) { return(_get_reg((u32 *) R_LOCAL_SBUS(reg_no))); }

// API call
void SBUS_set_reg(int reg_no, u32 val) { _set_reg((u32 *) R_LOCAL_SBUS(reg_no), val); }

// Interrupt handler for local SBUS interrupts
int __local_sbus_irq_handler(void)
{
    u32 flag;
    int irq;

    flag = SBUS_get_reg(PS2_SBUS_RL_FLAG);

    // loop through each bit of RL_FLAG calling the handler of any bit which is set.
    for(irq = 31; ((flag != 0) && (irq >= 0)); irq--)
    {
        if(flag & (1 << irq))
        {
        	if(_sbus_irq_handlers[irq].func)
        	{
                _sbus_irq_handlers[irq].func(irq, _sbus_irq_handlers[irq].param);
            }

            SBUS_set_reg(PS2_SBUS_RL_FLAG, (1 << irq));
            
            flag ^= (1 << irq);
        }
    }

	return(1);
}

// API call
void *SBUS_set_irq_handler(int irq, SBUS_IrqHandlerFunc func, void *param)
{
	int oldi = 0;
    void *rv = NULL;

    if(irq < 32)
    {
    	M_SuspendIntr(&oldi);
        rv = _sbus_irq_handlers[irq].func;
        _sbus_irq_handlers[irq].func = func;
        _sbus_irq_handlers[irq].param = param;
    	M_ResumeIntr(oldi);
	}

    // return pointer to old handler
	return(rv);
}

// API call
int SBUS_rem_irq_handler(int irq)
{
	int oldi = 0;

    if(irq < 32)
    {
        M_SuspendIntr(&oldi);
        _sbus_irq_handlers[irq].func = NULL;
        _sbus_irq_handlers[irq].param = NULL;
        M_ResumeIntr(oldi);
        return(0);
    }

    return(-1);
}

static int __sbus_init = 0;

int SBUS_init(void)
{
    int i;
    int old_irq_state;
    int old_intr_state;

    if(__sbus_init) { return(-1); }

    M_SuspendIntr(&old_intr_state);

    M_DisableIrq(PS2_IRQ_SBUS, &old_irq_state);

    M_ReleaseIrqHandler(PS2_IRQ_SBUS);

	for(i = 0; i < 32; i++)
	{
	    _sbus_irq_handlers[i].func = NULL;
	    _sbus_irq_handlers[i].param = NULL;
	}

	M_RegisterIrqHandler(PS2_IRQ_SBUS, (void *) __local_sbus_irq_handler, NULL);

	M_EnableIrq(PS2_IRQ_SBUS);

    M_ResumeIntr(old_intr_state);

    __sbus_init = 1;

	return(0);
}

int SBUS_deinit(void)
{
    int old_irq_state;
    int old_intr_state;

    if(!__sbus_init) { return(-1); }

    M_SuspendIntr(&old_intr_state);

    M_DisableIrq(PS2_IRQ_SBUS, &old_irq_state);
    M_ReleaseIrqHandler(PS2_IRQ_SBUS);

    M_ResumeIntr(old_intr_state);

    __sbus_init = 0;

	return(0);
}
