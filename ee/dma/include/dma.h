/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * DMA channel utilities
 */

#ifndef __DMA_H__
#define __DMA_H__

#include <tamtypes.h>
#include <packet2_types.h>

#define DMA_CHANNEL_VIF0     0x00
#define DMA_CHANNEL_VIF1     0x01
#define DMA_CHANNEL_GIF      0x02
#define DMA_CHANNEL_fromIPU  0x03
#define DMA_CHANNEL_toIPU    0x04
#define DMA_CHANNEL_fromSIF0 0x05
#define DMA_CHANNEL_toSIF1   0x06
#define DMA_CHANNEL_SIF2     0x07
#define DMA_CHANNEL_fromSPR  0x08
#define DMA_CHANNEL_toSPR    0x09

#define DMA_FLAG_TRANSFERTAG   0x01
#define DMA_FLAG_INTERRUPTSAFE 0x02

#ifdef __cplusplus
extern "C" {
#endif

int dma_reset(void);

/** Initializes the specified dma channel. */
int dma_channel_initialize(int channel, void *handler, int flags);

/** Enables dma fast waits for that channel */
void dma_channel_fast_waits(int channel);

/** Waits until channel is usable based on coprocessor status */
void dma_wait_fast(void);

/** Wait until the specified dma channel is ready. */
int dma_channel_wait(int channel, int timeout);

/** 
 * Send packet2. 
 * Type chain/normal is choosen from packet2_t. 
 * @param packet2 Pointer to packet. 
 * @param channel DMA channel. 
 * @param flush_cache Should be cache flushed before send? 
 */
void dma_channel_send_packet2(packet2_t *packet2, int channel, u8 flush_cache);

/** Send a dmachain to the specified dma channel. */
int dma_channel_send_chain(int channel, void *data, int qwc, int flags, int spr);

/** Send a ucab dmachain to the specified dma channel. */
int dma_channel_send_chain_ucab(int channel, void *data, int qwc, int flags);

/** Send data to the specified dma channel. */
int dma_channel_send_normal(int channel, void *data, int qwc, int flags, int spr);

/** Send ucab data to the specified dma channel. */
int dma_channel_send_normal_ucab(int channel, void *data, int qwc, int flags);

/** Receive data from the specified dma channel. */
int dma_channel_receive_normal(int channel, void *data, int data_size, int flags, int spr);

/** Receive data from the specified dma channel. */
int dma_channel_receive_chain(int channel, void *data, int data_size, int flags, int spr);

/** Shut down the specified dma channel. */
int dma_channel_shutdown(int channel, int flags);

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H__ */
