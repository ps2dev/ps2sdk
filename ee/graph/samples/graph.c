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

 #include <dma.h>
 #include <graph.h>
 #include <graph_registers.h>

 GRAPH_PACKET packet;

 int main() { int loop0;

  // Initialize the graphics library.
  graph_initialize();

  // Set the display mode.
  graph_set_displaymode(GRAPH_MODE_NTSC, GRAPH_BPP_32);

  // Set up the display, draw and zbuffers.
  graph_set_displaybuffer(0);
  graph_set_drawbuffer(0, -1, -1, -1);
  graph_set_zbuffer(2097152, GRAPH_BPP_32);

  // Clear the screen.
  graph_set_clearbuffer(0, 0, 0, 0);

  // Allocate space for the packet.
  graph_packet_allocate(&packet, 8192);

  // Draw 20 squares...
  for (loop0=0;loop0<20;loop0++) {

   // Wait for the vsync period.
   graph_vsync_wait();

   // Draw another square on the screen.
   graph_packet_clear(&packet);
   graph_packet_append(&packet, GIF_SET_TAG(4, 1, 0, 0, 0, 1));
   graph_packet_append(&packet, 0x000000000000000E);
   graph_packet_append(&packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
   graph_packet_append(&packet, GIF_REG_PRIM);
   graph_packet_append(&packet, GIF_SET_RGBAQ(256 - (loop0 * 10), 0, (loop0 * 10), 0x80, 0x3F800000));
   graph_packet_append(&packet, GIF_REG_RGBAQ);
   graph_packet_append(&packet, GIF_SET_XYZ(((loop0 * 20) + 100) << 4, ((loop0 * 10) + 100) << 4, 0));
   graph_packet_append(&packet, GIF_REG_XYZ2);
   graph_packet_append(&packet, GIF_SET_XYZ(((loop0 * 20) + 150) << 4, ((loop0 * 10) + 150) << 4, 0));
   graph_packet_append(&packet, GIF_REG_XYZ2);
   graph_packet_send(&packet);

  }

  // Free the packet.
  graph_packet_free(&packet);

  // Shut down the graphics library.
  graph_shutdown();

  // End program.
  return 0;

 }
