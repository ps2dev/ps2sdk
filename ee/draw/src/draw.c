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
 #include <draw.h>
 #include <graph.h>
 #include <math3d.h>
 #include <packet.h>

 PACKET draw_packet;

 ////////////////////
 // DRAW FUNCTIONS //
 ////////////////////

 int draw_initialize(int mode, int bpp, int zbpp) {

  // Initialize the graphics library.
  if (graph_initialize() < 0) { return -1; }

  // Set the graphics mode.
  if (graph_set_mode(mode, bpp, zbpp) < 0) { return -1; }

  // If the mode is interlaced...
  if (graph_get_interlace() == GRAPH_INTERLACED) {

   // Set the display buffer address.
   if (graph_set_displaybuffer(0) < 0) { return -1; }

   // Set the draw buffer address.
   if (graph_set_drawbuffer(0) < 0) { return -1; }

   // Set the zbuffer address.
   if (graph_set_zbuffer(graph_get_size()) < 0) { return -1; }

  // Else, the mode is non-interlaced...
  } else {

   // Set the display buffer address.
   if (graph_set_displaybuffer(0) < 0) { return -1; }

   // Set the draw buffer address.
   if (graph_set_drawbuffer(graph_get_size()) < 0) { return -1; }

   // Set the zbuffer address.
   if (graph_set_zbuffer(graph_get_size() * 2) < 0) { return -1; }

  }

  // Wait and clear both buffers.
  draw_swap(); draw_clear(0.00f, 0.00f, 0.00f);
  draw_swap(); draw_clear(0.00f, 0.00f, 0.00f);

  // Initialize the packet.
  if (packet_allocate(&draw_packet, 1024) < 0) { return -1; }

  // End function.
  return 0;

 }

 int draw_swap(void) {

  // If the mode is interlaced...
  if (graph_get_interlace() == GRAPH_INTERLACED) {

   // If the display field is even.
   if (graph_get_displayfield() == GRAPH_FIELD_EVEN) {

    // Set the drawfield to odd.
    if (graph_set_drawfield(GRAPH_FIELD_ODD) < 0) { return -1; }

   // Else, the display field is odd...
   } else {

    // Set the drawfield to even.
    if (graph_set_drawfield(GRAPH_FIELD_EVEN) < 0) { return -1; }

   }

  // Else, the mode is non-interlaced...
  } else { int temp = 0;

   // Save the display buffer address.
   temp = graph_get_displaybuffer();

   // Set the display buffer address.
   if (graph_set_displaybuffer(graph_get_drawbuffer()) < 0) { return -1; }

   // Set the draw buffer address.
   if (graph_set_drawbuffer(temp) < 0) { return -1; }

  }

  // End function.
  return 0;

 }

 int draw_clear(float red, float green, float blue) {

  // Reset the packet.
  if (packet_reset(&draw_packet) < 0) { return -1; }

  // Clear the draw buffer and zbuffer.
  packet_append_64(&draw_packet, GIF_SET_TAG(6, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&draw_packet, 0x0E);
  packet_append_64(&draw_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
  packet_append_64(&draw_packet, GIF_REG_TEST_1);
  packet_append_64(&draw_packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
  packet_append_64(&draw_packet, GIF_REG_PRIM);
  packet_append_64(&draw_packet, GIF_SET_RGBAQ((int)(red * 128), (int)(green * 128), (int)(blue * 128), 0x80, 0x3F800000));
  packet_append_64(&draw_packet, GIF_REG_RGBAQ);
  packet_append_64(&draw_packet, GIF_SET_XYZ(0x0000, 0x0000, 0x0000));
  packet_append_64(&draw_packet, GIF_REG_XYZ2);
  packet_append_64(&draw_packet, GIF_SET_XYZ(0xFFFF, 0xFFFF, 0x0000));
  packet_append_64(&draw_packet, GIF_REG_XYZ2);
  packet_append_64(&draw_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  packet_append_64(&draw_packet, GIF_REG_TEST_1);

  // Send the packet.
  if (packet_send(&draw_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 /////////////////////////////
 // DRAW GENERATE FUNCTIONS //
 /////////////////////////////

 int draw_generate_xyz(u64 *output, int count, VECTOR *vertices) {
  int loop0; unsigned int max_z;

  // Find the maximum Z value.
  max_z = 1 << (graph_get_zbpp() - 1);

  // For each vertex...
  for (loop0=0;loop0<count;loop0++) {

   // Calculate the XYZ register value.
   output[loop0] = GIF_SET_XYZ(
    (int)((vertices[loop0][0] + 1.00f) *  32768),
    (int)((vertices[loop0][1] + 1.00f) * -32768),
    (int)((vertices[loop0][2] + 1.00f) * max_z)
   );

  }

  // End function.
  return 0;

 }

 int draw_generate_rgbaq(u64 *output, int count, VECTOR *vertices, VECTOR *colours) {
  int loop0; float q = 1.00f;

  // For each colour...
  for (loop0=0;loop0<count;loop0++) {

   // Calculate the Q value.
   if (vertices[loop0][3] != 0) { q = 1 / vertices[loop0][3]; }

   // Calculate the RGBAQ register value.
   output[loop0] = GIF_SET_RGBAQ(
    (int)(colours[loop0][0] * 128),
    (int)(colours[loop0][1] * 128),
    (int)(colours[loop0][2] * 128),
    (int)(colours[loop0][3] * 128),
    *(unsigned int *)&q
   );

  }

  // End function.
  return 0;

 }

 int draw_generate_st(u64 *output, int count, VECTOR *vertices, VECTOR *coords) {
  int loop0; float q = 1.00f, s = 1.00f, t = 1.00f;

  // For each coordinate...
  for (loop0=0;loop0<count;loop0++) {

   // Calculate the Q value.
   if (vertices[loop0][3] != 0) { q = 1 / vertices[loop0][3]; }

   // Calculate the S and T values.
   s = (float)(coords[loop0][0] * q);
   t = (float)(coords[loop0][1] * q);

   // Calculate the ST register value.
   output[loop0] = GIF_SET_ST(
    *(unsigned int *)&s,
    *(unsigned int *)&t
   );

  }

  // End function.
  return 0;

 }

 //////////////////////////////
 // DRAW PRIMITIVE FUNCTIONS //
 //////////////////////////////

 int draw_triangles(int *points, int count, u64 *xyz, u64 *rgbaq) {
  int loop0;

  // If the packet is too small...
  if (draw_packet.size < (48 + (16 * count))) {

   // Free the packet.
   packet_free(&draw_packet);

   // Allocate a larger packet.
   packet_allocate(&draw_packet, (48 + (16 * count)));

  }

  // Reset the packet.
  if (packet_reset(&draw_packet) < 0) { return -1; }

  // Set up the draw process.
  packet_append_64(&draw_packet, GIF_SET_TAG(1, 0, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&draw_packet, 0x000000000000000E);
  packet_append_64(&draw_packet, GIF_SET_PRIM(3, 1, 0, 0, 0, 0, 0, 0, 0));
  packet_append_64(&draw_packet, GIF_REG_PRIM);
  packet_append_64(&draw_packet, GIF_SET_TAG(count, 1, 0, 0, GIF_TAG_REGLIST, 2));
  packet_append_64(&draw_packet, 0x0000000000000051);

  // For each point...
  for (loop0=0;loop0<count;loop0++) {

   // Add the RGBAQ register value.
   packet_append_64(&draw_packet, rgbaq[points[loop0]]);

   // Add the XYZ register value.
   packet_append_64(&draw_packet, xyz[points[loop0]]);

  }

  // Send the packet.
  if (packet_send(&draw_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 int draw_triangles_textured(int *points, int count, u64 *xyz, u64 *rgbaq, u64 *st) {
  int loop0;

  // If the packet is too small...
  if (draw_packet.size < (48 + (16 * count))) {

   // Free the packet.
   packet_free(&draw_packet);

   // Allocate a larger packet.
   packet_allocate(&draw_packet, (48 + (16 * count)));

  }

  // Reset the packet.
  if (packet_reset(&draw_packet) < 0) { return -1; }

  // Set up the draw process.
  packet_append_64(&draw_packet, GIF_SET_TAG(1, 0, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&draw_packet, 0x000000000000000E);
  packet_append_64(&draw_packet, GIF_SET_PRIM(3, 1, 1, 0, 0, 0, 0, 0, 0));
  packet_append_64(&draw_packet, GIF_REG_PRIM);
  packet_append_64(&draw_packet, GIF_SET_TAG(count, 1, 0, 0, GIF_TAG_REGLIST, 3));
  packet_append_64(&draw_packet, 0x0000000000000512);

  // For each point...
  for (loop0=0;loop0<count;loop0++) {

   // Add the ST register value.
   packet_append_64(&draw_packet, st[points[loop0]]);

   // Add the RGBAQ register value.
   packet_append_64(&draw_packet, rgbaq[points[loop0]]);

   // Add the XYZ register value.
   packet_append_64(&draw_packet, xyz[points[loop0]]);

  }

  // Send the packet.
  if (packet_send(&draw_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }
