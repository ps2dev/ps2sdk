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
 #include <stdio.h>
 #include <malloc.h>
 #include <math3d.h>
 #include <dma_registers.h>
 #include <graph_registers.h>

 int lines[12][2] = {
  { 0, 1 }, { 1, 3 }, { 3, 2 },
  { 2, 0 }, { 4, 5 }, { 5, 7 },
  { 7, 6 }, { 6, 4 }, { 0, 4 },
  { 2, 6 }, { 1, 5 }, { 3, 7 }
 };

 int triangles[12][3] = {
  { 0, 1, 2 }, { 1, 2, 3 }, { 4, 5, 6 },
  { 5, 6, 7 }, { 1, 5, 3 }, { 5, 3, 7 },
  { 0, 4, 2 }, { 4, 2, 6 }, { 4, 5, 0 },
  { 5, 0, 1 }, { 6, 7, 2 }, { 7, 2, 3 }
 };

 VECTOR points[8] = {
  { -10.00f, -10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f, -10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f,  10.00f, -10.00f, 1.00f },
  { -10.00f, -10.00f,  10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f,  10.00f, 1.00f },
  {  10.00f,  10.00f,  10.00f, 1.00f }
 };

 u64 colours[5] = {
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),	// Gradient top.
  GIF_SET_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x3F800000)	// Gradient bottom.
 };

 VECTOR object_position = { 0.00f, 0.00f, 0.00f, 0.00f };
 VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 0.00f };

 VECTOR camera_position = { 0.00f, 0.00f, -50.00f, 0.00f };
 VECTOR camera_rotation = { 0.00f, 0.00f,   0.00f, 0.00f };

 int main() {

  int loop0; 

  GRAPH_MODE mode;

  DMA_PACKET packet;

  VECTORI pointsi[8];

  MATRIX local_world;
  MATRIX world_view;
  MATRIX view_screen;
  MATRIX local_screen;

  // Initialize the dma library.
  dma_initialize();

  // Allocate the dma packet.
  dma_packet_allocate(&packet, 8192);

  // Initialize the graphics library.
  graph_initialize();

  // Set the graphics mode.
  graph_mode_set(GRAPH_MODE_NTSC, GRAPH_BPP_32, GRAPH_BPP_32);		// NTSC (640x448i@60Hz)
  // graph_mode_set(GRAPH_MODE_PAL,  GRAPH_BPP_32, GRAPH_BPP_32);	// PAL  (640x512i@50Hz)
  // graph_mode_set(GRAPH_MODE_HDTV, GRAPH_BPP_32, GRAPH_BPP_32);	// HDTV (720x420p@60Hz)
  // graph_mode_set(GRAPH_MODE_VGA,  GRAPH_BPP_32, GRAPH_BPP_32);	// VGA  (640x480p@60Hz)

  // Save the mode information.
  graph_mode_get(&mode);

  // Set the display buffer address.
  graph_set_displaybuffer(0);

  // Set the draw buffer address.
  graph_set_drawbuffer(0);

  // Set the zbuffer address.
  graph_set_zbuffer(mode.width * mode.height * 4);

  // Create the view_screen matrix.
  create_view_screen(view_screen, mode.width, mode.height, 16777215, 4, 3, 512.00f, 1.00f, 65536.00f);		// Standard (4:3)
  // create_view_screen(view_screen, mode.width, mode.height, 16777215, 16, 9, 512.00f, 1.00f, 65536.00f);	// Widescreen (16:9)

  // The main loop...
  for (;;) {

   // Spin the cube a bit.
   object_rotation[1] += 0.010f;
   object_rotation[2] += 0.013f;

   // Create the local_world matrix. (object space)
   create_local_world(local_world, object_position, object_rotation);

   // Create the world_view matrix. (camera space)
   create_world_view(world_view, camera_position, camera_rotation);

   // Create the local_screen matrix.
   create_local_screen(local_screen, local_world, world_view, view_screen);

   // Apply the local_screen matrix to the points.
   point_calculate(pointsi, local_screen, points, 8);

   // Wait for the vsync period.
   graph_wait_vsync();

   // Clear the screen.
   graph_set_clearbuffer(80, 80, 80);

   // Clear the packet.
   dma_packet_clear(&packet);

   // Draw the gradient.
   dma_packet_append(&packet, GIF_SET_TAG(10, 1, 0, 0, GIF_TAG_PACKED, 1));
   dma_packet_append(&packet, 0x0E);
   dma_packet_append(&packet, GIF_SET_PRIM(3, 1, 0, 0, 0, 1, 0, 0, 0));
   dma_packet_append(&packet, GIF_REG_PRIM);
   dma_packet_append(&packet, colours[3]);
   dma_packet_append(&packet, GIF_REG_RGBAQ);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 - (mode.width / 2)) << 4, (2048 - (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 + (mode.width / 2)) << 4, (2048 - (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, colours[4]);
   dma_packet_append(&packet, GIF_REG_RGBAQ);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 + (mode.width / 2)) << 4, (2048 + (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 + (mode.width / 2)) << 4, (2048 + (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 - (mode.width / 2)) << 4, (2048 + (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);
   dma_packet_append(&packet, colours[3]);
   dma_packet_append(&packet, GIF_REG_RGBAQ);
   dma_packet_append(&packet, GIF_SET_XYZ((2048 - (mode.width / 2)) << 4, (2048 - (mode.height / 2)) << 4, 0));
   dma_packet_append(&packet, GIF_REG_XYZ2);

   // Draw the lines.
   for (loop0=0;loop0<12;loop0++) {
    dma_packet_append(&packet, GIF_SET_TAG(4, 1, 1, GIF_SET_PRIM(1, 1, 0, 0, 0, 1, 0, 0, 0), GIF_TAG_PACKED, 1));
    dma_packet_append(&packet, 0x0E);
    dma_packet_append(&packet, GIF_SET_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x3F800000));
    dma_packet_append(&packet, GIF_REG_RGBAQ);
    dma_packet_append(&packet, GIF_SET_XYZ(pointsi[lines[loop0][0]][0], pointsi[lines[loop0][0]][1], pointsi[lines[loop0][0]][2]));
    dma_packet_append(&packet, GIF_REG_XYZ2);
    dma_packet_append(&packet, GIF_SET_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x3F800000));
    dma_packet_append(&packet, GIF_REG_RGBAQ);
    dma_packet_append(&packet, GIF_SET_XYZ(pointsi[lines[loop0][1]][0], pointsi[lines[loop0][1]][1], pointsi[lines[loop0][1]][2]));
    dma_packet_append(&packet, GIF_REG_XYZ2);
   }

   // Draw the triangles.
   for (loop0=0;loop0<12;loop0++) {
    dma_packet_append(&packet, GIF_SET_TAG(6, 1, 1, GIF_SET_PRIM(3, 1, 0, 0, 0, 0, 0, 0, 0), GIF_TAG_PACKED, 1));
    dma_packet_append(&packet, 0x0E);
    dma_packet_append(&packet, colours[loop0 >> 2]);
    dma_packet_append(&packet, GIF_REG_RGBAQ);
    dma_packet_append(&packet, GIF_SET_XYZ(pointsi[triangles[loop0][0]][0], pointsi[triangles[loop0][0]][1], pointsi[triangles[loop0][0]][2]));
    dma_packet_append(&packet, GIF_REG_XYZ2);
    dma_packet_append(&packet, colours[loop0 >> 2]);
    dma_packet_append(&packet, GIF_REG_RGBAQ);
    dma_packet_append(&packet, GIF_SET_XYZ(pointsi[triangles[loop0][1]][0], pointsi[triangles[loop0][1]][1], pointsi[triangles[loop0][1]][2]));
    dma_packet_append(&packet, GIF_REG_XYZ2);
    dma_packet_append(&packet, colours[loop0 >> 2]);
    dma_packet_append(&packet, GIF_REG_RGBAQ);
    dma_packet_append(&packet, GIF_SET_XYZ(pointsi[triangles[loop0][2]][0], pointsi[triangles[loop0][2]][1], pointsi[triangles[loop0][2]][2]));
    dma_packet_append(&packet, GIF_REG_XYZ2);
   }

   // Send off the packet.
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
