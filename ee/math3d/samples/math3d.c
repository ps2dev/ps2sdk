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
 #include <math3d.h>
 #include <packet.h>

 VECTOR points[36] = {
  { -10.00f, -10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f, -10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f, -10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f,  10.00f, -10.00f, 1.00f },
  { -10.00f, -10.00f,  10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f,  10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f,  10.00f, 1.00f },
  {  10.00f,  10.00f,  10.00f, 1.00f },
  {  10.00f, -10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  {  10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  {  10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f,  10.00f,  10.00f, 1.00f },
  { -10.00f, -10.00f, -10.00f, 1.00f },
  { -10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  { -10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  { -10.00f,  10.00f,  10.00f, 1.00f },
  { -10.00f, -10.00f,  10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f, -10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f,  10.00f, 1.00f },
  { -10.00f, -10.00f, -10.00f, 1.00f },
  {  10.00f, -10.00f, -10.00f, 1.00f },
  { -10.00f,  10.00f,  10.00f, 1.00f },
  {  10.00f,  10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f,  10.00f,  10.00f, 1.00f },
  { -10.00f,  10.00f, -10.00f, 1.00f },
  {  10.00f,  10.00f, -10.00f, 1.00f },
 };

 u64 colours[36] = {
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x80, 0x00, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x80, 0x00, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
  GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000),
 };

 VECTOR object_position = { 0.00f, 0.00f, 0.00f, 1.00f };
 VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 1.00f };

 VECTOR camera_position = { 0.00f, 0.00f, -100.00f, 1.00f };
 VECTOR camera_rotation = { 0.00f, 0.00f,    0.00f, 1.00f };

 //////////////////
 // MAIN PROGRAM //
 //////////////////

 // Packet structure used by draw_gradient().
 PACKET packet;

 // Draws a fancy gradient background.
 int draw_gradient(void);

 int main(int argc, char **argv) {

  MATRIX local_world;
  MATRIX world_view;
  MATRIX view_clip;
  MATRIX local_clip;
  MATRIX clip_screen;

  VECTORI pointsi[36];

  // Allocate the packet.
  packet_allocate(&packet, 128);

  // Set the graphics mode.
  graph_set_mode(GRAPH_MODE_AUTO, GRAPH_PSM_32, GRAPH_PSM_32);

  // Set the display buffer address.
  graph_set_displaybuffer(0);

  // Set the draw buffer address.
  graph_set_drawbuffer(0);

  // Set the zbuffer address.
  graph_set_zbuffer(graph_get_size());

  // Create the view_clip matrix.
  create_view_clip(view_clip, graph_get_aspect(), -2.00f, 2.00f, -2.00f, 2.00f, 1.00f, 1000.00f);

  // Create the clip_screen matrix.
  create_clip_screen(clip_screen, 4096.00f, 4096.00f, 16777215.00f);

  // The main loop...
  for (;;) {

   // Spin the cube a bit.
   object_rotation[0] += 0.028f; while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
   object_rotation[1] += 0.012f; while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

   // Create the local_world matrix. (object space)
   create_local_world(local_world, object_position, object_rotation);

   // Create the world_view matrix. (camera space)
   create_world_view(world_view, camera_position, camera_rotation);

   // Create the local_clip matrix.
   create_local_clip(local_clip, local_world, world_view, view_clip);

   // Apply the matrices to the points.
   point_calculate(pointsi, points, 36, local_clip, clip_screen);

   // Wait for the vsync period.
   graph_wait_vsync();

   // Clear the screen.
   graph_draw_clear(80, 80, 80);

   // Draw the gradient background.
   draw_gradient();

   // Draw the triangles.
   graph_draw_primatives(GRAPH_PRIM_TRIANGLE, 36, pointsi, colours, NULL);

  }

  // End program.
  return 0;

 }

 int draw_gradient(void) {

  // Reset the packet.
  packet_reset(&packet);

  // Draw the gradient.
  packet_append_64(&packet, GIF_SET_TAG(10, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&packet, 0x0E);
  packet_append_64(&packet, GIF_SET_PRIM(3, 1, 0, 0, 0, 1, 0, 0, 0));
  packet_append_64(&packet, GIF_REG_PRIM);
  packet_append_64(&packet, GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000));
  packet_append_64(&packet, GIF_REG_RGBAQ);
  packet_append_64(&packet, GIF_SET_XYZ((2048 - (graph_get_width() / 2)) << 4, (2048 - (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);
  packet_append_64(&packet, GIF_SET_XYZ((2048 + (graph_get_width() / 2)) << 4, (2048 - (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);
  packet_append_64(&packet, GIF_SET_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x3F800000));
  packet_append_64(&packet, GIF_REG_RGBAQ);
  packet_append_64(&packet, GIF_SET_XYZ((2048 + (graph_get_width() / 2)) << 4, (2048 + (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);
  packet_append_64(&packet, GIF_SET_XYZ((2048 + (graph_get_width() / 2)) << 4, (2048 + (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);
  packet_append_64(&packet, GIF_SET_XYZ((2048 - (graph_get_width() / 2)) << 4, (2048 + (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);
  packet_append_64(&packet, GIF_SET_RGBAQ(0x00, 0x00, 0x80, 0x80, 0x3F800000));
  packet_append_64(&packet, GIF_REG_RGBAQ);
  packet_append_64(&packet, GIF_SET_XYZ((2048 - (graph_get_width() / 2)) << 4, (2048 - (graph_get_height() / 2)) << 4, 0));
  packet_append_64(&packet, GIF_REG_XYZ2);

  // Send off the packet.
  packet_send(&packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL);

  // End function.
  return 0;

 }
