#include <tamtypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <sysclib.h>
#include <sysmem.h>
#include <sys/stat.h>
#include <excepman.h>
#include <intrman.h>
#include <loadcore.h>
#include <thbase.h>
#include <ioman.h>
#include <ps2_debug.h>
#include <ps2_sbus.h>

#include "iopdebug.h"

#define SIF2_CMD_DBG_PUTS (1)
#define SIF2_CMD_DBG_EXC (2)
#define SIF2_CMD_DBG_CTRL (3)

extern int _iop_exception_state;

static IOP_RegFrame *_iop_ex_def_frame;
static IOP_RegFrame *_iop_ex_dbg_frame;

typedef struct st_SBUS_ControlCPU_Params
{
    u32 mode; // 0 = update register frame and release, 1 = control
    IOP_RegFrame reg_frame;
} SBUS_ControlCPU_Params;

extern int _iop_controlled;

// send a string to EE for output.
void sbus_tty_puts(const char *str)
{
    SIF2_send_cmd(SIF2_CMD_DBG_PUTS, (void *) str, strlen(str) + 1);
}

void signal_iop_exception(IOP_RegFrame *frame)
{
    SIF2_send_cmd(SIF2_CMD_DBG_EXC, (void *) frame, sizeof(IOP_RegFrame));
}

extern int tid;

void _sif2_cmd_dbg_control(SIF2_CmdPkt *cmd, void *param)
{
    SBUS_ControlCPU_Params *params;
    IOP_RegFrame *frame;

    params = (SBUS_ControlCPU_Params *) ((((u32) cmd) + sizeof(SIF2_CmdPkt)) | 0x00000000);

    if(params->mode == 1)
    {
        // wake up the controller thread.
        iWakeupThread(tid);
    }
    else
    {
        if(_iop_exception_state == 0) { return; }

        if(_iop_exception_state == 1) { frame = _iop_ex_def_frame; }
        else { frame = _iop_ex_dbg_frame; }

        // update the register frame and release the CPU.
        memcpy(frame, &params->reg_frame, sizeof(IOP_RegFrame));

        FlushIcache();
        FlushDcache();

        _iop_exception_state = 0;
        _iop_controlled = 0;
    }
}

void SBUS_dbg_init(void)
{
    iop_dbg_get_reg_frames(&_iop_ex_def_frame, &_iop_ex_dbg_frame);
    SIF2_set_cmd_handler(SIF2_CMD_DBG_CTRL, &_sif2_cmd_dbg_control, NULL);
}
