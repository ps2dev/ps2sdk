/*

Common PS2 SIF management API for both EE and IOP.

This file contains all common code for both EE and IOP SIF management.

*/

#include <tamtypes.h>
#include <ps2_reg_defs.h>
#include "ps2_sbus.h"
#include "sbus_priv.h"

#define SBUS_IRQ_XFER (30)
#define SBUS_IRQ_EXEC (31)

// SIF2 Commands
#define SIF2_CMD_SET_XFER_PARAMS (0)

// Transfer Types
#define SIF2_XFER_RECV  (0)
#define SIF2_XFER_SEND  (1)

static u32 _sif2_req_type = 0;
static u32 _sif2_req_addr = 0;
static u32 _sif2_req_size = 0;

static int __sif2_cmd_inited = 0;

static SIF2_CmdHandler _sif2_cmd_handlers[SIF2_MAX_CMD_HANDLERS];

// 512 byte command buffer(aligned 16 bytes)
static u32 _sif2_dma_cmd_buf[128] __attribute__ ((aligned(128)));

int SIF2_set_cmd_handler(int cid, SIF2_CmdHandlerFunc func, void *param)
{
    if((cid < 0) || (cid >= SIF2_MAX_CMD_HANDLERS)) { return(-1); }

    // return an error if a handler already exists(you must remove it first)
    if(_sif2_cmd_handlers[cid].func) { return(-2); }

    _sif2_cmd_handlers[cid].func = func;
    _sif2_cmd_handlers[cid].param = param;

    return(0);
}

int SIF2_rem_cmd_handler(int cid)
{
    if((cid < 0) || (cid >= SIF2_MAX_CMD_HANDLERS)) { return(-1); }

    _sif2_cmd_handlers[cid].func = NULL;
    _sif2_cmd_handlers[cid].param = NULL;

    return(0);
}

// called when remote requests local to setup a DMA transfer to/from remote.
void _sif2_irq_xfer(void)
{
    if(_sif2_req_type == SIF2_XFER_RECV)
    {
        // remote has requested us to receive a transfer from remote.
        SIF2_set_dma(_sif2_req_addr, _sif2_req_size, PS2_DMA_TO_MEM);
    }
    else if(_sif2_req_type == SIF2_XFER_SEND)
    {
        // remote has requested us to send a transfer to remote.
        SIF2_set_dma(_sif2_req_addr, _sif2_req_size, PS2_DMA_FROM_MEM);
    }
}

void _sif2_irq_exec(void)
{
    int cid;
    SIF2_CmdPkt *pkt;

    // remote is waiting to DMA the command parameters

    // set up a "receive" DMA transfer to the command buffer.
    SIF2_set_dma(((u32) &_sif2_dma_cmd_buf), sizeof(_sif2_dma_cmd_buf), PS2_DMA_TO_MEM);

    // wait for transfer to complete.
    SIF2_sync_dma();

#ifdef _EE
    pkt = (SIF2_CmdPkt *) (((u32) &_sif2_dma_cmd_buf) | 0x20000000);
#else
    pkt = (SIF2_CmdPkt *) (((u32) &_sif2_dma_cmd_buf));
#endif

    cid = pkt->cid;

    if(_sif2_cmd_handlers[cid].func)
    {
        _sif2_cmd_handlers[cid].func(pkt, _sif2_cmd_handlers[cid].param);
    }
    else
    {
#ifdef _EE
        sio_printf("unknown command: %d!\n", cid);
#endif
    }
}

void WriteBackDCache(void *, u32);

void SIF2_send_cmd(u32 cid, void *extra, int extra_size)
{
    SIF2_CmdPkt *cmd = (SIF2_CmdPkt *) _sif2_dma_cmd_buf;

    cmd->cid = cid;
    cmd->size = sizeof(SIF2_CmdPkt);
    cmd->extra = 0;
    cmd->extra_size = 0;

    if(extra)
    {
        if(extra_size <= (sizeof(_sif2_dma_cmd_buf) - sizeof(SIF2_CmdPkt)))
        {
            memcpy((u32 *) (((u32) &_sif2_dma_cmd_buf) + sizeof(SIF2_CmdPkt)), extra, extra_size);
            cmd->size += extra_size;
        }
        else
        {
            cmd->extra = (u32) extra;
            cmd->extra_size = extra_size;
        }
    }

#ifdef _EE
	WriteBackDCache(_sif2_dma_cmd_buf, sizeof(_sif2_dma_cmd_buf));
#endif

    // set up the DMA transfer to send the cmd packet to remote.
    SIF2_set_dma((u32) cmd, sizeof(_sif2_dma_cmd_buf), PS2_DMA_FROM_MEM);

    // interrupt the remote CPU with "EXEC".
    SBUS_interrupt_remote(SBUS_IRQ_EXEC);

    // wait for transfer to complete.
    SIF2_sync_dma();

    if(cmd->extra)
    {
#ifdef _EE
    	WriteBackDCache((void *) cmd->extra, cmd->extra_size);
#endif

        // set up the DMA transfer to send the extra data to remote.
        SIF2_set_dma((u32) cmd->extra, cmd->extra_size, PS2_DMA_FROM_MEM);

        // wait for transfer to complete.
        SIF2_sync_dma();
    }
}

void _sif2_cmd_set_req_params(SIF2_CmdPkt *cmd, void *param)
{
    u32 *p = &((u32 *) cmd)[4];

    _sif2_req_type = p[0];
    _sif2_req_addr = p[1];
    _sif2_req_size = p[2];
}

int SIF2_init_cmd(void)
{
    if(__sif2_cmd_inited) { return(-1); }

    _sif2_req_type = 0;
    _sif2_req_addr = 0;
    _sif2_req_size = 0;

    // init SIF2, if not already
    SIF2_init();

    SBUS_set_irq_handler(SBUS_IRQ_XFER, (SBUS_IrqHandlerFunc) _sif2_irq_xfer, NULL);
    SBUS_set_irq_handler(SBUS_IRQ_EXEC, (SBUS_IrqHandlerFunc) _sif2_irq_exec, NULL);

    SIF2_set_cmd_handler(SIF2_CMD_SET_XFER_PARAMS, &_sif2_cmd_set_req_params, NULL);

    __sif2_cmd_inited = 1;
    return(0);
}
