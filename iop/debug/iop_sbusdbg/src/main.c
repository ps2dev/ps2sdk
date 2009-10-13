#include <tamtypes.h>
#include <stdio.h>
#include <loadcore.h>
#include <thbase.h>
#include <iop_cop0_defs.h>
#include <ps2_sbus.h>
#include <iopdebug.h>
#include <excepman.h>
#include "sbus_tty.h"

IRX_ID("sbusdebug", 1, 0);

volatile int _iop_controlled = 0;
volatile int _iop_exception_state = 0;

extern void signal_iop_exception(IOP_RegFrame *frame);

// this is called by the IOP debug/exception handler
void _iop_ex_handler(IOP_RegFrame *frame)
{
    int excode = M_IOP_GET_CAUSE_EXCODE(frame->cause);

    if(excode == IOP_EXCEPTION_HDB)
    {
        _iop_exception_state = 2;
    }
    else
    {
        _iop_exception_state = 1;
    }

    _iop_controlled = 1;

    // signal to the EE that an IOP exception occured.
    signal_iop_exception(frame);

    // wait for EE to send a "IOP control" command and release the IOP.
    while(_iop_controlled)
    {
        // since we're spinning in an interrupt-disabled state, we have to manually
        //  poll for SBUS interrupts from EE, otherwise we'll never get released!
        SBUS_check_intr();
    }
}

int tid;

extern void soft_break(void);

void _controller_thread(void)
{
    while(1)
    {
//        printf("IOP: Controller thread going to sleep(%d)...\n", _iop_exception_state);
        SleepThread();
//        printf("IOP: Controller thread woke up!\n");
        soft_break();
    }
}

void create_th(void)
{
	iop_thread_t param;

	param.attr = TH_C;
	param.thread = (void *) _controller_thread;
	param.priority = 9; // highest possible priority
	param.stacksize = 512; // tiny stack, not much is needed!
	param.option = 0;

	tid = CreateThread(&param);
	StartThread(tid, 0);
}

extern void SBUS_dbg_init(void);

int _start(int argc, char *argv[])
{
    int i;

    for(i = 2; i <= 7; i++)
    {
        iop_dbg_set_handler(i, (IOP_ExceptionHandler *) _iop_ex_handler);
    }

    for(i = 9; i <= 15; i++)
    {
        iop_dbg_set_handler(i, (IOP_ExceptionHandler *) _iop_ex_handler);
    }

    create_th();

    if(sbus_tty_init() != 0)
    {
        printf("Failed initializing SBUS TTY system!\n");
        return(1);
    }

    SBUS_dbg_init();

    FlushIcache();
    FlushDcache();

    printf("IOP SBUS Debug installed!\n");

    return(0); // return resident!
}
