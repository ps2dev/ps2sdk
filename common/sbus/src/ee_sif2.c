/*

Common PS2 SIF management API for both EE and IOP.

This file contains all common code for both EE and IOP SIF management.

*/

#include <tamtypes.h>
#include <ps2_reg_defs.h>
#include "ee_regs.h"
#include "ps2_sbus.h"
#include "sbus_priv.h"

static int _sif2_inited = 0;
static u32 _sif2_xfer_addr = 0;
static u32 _sif2_xfer_size = 0;
static u32 _sif2_xfer_attr = 0;
static u32 _sif2_xfer_chunk_size = 0;

int SIF2_RestartDma(void);

void SIF2_sync_dma(void)
{
    int oldi;

    M_SuspendIntr(&oldi);

    while(1)
    {
        if(!(*R_EE_D7_CHCR & EE_CHCR_STR))
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
                break;
            }
        }
    }

    M_ResumeIntr(oldi);
}

int SIF2_RestartDma(void)
{
    int oldi;

    M_SuspendIntr(&oldi);

// disable CH7 interrupt and clear interrupt status
    *R_EE_D_STAT &= 0x00800080;

// enable CH7 priority
//    *R_EE_D_PCR |= 0x00800000;

    _sif2_xfer_chunk_size = (_sif2_xfer_size > SIF2_XFER_CHUNK_SIZE) ? SIF2_XFER_CHUNK_SIZE : _sif2_xfer_size;

    *R_EE_D7_CHCR = 0;
    *R_EE_D7_MADR = (_sif2_xfer_addr & 0x0FFFFFFF);
    *R_EE_D7_QWC = ((_sif2_xfer_chunk_size + 15) / 16);
    *R_EE_D7_CHCR = _sif2_xfer_attr | EE_CHCR_STR;

    M_ResumeIntr(oldi);
}

int SIF2_set_dma(u32 addr, u32 size, u32 attr)
{
    size = ((size + 15) / 16) * 16;

    _sif2_xfer_addr = addr;
    _sif2_xfer_size = size;
    _sif2_xfer_attr = attr;

    SIF2_RestartDma();
    return(0);
}


int SIF2_init(void)
{
    int oldi;

    if(_sif2_inited) { return(-1); }

    _DisableDmac(7);

    M_SuspendIntr(&oldi);

    // Enable DMA
    *R_EE_D_CTRL |= 1;

    *R_LOCAL_SBUS(PS2_SBUS_REG4) = 0x00000100;
    *R_LOCAL_SBUS(PS2_SBUS_REG6) = 0x000000FF;

    *R_EE_D7_CHCR = 0;

    M_ResumeIntr(oldi);

//    EnableDmac(7);

    _sif2_inited = 1;

    return(0);
}

int SIF2_deinit(void)
{
    int oldi;

    if(!_sif2_inited) { return(-1); }

    _DisableDmac(7);

    M_SuspendIntr(&oldi);
    *R_EE_D7_CHCR = 0;
    M_ResumeIntr(oldi);

    _sif2_inited = 0;

    return(0);
}

