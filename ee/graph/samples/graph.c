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
#include <stdio.h>

#include <gif_tags.h>

#include <gs_gp.h>
#include <gs_psm.h>

#include <dma.h>
#include <dma_tags.h>

#include <draw.h>
#include <graph.h>
#include <graph_vram.h>
#include <packet.h>

void init_gs(FRAMEBUFFER *frame, ZBUFFER *z)
{

	// Define a 32-bit 512x512 framebuffer.
	frame->width = 512;
	frame->height = 512;
	frame->mask = 0;
	frame->psm = GS_PSM_32;

	// Switch the mask on for some fun.
	//frame->mask = 0xFFFF0000;

	// Allocate some vram for our framebuffer
	frame->address = graph_vram_allocate(frame->width,frame->height, frame->psm, GRAPH_ALIGN_PAGE);

	// Disable the zbuffer.
	z->enable = 0;
	z->address = 0;
	z->mask = 0;
	z->zsm = 0;

	// Initialize the screen and tie the framebuffer to the read circuits.
	graph_initialize(frame->address,frame->width,frame->height,frame->psm,0,0);

	// This is how you would define a custom mode
	//graph_set_mode(GRAPH_MODE_NONINTERLACED,GRAPH_MODE_VGA_1024_60,GRAPH_MODE_FRAME,GRAPH_DISABLE);
	//graph_set_screen(0,0,512,768);
	//graph_set_bgcolor(0,0,0);
	//graph_set_framebuffer_filtered(frame->address,frame->width,frame->psm,0,0);
	//graph_enable_output();

}

void init_drawing_environment(PACKET *packet, FRAMEBUFFER *frame, ZBUFFER *z)
{

	// This is our generic qword pointer.
	QWORD *q = packet->data;

	// This will setup a default drawing environment.
	q = draw_setup_environment(q,0,frame,z);

	// This is where you could add various other drawing environment settings,
	// or keep on adding onto the packet, but I'll stop with the default setup
	// by appending a finish tag.

	q = draw_finish(q);

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_normal(DMA_CHANNEL_GIF,packet->data,q - packet->data, 0, 0);

	// Wait until the finish event occurs.
	draw_wait_finish();

}

void render(PACKET *packet, FRAMEBUFFER *frame)
{

	// Used for the render loop.
	int loop0;

	// Used for the qword pointer.
	QWORD *q = packet->data;

	// Since we only have one packet, we need to wait until the dmac is done
	// before reusing our pointer;
	dma_wait_fast();

	q = packet->data;
	q = draw_clear(q,0,0,0,frame->width,frame->height,0,0,0);
	q = draw_finish(q);

	dma_channel_send_normal(DMA_CHANNEL_GIF,packet->data, q - packet->data, 0, 0);

	// Wait until the screen is cleared.
	draw_wait_finish();

	// Update the screen.
   graph_wait_vsync();

	// Draw 20 100x100 squares from the origin point.
	for (loop0=0;loop0<20;loop0++)
	{

		// No dmatags in a normal transfer.
		q = packet->data;

		// Wait for our previous dma transfer to end.
		dma_wait_fast();

   // Draw another square on the screen.
		PACK_GIFTAG(q, GIF_SET_TAG(4, 1, 0, 0, 0, 1),GIF_REG_AD);
		q++;
		PACK_GIFTAG(q, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0), GIF_REG_PRIM);
		q++;
		PACK_GIFTAG(q, GIF_SET_RGBAQ((loop0 * 10), 0, 255 - (loop0 * 10), 0x80, 0x3F800000), GIF_REG_RGBAQ);
		q++;
		PACK_GIFTAG(q, GIF_SET_XYZ(((loop0 * 20)) << 4, ((loop0 * 10)) << 4, 0), GIF_REG_XYZ2);
		q++;
		PACK_GIFTAG(q, GIF_SET_XYZ(((loop0 * 20) + 100) << 4, ((loop0 * 10) + 100) << 4, 0), GIF_REG_XYZ2);
		q++;

		q = draw_finish(q);

		// DMA send
		dma_channel_send_normal(DMA_CHANNEL_GIF,packet->data,q - packet->data, 0, 0);

		// Wait until the drawing is finished.
		draw_wait_finish();

		// Now initiate vsync.
		graph_wait_vsync();

  }

}

int main(void)
{

	// The minimum buffers needed for single buffered rendering.
	FRAMEBUFFER frame;
	ZBUFFER z;

	// The data packet.
	PACKET packet;
	
	packet_allocate(&packet, 50, 0, 0);

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(&frame,&z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(&packet,&frame,&z);

	// Render the sample.
	render(&packet,&frame);

	// Free the vram.
	graph_vram_free(frame.address);

	// Free the packet.
	packet_free(&packet);

	// Disable output and reset the GS.
  graph_shutdown();

	// Shutdown our currently used dma channel.
	dma_channel_shutdown(DMA_CHANNEL_GIF,0);

  // End program.
  return 0;

}
