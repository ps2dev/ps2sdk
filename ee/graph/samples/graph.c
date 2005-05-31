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
 #include <stdio.h>
 #include <graph.h>
 #include <malloc.h>
 #include <packet.h>

 int main() { int loop0;

  PACKET packet;

  // Initialize the dma library.
  dma_initialize();

  // Initialize the graphics library.
  graph_initialize();

  // Allocate space for the packet.
  packet_allocate(&packet, 1024);

  // Set the mode.
  graph_set_mode(GRAPH_MODE_AUTO, GRAPH_PSM_32, GRAPH_PSM_32);

  // Set the display buffer.
  graph_set_displaybuffer(0);

  // Set the draw buffer.
  graph_set_drawbuffer(0);

  // Set the zbuffer.
  graph_set_zbuffer(graph_get_width() * graph_get_height() * (graph_get_bpp() >> 3));

  // Clear the screen.
  graph_draw_clear(0, 64, 0);

  // Draw 20 squares...
  for (loop0=0;loop0<20;loop0++) {

   // Wait for the vsync period.
   graph_wait_vsync();

   // Reset the packet.
   packet_reset(&packet);

   // Draw another square on the screen.
   packet_append_64(&packet, GIF_SET_TAG(4, 1, 0, 0, 0, 1));
   packet_append_64(&packet, 0x0E);
   packet_append_64(&packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
   packet_append_64(&packet, GIF_REG_PRIM);
   packet_append_64(&packet, GIF_SET_RGBAQ((loop0 * 10), 0, 255 - (loop0 * 10), 0x80, 0x3F800000));
   packet_append_64(&packet, GIF_REG_RGBAQ);
   packet_append_64(&packet, GIF_SET_XYZ(((loop0 * 20) + 1800) << 4, ((loop0 * 10) + 1900) << 4, 0));
   packet_append_64(&packet, GIF_REG_XYZ2);
   packet_append_64(&packet, GIF_SET_XYZ(((loop0 * 20) + 1900) << 4, ((loop0 * 10) + 2000) << 4, 0));
   packet_append_64(&packet, GIF_REG_XYZ2);

   // Send off the packet.
   packet_send(&packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL);

  }

  // Shut down the graphics library.
  graph_shutdown();

  // Shut down the dma library.
  dma_shutdown();

  // End program.
  return 0;

 }
