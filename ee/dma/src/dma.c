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

#include <tamtypes.h>

#include <dma.h>
#include <kernel.h>
#include <malloc.h>
#include <string.h>

#include <dma_registers.h>

// Channel Control
static u32 dma_chcr[10] = { 0x10008000, 0x10009000, 0x1000A000, 0x1000B000, 0x1000B400, 0x1000C000, 0x1000C400, 0x1000C800, 0x1000D000, 0x1000D400 };
// Quadword Count
static u32 dma_qwc[10]  = { 0x10008020, 0x10009020, 0x1000A020, 0x1000B020, 0x1000B420, 0x1000C020, 0x1000C420, 0x1000C820, 0x1000D020, 0x1000D420 };
// Memory Address
static u32 dma_madr[10] = { 0x10008010, 0x10009010, 0x1000A010, 0x1000B010, 0x1000B410, 0x1000C010, 0x1000C410, 0x1000C810, 0x1000D010, 0x1000D410 };
// Tag Address
static u32 dma_tadr[10] = { 0x10008030, 0x10009030, 0x1000A030, 0x1000B030, 0x1000B430, 0x1000C030, 0x1000C430, 0x1000C830, 0x1000D030, 0x1000D430 };
// Tag Address Save 0
static u32 dma_asr0[10] = { 0x10008040, 0x10009040, 0x1000A040, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
// Tag Address Save 1
static u32 dma_asr1[10] = { 0x10008050, 0x10009050, 0x1000A050, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
// SPR Transfer Address
static u32 dma_sadr[10] = { 0x10008080, 0x10009080, 0x1000A080, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1000D080, 0x1000D480 };

static int dma_handler_id[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static int dma_channel_initialized[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


int dma_reset(void)
{

	int i;

	for(i = 0; i < 10; i++)
	{

		dma_channel_shutdown(i,0);

 }

	ResetEE(0x01);

  return 0;

}

int dma_channel_initialize(int channel, void *handler, int flags)
{

  // Shut down the channel before making changes.
	if (dma_channel_shutdown(channel, flags) < 0)
	{
		return -1;
	}

	// Clear any saved dmatags
	if (dma_asr0[channel])
	{
		*(vu32 *)dma_asr0[channel] = 0;
		*(vu32 *)dma_asr1[channel] = 0;
	}

	// Clear saved spr address
	if (dma_sadr[channel])
	{
		*(vu32 *)dma_sadr[channel] = 0;
	}

  // If a handler is provided...
	if (handler != NULL)
	{

   // Add the handler, storing the handler id.
   dma_handler_id[channel] = AddDmacHandler(channel, handler, 0);

   // Enable the channel interrupt.
		if (flags & DMA_FLAG_INTERRUPTSAFE)
		{
    iEnableDmac(channel);
		}
		else
		{
    EnableDmac(channel);
   }

  }

  // Tell everyone we are initialized.
  dma_channel_initialized[channel] = 1;

  // End function.
  return 0;

}

// allows for cpcond0 checking
void dma_channel_fast_waits(int channel)
{

	*DMA_REG_PCR |= 1 << channel;

}

void dma_wait_fast(void)
{

	asm volatile (
		"sync.l; sync.p;" \
		"0:" \
		"bc0t 0f; nop;" \
		"bc0t 0f; nop;" \
		"bc0t 0f; nop;" \
		"bc0f 0b; nop;" \
		"0:"
	);

}

int dma_channel_wait(int channel, int timeout)
{

	// While the channel is not ready...
	while (*((vu32 *)dma_chcr[channel]) & 0x00000100)
	{

    // Decrement the timeout counter, exiting if it expires.
		if (timeout > 0)
		{

			if (timeout-- == 0)
			{

				return -1;

			};

   }

  }

  // End function.
  return 0;

}

int dma_channel_send_chain(int channel, void *data, int data_size, int flags, int spr)
{

	// clear channel status
	*DMA_REG_STAT = DMA_SET_STAT(1 << channel,0,0,0,0,0,0);

	if (flags & DMA_FLAG_INTERRUPTSAFE)
	{
		iSyncDCache(data, data + (data_size<<4));
	}
	else
	{
		SyncDCache(data, data + (data_size<<4));
 }

	// Set the size of the data, in quadwords.
	*(vu32 *)dma_qwc[channel] = DMA_SET_QWC(0);

	// Set the address of the data.
	*(vu32 *)dma_madr[channel] = DMA_SET_MADR(0, 0);

	// Set the address of the data tag.
	*(vu32 *)dma_tadr[channel] = DMA_SET_TADR((u32)data, spr);

	// Start the transfer.
	*(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(1, 1, 0, flags & DMA_FLAG_TRANSFERTAG, 1, 1, 0);

	return 0;

}

int dma_channel_send_chain_ucab(int channel, void *data, int qwc, int flags)
{

	// clear channel status
	*DMA_REG_STAT = 1 << channel;

   // Set the size of the data, in quadwords.
   *(vu32 *)dma_qwc[channel] = DMA_SET_QWC(0);

   // Set the address of the data.
   *(vu32 *)dma_madr[channel] = DMA_SET_MADR(0, 0);

   // Set the address of the data tag.
	*(vu32 *)dma_tadr[channel] = DMA_SET_TADR((u32)data - 0x30000000, 0);

   // Start the transfer.
	*(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(1, 1, 0, flags & DMA_FLAG_TRANSFERTAG, 1, 1, 0);

	// End function.
	return 0;

}

int dma_channel_send_normal(int channel, void *data, int qwc, int flags, int spr)
{

	// clear channel status
	*DMA_REG_STAT = DMA_SET_STAT(1 << channel,0,0,0,0,0,0);

	// Not sure if this should be here.
	if (flags & DMA_FLAG_INTERRUPTSAFE)
	{
		iSyncDCache(data, data + (qwc<<4));
	}
	else
	{
		SyncDCache(data, data + (qwc<<4));
	}

   // Set the size of the data, in quadwords.
	*(vu32 *)dma_qwc[channel] = DMA_SET_QWC(qwc);

   // Set the address of the data.
	*(vu32 *)dma_madr[channel] = DMA_SET_MADR((u32)data, spr);

	// Start the transfer.
	*(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(1, 0, 0, flags & DMA_FLAG_TRANSFERTAG, 1, 1, 0);

	return 0;

}

int dma_channel_send_normal_ucab(int channel, void *data, int qwc, int flags)
{

	// clear channel status
	*DMA_REG_STAT = DMA_SET_STAT(1 << channel,0,0,0,0,0,0);

	// Not sure if this should be here.
	if (flags & DMA_FLAG_INTERRUPTSAFE)
	{
		iSyncDCache(data, data + (qwc<<4));
	}
	else
	{
		SyncDCache(data, data + (qwc<<4));
   }

	// Set the size of the data, in quadwords.
	*(vu32 *)dma_qwc[channel] = DMA_SET_QWC(qwc);

	// Set the address of the data.
	*(vu32 *)dma_madr[channel] = DMA_SET_MADR((u32)data - 0x30000000, 0);

   // Start the transfer.
	*(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(1, 0, 0, flags & DMA_FLAG_TRANSFERTAG, 1, 1, 0);

	return 0;

}

int dma_channel_receive_chain(int channel, void *data, int data_size, int flags, int spr)
{

	// If we are not initialized...
	if (dma_channel_initialized[channel] < 0)
	{

		// Initialize the channel.
		if (dma_channel_initialize(channel, NULL, flags) < 0)
		{
			return -1;
		}

  }

	// Set the size of the data, in quadwords.
	*(vu32 *)dma_qwc[channel] = DMA_SET_QWC((data_size + 15) >> 4);

	// Set the address of the data.
	*(vu32 *)dma_madr[channel] = DMA_SET_MADR((u32)data, spr);

	// Start the transfer.
	*(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(0, 1, 0, 0, 0, 1, 0);

  // End function.
  return 0;

}

int dma_channel_receive_normal(int channel, void *data, int data_size, int flags, int spr)
{

  // If we are not initialized...
	if (dma_channel_initialized[channel] < 0)
	{

   // Initialize the channel.
		if (dma_channel_initialize(channel, NULL, flags) < 0)
		{
			return -1;
  }

	}

  // Set the size of the data, in quadwords.
  *(vu32 *)dma_qwc[channel] = DMA_SET_QWC((data_size + 15) >> 4);

  // Set the address of the data.
	*(vu32 *)dma_madr[channel] = DMA_SET_MADR((u32)data, spr);

  // Start the transfer.
   *(vu32 *)dma_chcr[channel] = DMA_SET_CHCR(0, 0, 0, 0, 0, 1, 0);

  // End function.
  return 0;

}

int dma_channel_shutdown(int channel, int flags)
{

  // If we are not initialized, no need to shut down.
	if (dma_channel_initialized[channel] < 0)
	{
		return 0;
	}

  // If a handler was provided...
	if (dma_handler_id[channel] != 0)
	{

   // Disable the channel.
		if (flags & DMA_FLAG_INTERRUPTSAFE)
		{
    iDisableDmac(channel);
		}
		else
		{
    DisableDmac(channel);
   }

   // Remove the handler.
   RemoveDmacHandler(channel, dma_handler_id[channel]);

   // Clear the handler id.
   dma_handler_id[channel] = 0;

  }

  // Tell everyone we are not initialized.
	dma_channel_initialized[channel] = 0;

  // End function.
  return 0;

}
