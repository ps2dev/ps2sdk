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

 #include <stdio.h>
 #include <malloc.h>

 #include <dma.h>
 #include <dma_registers.h>

 #include <graph.h>
 #include <graph_registers.h>

 int main() { DMA_PACKET packet; int loop0;

  // Initialize the dma library.
  dma_initialize();

  // Allocate the packet.
  dma_packet_allocate(&packet, 8192);

  // Initialize the graphics library.
  graph_initialize();

  // Set the display mode.
  graph_set_displaymode(GRAPH_MODE_NTSC, GRAPH_BPP_32);

  // Set the display buffer.
  graph_set_displaybuffer(0);

  // Set the draw buffer.
  graph_set_drawbuffer(0, -1, -1, -1);

  // SEt the zbuffer.
  graph_set_zbuffer(2097152, GRAPH_BPP_32); // 2MB

  // Clear the screen.
  graph_set_clearbuffer(0, 64, 0, 0);

  // Draw 20 squares...
  for (loop0=0;loop0<20;loop0++) {

   // Wait for the vsync period.
   graph_wait_vsync();

   // Draw another square on the screen.
   dma_packet_clear(&packet);
   dma_packet_append(&packet, GIF_SET_TAG(4, 1, 0, 0, 0, 1));
   dma_packet_append(&packet, 0x0E);
   dma_packet_append(&packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
   dma_packet_append(&packet, GIF_REG_PRIM);
   dma_packet_append(&packet, GIF_SET_RGBAQ((loop0 * 10), 0, 255 - (loop0 * 10), 0x80, 0x3F800000));
   dma_packet_append(&packet, GIF_REG_RGBAQ);
   dma_packet_append(&packet, GIF_SET_XYZ(((loop0 * 20) + 100) << 4, ((loop0 * 10) + 100) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, GIF_SET_XYZ(((loop0 * 20) + 150) << 4, ((loop0 * 10) + 150) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_send(&packet, DMA_CHANNEL_GIF);

  }

  // Shut down the graphics library.
  graph_shutdown();

  // Free the packet.
  dma_packet_free(&packet);

  // Shut down the dma library.
  dma_shutdown();

  // End program.
  return 0;

 }
