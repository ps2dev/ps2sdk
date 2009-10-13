#include <kernel.h>
#include <ee_debug.h>
#include <string.h>

#include "eedebug_defs.h"

// the stacks for the Level 1 and Level 2 exception handlers.
u128 __ee_ex_l1_stack[_EX_L1_STACK_SIZE / 16] __attribute__ ((aligned(128)));
u128 __ee_ex_l2_stack[_EX_L2_STACK_SIZE / 16] __attribute__ ((aligned(128)));

// the register frames used by the Level 1 and Level 2 exception handlers to store the register states.
EE_RegFrame __ee_ex_l1_frame __attribute__ ((aligned(128)));
EE_RegFrame __ee_ex_l2_frame __attribute__ ((aligned(128)));

// used to preserve the original "debug" exception vector.
static u32 __saved_dbg_ex_vector[0x80 / 4] = { 0 };

// used to preserve the original "Level 1" exception handlers.
static void *_old_l1_handlers[16] = { 0 };

static int _installed_levels = 0;

static EE_ExceptionHandler *ee_level1_exception_handlers[16] = { 0 };
static EE_ExceptionHandler *ee_level2_exception_handlers[4] = { 0 };

void _def_ee_ex_handler(EE_RegFrame *frame)
{
    while(1);    
}

void ee_level1_ex_dispatcher(EE_RegFrame *frame)
{
    int excode = M_EE_GET_CAUSE_EXCODE(frame->cause);
    EE_ExceptionHandler *handler = ee_level1_exception_handlers[excode];

    if(handler)
    {
        handler(frame);
    }
}

void ee_level2_ex_dispatcher(EE_RegFrame *frame)
{
    int exc2 = M_EE_GET_CAUSE_EXC2(frame->cause);
    EE_ExceptionHandler *handler = ee_level2_exception_handlers[exc2];

    if(handler)
    {
        handler(frame);
    }
}

EE_ExceptionHandler *ee_dbg_get_level1_handler(int cause)
{
    if((cause < 0) || (cause > 13)) { return(NULL); }

    return(ee_level1_exception_handlers[cause]);
}

EE_ExceptionHandler *ee_dbg_set_level1_handler(int cause, EE_ExceptionHandler *handler)
{
    EE_ExceptionHandler *old_handler;
    u32 oldintr;

    if((cause < 0) || (cause > 13)) { return(NULL); }

    oldintr = DIntr();

    old_handler = ee_level1_exception_handlers[cause];
    ee_level1_exception_handlers[cause] = handler;

    if(oldintr) { EIntr(); }

    return(old_handler);
}

EE_ExceptionHandler *ee_dbg_get_level2_handler(int cause)
{
    if((cause < 0) || (cause > 3)) { return(NULL); }

    return(ee_level2_exception_handlers[cause]);
}

EE_ExceptionHandler *ee_dbg_set_level2_handler(int cause, EE_ExceptionHandler *handler)
{
    EE_ExceptionHandler *old_handler;
    u32 oldintr;

    if((cause < 0) || (cause > 3)) { return(NULL); }

    oldintr = DIntr();

    old_handler = ee_level2_exception_handlers[cause];
    ee_level2_exception_handlers[cause] = handler;

    if(oldintr) { EIntr(); }

    return(old_handler);
}

extern void _ee_dbg_set_bpda(u32, u32, u32);
extern void _ee_dbg_set_bpdv(u32, u32, u32);
extern void _ee_dbg_set_bpx(u32, u32, u32);

// ex: ee_dbg_set_bpr(&var, 0xFFFFFFFF, (EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE));
void ee_dbg_set_bpr(u32 addr, u32 mask, u32 opmode_mask) { _ee_dbg_set_bpda(addr, mask, EE_BPC_DRE | opmode_mask); }

// ex: ee_dbg_set_bpw(&var, 0xFFFFFFFF, (EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE));
void ee_dbg_set_bpw(u32 addr, u32 mask, u32 opmode_mask) { _ee_dbg_set_bpda(addr, mask, EE_BPC_DWE | opmode_mask); }

// ex: ee_dbg_set_bprw(&var, 0xFFFFFFFF, (EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE));
void ee_dbg_set_bprw(u32 addr, u32 mask, u32 opmode_mask) { _ee_dbg_set_bpda(addr, mask, EE_BPC_DRE | EE_BPC_DWE | opmode_mask); }

// ex: ee_dbg_set_bpv(0xDEADBEEF, 0xFFFFFFFF, (EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE));
void ee_dbg_set_bpv(u32 value, u32 mask, u32 opmode_mask) { _ee_dbg_set_bpdv(value, mask, opmode_mask); }

// ex: ee_dbg_set_bpx(&func, 0xFFFFFFFF, (EE_BPC_IUE | EE_BPC_ISE | EE_BPC_IKE | EE_BPC_IXE));
void ee_dbg_set_bpx(u32 addr, u32 mask, u32 opmode_mask) { _ee_dbg_set_bpx(addr, mask, opmode_mask); }

