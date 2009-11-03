/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: iop_debug.c $
#
# IOPDEBUG - IOP debugging library.
#
# iop_debug.c - high-level IOP debugging library code.
#
*/

#include <tamtypes.h>
#include <excepman.h>
#include <iop_cop0_defs.h>
#include <ps2_debug.h>

#include "iopdebug.h"
#include "iopdebug_priv.h"
#include "iopdebug_defs.h"

u32 _iop_ex_def_stack[_EX_DEF_STACK_SIZE / 4];
u32 _iop_ex_dbg_stack[_EX_DBG_STACK_SIZE / 4];

IOP_RegFrame _iop_ex_def_frame;
IOP_RegFrame _iop_ex_dbg_frame;

static int _is_iop_dbg_installed = 0;

static void *orig_iop_def_ex_handler = NULL;
static void *orig_iop_break_ex_handler = NULL;
static void *orig_iop_dbg_ex_handler = NULL;

static IOP_ExceptionHandler *iop_exception_handlers[16];

IOP_ExceptionHandler *iop_dbg_get_handler(int cause)
{
    if((cause < 0) || (cause > 15)) { return(NULL); }

    return(iop_exception_handlers[cause]);
}

IOP_ExceptionHandler *iop_dbg_set_handler(int cause, IOP_ExceptionHandler *handler)
{
    IOP_ExceptionHandler *old_handler;
    u32 old_state;

    if((cause < 0) || (cause > 15)) { return(NULL); }

    EnterCritical(&old_state);

    old_handler = iop_exception_handlers[cause];
    iop_exception_handlers[cause] = handler;

    ExitCritical(old_state);

    return(old_handler);
}

// this is called by the IOP debug/exception handlers
void iop_exception_dispatcher(IOP_RegFrame *frame)
{
    int excode = M_IOP_GET_CAUSE_EXCODE(frame->cause);
    IOP_ExceptionHandler *handler = iop_exception_handlers[excode];

    if(handler)
    {
        handler(frame);
    }
}

extern void _iop_dbg_set_bpda(u32 addr, u32 mask, u32 user_mask);
extern void _iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask);

// ex: iop_dbg_set_bpr(&var, 0xFFFFFFFF, (IOP_DCIC_UD | IOP_DCIC_KD));
void iop_dbg_set_bpr(u32 addr, u32 mask, u32 user_mask) { _iop_dbg_set_bpda(addr, mask, IOP_DCIC_DR | user_mask); }

// ex: iop_dbg_set_bpw(&var, 0xFFFFFFFF, (IOP_DCIC_UD | IOP_DCIC_KD));
void iop_dbg_set_bpw(u32 addr, u32 mask, u32 user_mask) { _iop_dbg_set_bpda(addr, mask, IOP_DCIC_DW | user_mask); }

// ex: iop_dbg_set_bpx(&func, 0xFFFFFFFF, (IOP_DCIC_UD | IOP_DCIC_KD));
void iop_dbg_set_bpx(u32 addr, u32 mask, u32 user_mask) { _iop_dbg_set_bpx(addr, mask, user_mask); }

void iop_dbg_clr_bpda(void)
{
    u32 dcic = iop_dbg_get_dcic();
    dcic &= ~(IOP_DCIC_DW | IOP_DCIC_DR | IOP_DCIC_DAE);
    if(!(dcic & IOP_DCIC_PCE)) { dcic = 0; }
    iop_dbg_set_dcic(dcic);
}

void iop_dbg_clr_bpx(void)
{
    u32 dcic = iop_dbg_get_dcic();
    dcic &= ~(IOP_DCIC_PCE);
    if(!(dcic & IOP_DCIC_DAE)) { dcic = 0; }
    iop_dbg_set_dcic(dcic);
}

void iop_dbg_clr_bps(void)
{
    iop_dbg_set_dcic(0);
    iop_dbg_set_bda(0);
    iop_dbg_set_bdam(0);
    iop_dbg_set_bpc(0);
    iop_dbg_set_bpcm(0);
}

extern void _iop_ex_def_handler(void);
extern void _iop_ex_dbg_handler(void);
extern void _iop_ex_break_handler(void);

int iop_dbg_install(void)
{
    int i;

    // if already installed, return with an error.
    if(_is_iop_dbg_installed) { return(-1); }

    // initialize our exception handler pointers.
    for(i = 0; i < 16; i++)
    {
        iop_exception_handlers[i] = NULL;
    }

    // todo: preserve original exception handlers here!
    // This will require using "EXCEPMAN" export 3..

    // register our "default" exception handler.
    if(RegisterDefaultExceptionHandler((exception_handler_t) _iop_ex_def_handler) < 0)
    {
        orig_iop_def_ex_handler = NULL;
        return(-2);
    }

    // register our "BREAK" instruction exception handler.
    if(RegisterPriorityExceptionHandler(IOP_EXCEPTION_BP, 0, (exception_handler_t) _iop_ex_break_handler) < 0)
    {
        RegisterDefaultExceptionHandler((exception_handler_t) orig_iop_def_ex_handler);
        orig_iop_def_ex_handler = NULL;
        orig_iop_break_ex_handler = NULL;
        return(-3);
    }

    // register our "Hardware DeBug"(aka "Hardware BreakPoint") exception handler.    
    if(RegisterPriorityExceptionHandler(IOP_EXCEPTION_HDB, 0, (exception_handler_t) _iop_ex_dbg_handler) < 0)
    {
        RegisterDefaultExceptionHandler((exception_handler_t) orig_iop_def_ex_handler);
        orig_iop_def_ex_handler = NULL;
        
        RegisterPriorityExceptionHandler(IOP_EXCEPTION_BP, 0, (exception_handler_t) orig_iop_break_ex_handler);
        orig_iop_break_ex_handler = NULL;

        orig_iop_dbg_ex_handler = NULL;
        return(-4);
    }

    _is_iop_dbg_installed = 1;
    return(0);
}

int iop_dbg_remove(void)
{
    // if not installed, return with an error.
    if(!_is_iop_dbg_installed) { return(-1); }

    if(orig_iop_def_ex_handler)
    {
        RegisterDefaultExceptionHandler((exception_handler_t) orig_iop_def_ex_handler);
        orig_iop_def_ex_handler = NULL;
    }
    
    if(orig_iop_break_ex_handler)
    {
        RegisterPriorityExceptionHandler(IOP_EXCEPTION_BP, 0, (exception_handler_t) orig_iop_break_ex_handler);
        orig_iop_break_ex_handler = NULL;
    }

    if(orig_iop_dbg_ex_handler)
    {
        RegisterPriorityExceptionHandler(IOP_EXCEPTION_HDB, 0, (exception_handler_t) orig_iop_dbg_ex_handler);
        orig_iop_dbg_ex_handler = NULL;
    }

    _is_iop_dbg_installed = 0;
    return(0);
}

void iop_dbg_get_reg_frames(IOP_RegFrame **def_frame_ptr, IOP_RegFrame **dbg_frame_ptr)
{
    if(def_frame_ptr) { *def_frame_ptr = &_iop_ex_def_frame; }
    if(dbg_frame_ptr) { *dbg_frame_ptr = &_iop_ex_dbg_frame; }
}

