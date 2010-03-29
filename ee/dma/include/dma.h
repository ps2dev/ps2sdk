/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#ifndef __DMA_H__
#define __DMA_H__

#include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

 #define DMA_CHANNEL_VIF0		0x00
 #define DMA_CHANNEL_VIF1		0x01
 #define DMA_CHANNEL_GIF		0x02
 #define DMA_CHANNEL_fromIPU		0x03
 #define DMA_CHANNEL_toIPU		0x04
	#define DMA_CHANNEL_fromSIF0	0x05
	#define DMA_CHANNEL_toSIF1		0x06
 #define DMA_CHANNEL_SIF2		0x07
 #define DMA_CHANNEL_fromSPR		0x08
 #define DMA_CHANNEL_toSPR		0x09

	#define DMA_FLAG_TRANSFERTAG	0x01
	#define DMA_FLAG_INTERRUPTSAFE	0x02

	int dma_reset(void);

 int dma_channel_initialize(int channel, void *handler, int flags);
 // Initializes the specified dma channel.

	void dma_channel_fast_waits(int channel);
	// Enables dma fast waits for that channel

	void dma_wait_fast(void);
	// Waits until channel is usable based on coprocessor status

	int dma_channel_wait(int channel, int timeout);
 // Wait until the specified dma channel is ready.

	int dma_channel_send_chain(int channel, void *data, int qwc, int flags, int spr);
	// Send a dmachain to the specified dma channel.

	int dma_channel_send_chain_ucab(int channel, void *data, int qwc, int flags);
	// Send a ucab dmachain to the specified dma channel.

	int dma_channel_send_normal(int channel, void *data, int qwc, int flags, int spr);
 // Send data to the specified dma channel.

	int dma_channel_send_normal_ucab(int channel, void *data, int qwc, int flags);
	// Send ucab data to the specified dma channel.

	int dma_channel_receive_normal(int channel, void *data, int data_size, int flags, int spr);
	// Receive data from the specified dma channel.

	int dma_channel_receive_chain(int channel, void *data, int data_size, int flags, int spr);
 // Receive data from the specified dma channel.

 int dma_channel_shutdown(int channel, int flags);
 // Shut down the specified dma channel.

#ifdef __cplusplus
}
#endif

#endif
