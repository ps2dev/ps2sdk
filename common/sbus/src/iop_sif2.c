/*

This file contains IOP-specific SIF-related code.

*/

#include <tamtypes.h>
#include <ps2_reg_defs.h>
#include "iop_regs.h"
#include "ps2_sbus.h"
#include <sbus_priv.h>

static int _sif2_inited = 0;

static u32 _sif2_xfer_addr = 0;
static u32 _sif2_xfer_size = 0;
static u32 _sif2_xfer_attr = 0;
static u32 _sif2_xfer_chunk_size = 0;
static SIF2_TransferCbFunc _sif2_xfer_cb = NULL;

extern void do_debug(u32 val);

int SIF2_RestartDma(void);

void SIF2_sync_dma(void)
{
    while(1)
    {
        if(!(*R_IOP_D2_CHCR & IOP_CHCR_TR))
        {
            if((_sif2_xfer_addr != 0) && (_sif2_xfer_size > 0))
            {
                _sif2_xfer_addr += _sif2_xfer_chunk_size;
                _sif2_xfer_size -= _sif2_xfer_chunk_size;
            }

            if((_sif2_xfer_addr != 0) && (_sif2_xfer_size > 0))
            {
                SIF2_RestartDma();
            }
            else
            {
                _sif2_xfer_addr = 0;
                _sif2_xfer_size = 0;
                if(_sif2_xfer_cb) { _sif2_xfer_cb(); }
                return;
            }
        }
    }
}

int SIF2_RestartDma(void)
{
    volatile register u32 d;
    int bc, bs;

    if(!(*R_LOCAL_SBUS(PS2_SBUS_REG4) & 0x80)) { *R_LOCAL_SBUS(PS2_SBUS_REG4) = 0x80; }

    *R_IOP_D2_CHCR = 0;
    d = *R_IOP_D2_CHCR;

    *R_IOP_D2_MADR = (_sif2_xfer_addr & 0x00FFFFFF);

    _sif2_xfer_chunk_size = (_sif2_xfer_size > SIF2_XFER_CHUNK_SIZE) ? SIF2_XFER_CHUNK_SIZE : _sif2_xfer_size;

    bs = ((_sif2_xfer_chunk_size + 3) / 4);
    if(bs > 32) { bs = 32; }

    bc = (_sif2_xfer_chunk_size + ((bs * 4) - 1)) / (bs * 4);

    *R_IOP_D2_BCR_BS = bs;
    *R_IOP_D2_BCR_BC = bc;
    *R_IOP_D2_CHCR = _sif2_xfer_attr;
    d = *R_IOP_D2_CHCR;

    return(0);
}

int SIF2_set_dma(u32 addr, u32 size, u32 attr)
{
    // just a precaution...
    while(*R_IOP_D2_CHCR & IOP_CHCR_TR);

    size = ((size + 3) / 4) * 4;

    _sif2_xfer_addr = addr;
    _sif2_xfer_size = size;

    if(!(attr & PS2_DMA_FROM_MEM)) { attr |= IOP_CHCR_30; }

    _sif2_xfer_attr = IOP_CHCR_TR | attr;

    SIF2_RestartDma();
    return(0);
}

#if 0
int _sif2_intr_handler(void)
{
    u32 chcr = *R_IOP_D2_CHCR;

    // is transfer stopped?
    if(!(chcr & IOP_CHCR_TR))
    {
        if((_sif2_xfer_addr != 0) && (_sif2_xfer_size > 0))
        {
            _sif2_xfer_addr += _sif2_xfer_chunk_size;
            _sif2_xfer_size -= _sif2_xfer_chunk_size;
        }

        if((_sif2_xfer_addr != 0) && (_sif2_xfer_size > 0))
        {
            SIF2_RestartDma();
        }
        else
        {
            _sif2_xfer_addr = 0;
            _sif2_xfer_size = 0;
            if(_sif2_xfer_cb) { _sif2_xfer_cb(); }
        }
    }

    return(1);
}
#endif

int SIF2_init(void)
{
    volatile register u32 d;
    int oldStat, old_irq;

    if(_sif2_inited) { return(-1); }

    CpuSuspendIntr(&oldStat);
    *R_IOP_D2_CHCR = 0;

    // Enable DMA CH2
    if(!(*R_IOP_DPCR & 0x0800))
    {
        *R_IOP_DPCR |= 0x0800;
        d = *R_IOP_DPCR;
    }

    DisableIntr(IOP_IRQ_DMA2, &old_irq);

#if 0
    RegisterIntrHandler(IOP_IRQ_DMA2, 1, (void *) &_sif2_intr_handler, NULL);

    EnableIntr(IOP_IRQ_DMA2);
#endif

    CpuResumeIntr(oldStat);

    _sif2_inited = 1;
    return(0);
}

int SIF2_deinit(void)
{
    volatile register u32 d;
    int oldStat, old_irq;

    if(!_sif2_inited) { return(-1); }

    CpuSuspendIntr(&oldStat);

    *R_IOP_D2_CHCR = 0;

    // Disable DMA CH2
    if((*R_IOP_DPCR & 0x0800))
    {
        *R_IOP_DPCR |= 0x0800;
        d = *R_IOP_DPCR;
    }

    DisableIntr(IOP_IRQ_DMA2, &old_irq);

    CpuResumeIntr(oldStat);

    _sif2_inited = 0;
    return(0);
}