void ee_dbg_clr_bpda(void)
{
    u32 bpc = ee_dbg_get_bpc();

    bpc &= ~(EE_BPC_DWE | EE_BPC_DRE);

    if(!(bpc & (EE_BPC_DVE)))
    {
        // if Data Value breakpoint not enabled, disable all Data bp bits.
        bpc &= ~(EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE | EE_BPC_DTE);
    }

    ee_dbg_set_bpc(bpc);
}

void ee_dbg_clr_bpdv(void)
{
    u32 bpc = ee_dbg_get_bpc();

    bpc &= ~(EE_BPC_DVE);

    if(!(bpc & (EE_BPC_DWE | EE_BPC_DRE)))
    {
        // if Data Read or Data Write breakpoints not enabled, disable all Data bp bits.
        bpc &= ~(EE_BPC_DUE | EE_BPC_DSE | EE_BPC_DKE | EE_BPC_DXE | EE_BPC_DTE);
    }

    ee_dbg_set_bpc(bpc);
}

void ee_dbg_clr_bpx(void)
{
    u32 bpc = ee_dbg_get_bpc();
    bpc &= ~(EE_BPC_IXE | EE_BPC_IUE | EE_BPC_ISE | EE_BPC_IKE | EE_BPC_IXE | EE_BPC_ITE);
    ee_dbg_set_bpc(bpc);
}

void ee_dbg_clr_bps(void)
{
    ee_dbg_set_bpc(EE_BPC_BED);
    ee_dbg_set_iab(0);
    ee_dbg_set_iabm(0);
    ee_dbg_set_dab(0);
    ee_dbg_set_dabm(0);
    ee_dbg_set_dvb(0);
    ee_dbg_set_dvbm(0);
}

extern void __ee_level1_ex_vector(void);
extern void __ee_level2_ex_vector(void);

int ee_dbg_install(int levels)
{
    u32 oldintr, oldop;
    int i;

    if(_installed_levels & levels)
    {
        return(-1);
    }

    if(levels & 1)
    {
        for(i = 0; i < 16; i++) { ee_level1_exception_handlers[i] = NULL; }
    }

    if(levels & 2)
    {
        for(i = 0; i < 4; i++) { ee_level2_exception_handlers[i] = NULL; }

        oldintr = DIntr();
        oldop = ee_set_opmode(0);

        ee_dbg_clr_bps();
        
        // save the original level 2 debug exception vector.
        memcpy(&__saved_dbg_ex_vector, (void *) (0x80000100), 0x80);

        // replace the level 2 debug exception vector with our own
        memcpy((void *) (0x80000100), &__ee_level2_ex_vector, 32);

        ee_set_opmode(oldop);
        if(oldintr) { EIntr(); }
    }

    if(levels & 1)
    {
        // redirect desirable "Level 1" exceptions to our level 1 handler.
        for(i = 1; i <= 3; i++)
        {
            _old_l1_handlers[i] = GetExceptionHandler(i);
            SetVTLBRefillHandler(i, __ee_level1_ex_vector);
        }

        for(i = 4; i <= 7; i++)
        {
            _old_l1_handlers[i] = GetExceptionHandler(i);
            SetVCommonHandler(i, __ee_level1_ex_vector);
        }

        for(i = 10; i <= 13; i++)
        {
            _old_l1_handlers[i] = GetExceptionHandler(i);
            SetVCommonHandler(i, __ee_level1_ex_vector);
        }
    }

    FlushCache(0);
    FlushCache(2);

    _installed_levels |= levels;

    return(0);
}

int ee_dbg_remove(int levels)
{
    u32 oldintr, oldop;
    int i;

    if((levels < 1) || (levels > 3)) { return(-1); }

    if(!(_installed_levels & levels)) { return(-1); }

    if((levels & _installed_levels) & 2)
    {
        oldintr = DIntr();
        oldop = ee_set_opmode(0);

        ee_dbg_clr_bps();

        // restore the original debug exception vector.
        memcpy((void *) (0x80000100), &__saved_dbg_ex_vector, sizeof(__saved_dbg_ex_vector));

        ee_set_opmode(oldop);
        if(oldintr) { EIntr(); }
        
        FlushCache(0);
        FlushCache(2);

        _installed_levels &= 1;
    }

    if((levels & _installed_levels) & 1)
    {
        // restore the exception handlers that we previously hooked.
        for(i = 1; i <= 3; i++)
        {
            if(_old_l1_handlers[i] != NULL)
            {
                SetVTLBRefillHandler(i, _old_l1_handlers[i]);
                _old_l1_handlers[i] = NULL;
            }
        }
        for(i = 4; i <= 7; i++)
        {
            if(_old_l1_handlers[i] != NULL)
            {
                SetVCommonHandler(i, _old_l1_handlers[i]);
                _old_l1_handlers[i] = NULL;
            }
        }
        for(i = 10; i <= 13; i++)
        {
            if(_old_l1_handlers[i] != NULL)
            {
                SetVCommonHandler(i, _old_l1_handlers[i]);
                _old_l1_handlers[i] = NULL;
            }
        }
        
        FlushCache(0);
        FlushCache(2);

        _installed_levels &= 2;
    }

    return(0);
}
