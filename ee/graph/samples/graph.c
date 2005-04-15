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

 int main() {

  int loop0;

  u64 packet[8192], *temp = packet;

  // Initialize the dma library.
  dma_initialize();

  // Initialize the graphics library.
  graph_initialize();

  // Set the display mode.
  graph_mode_set(GRAPH_MODE_NTSC, GRAPH_PSM_32, GRAPH_PSM_32);

  // Set the display buffer.
  graph_set_displaybuffer(0);

  // Set the draw buffer.
  graph_set_drawbuffer(0);

  // Set the zbuffer.
  graph_set_zbuffer(2 * 1024 * 1024);

  // Clear the screen.
  graph_set_clearbuffer(0, 64, 0);

  // Draw 20 squares...
  for (loop0=0;loop0<20;loop0++) {

   // Wait for the vsync period.
   graph_wait_vsync();

   // Draw another square on the screen.
   *temp++ = GIF_SET_TAG(4, 1, 0, 0, 0, 1);
   *temp++ = 0x0E;
   *temp++ = GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0);
   *temp++ = GIF_REG_PRIM;
   *temp++ = GIF_SET_RGBAQ((loop0 * 10), 0, 255 - (loop0 * 10), 0x80, 0x3F800000);
   *temp++ = GIF_REG_RGBAQ;
   *temp++ = GIF_SET_XYZ(((loop0 * 20) + 1800) << 4, ((loop0 * 10) + 1900) << 4, 0);
   *temp++ = GIF_REG_XYZ2;
   *temp++ = GIF_SET_XYZ(((loop0 * 20) + 1900) << 4, ((loop0 * 10) + 2000) << 4, 0);
   *temp++ = GIF_REG_XYZ2;

  }

  // Send off the packet.
  dma_channel_send(DMA_CHANNEL_GIF, packet, (temp - packet) << 3, DMA_FLAG_NORMAL);

  // Wait for the packet transfer.
  dma_channel_wait(DMA_CHANNEL_GIF, -1, DMA_FLAG_NORMAL);

  // Shut down the graphics library.
  graph_shutdown();

  // Shut down the dma library.
  dma_shutdown();

  // End program.
  return 0;

 }
