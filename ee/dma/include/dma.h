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

#ifdef __cplusplus
extern "C" {
#endif

 #define DMA_CHANNEL_VIF0	0x00
 #define DMA_CHANNEL_VIF1	0x01
 #define DMA_CHANNEL_GIF	0x02
 #define DMA_CHANNEL_fromIPU	0x03
 #define DMA_CHANNEL_toIPU	0x04
 #define DMA_CHANNEL_SIF0	0x05
 #define DMA_CHANNEL_SIF1	0x06
 #define DMA_CHANNEL_SIF2	0x07
 #define DMA_CHANNEL_fromSPR	0x08
 #define DMA_CHANNEL_toSPR	0x09

 ///////////////////
 // DMA FUNCTIONS //
 ///////////////////

 int dma_initialize(void);

 int dma_shutdown(void);

 ///////////////////////////
 // DMA CHANNEL FUNCTIONS //
 ///////////////////////////

 int dma_channel_initialize(int channel, void *handler);

 int dma_channel_wait(int channel, int timeout);

 int dma_channel_send(int channel, void *data, int data_size);

 int dma_channel_shutdown(int channel);

#ifdef __cplusplus
}
#endif

#endif
