/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SIO2 logging.
 */

#ifndef SIO2LOG_LOG_H
#define SIO2LOG_LOG_H

#include <types.h>

#include "sio2man.h"

enum _log_types {
	LOG_HEADER = 0x02, LOG_PAD_READY, LOG_MC_READY, LOG_MTAP_READY,
	LOG_TRS, LOG_TRS_PD, LOG_TRS_RD, LOG_TRS_DATA, LOG_TRS_DMA_IN,
	LOG_TRS_DMA_OUT,
	LOG_TRR, LOG_TRR_STAT, LOG_TRR_DATA,
	LOG_RESET
};

void log_write8(u8 val);
void log_write32(u32 val);
void log_flush(int now);

void log_default(int type);
void log_portdata(u32 *pd1, u32 *pd2);
void log_regdata(u32 *rd);
void log_data(int type, u8 *data, u32 size);
void log_dma(int type, struct _sio2_dma_arg *arg);
void log_stat(u32 stat6c, u32 stat70, u32 stat74);

#endif /* SIO2LOG_LOG_H */
