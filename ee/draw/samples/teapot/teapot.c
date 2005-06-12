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
 #include <stdio.h>
 #include <graph.h>
 #include <malloc.h>
 #include <math3d.h>
 #include <packet.h>
 #include <string.h>

 #include "mesh_data.c"

 VECTOR object_position = { 0.00f, 0.00f, 0.00f, 1.00f };
 VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 1.00f };

 VECTOR camera_position = { 0.00f, 0.00f, 100.00f, 1.00f };
 VECTOR camera_rotation = { 0.00f, 0.00f,   0.00f, 1.00f };

 int light_count = 4;

 VECTOR light_direction[4] = {
  {  0.00f,  0.00f,  0.00f, 1.00f },
  {  1.00f,  0.00f, -1.00f, 1.00f },
  {  0.00f,  1.00f, -1.00f, 1.00f },
  { -1.00f, -1.00f, -1.00f, 1.00f }
 };

 VECTOR light_colour[4] = {
  { 0.00f, 0.00f, 0.00f, 1.00f },
  { 1.00f, 0.00f, 0.00f, 1.00f },
  { 0.30f, 0.30f, 0.30f, 1.00f },
  { 0.50f, 0.50f, 0.50f, 1.00f }
 };

 int light_type[4] = {
  LIGHT_AMBIENT,
  LIGHT_DIRECTIONAL,
  LIGHT_DIRECTIONAL,
  LIGHT_DIRECTIONAL
 };

 //////////////////
 // MAIN PROGRAM //
 //////////////////

 int main(int argc, char **argv) {

  MATRIX local_world;
  MATRIX local_light;
  MATRIX world_view;
  MATRIX view_screen;
  MATRIX local_screen;

  VECTOR *temp_normals;
  VECTOR *temp_lights;
  VECTOR *temp_colours;
  VECTOR *temp_vertices;

  u64 *xyz;
  u64 *rgbaq;

  // Allocate calculation space.
  temp_normals  = memalign(128, sizeof(VECTOR) * vertex_count);
  temp_lights   = memalign(128, sizeof(VECTOR) * vertex_count);
  temp_colours  = memalign(128, sizeof(VECTOR) * vertex_count);
  temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

  // Allocate register space.
  xyz   = memalign(128, sizeof(u64) * vertex_count);
  rgbaq = memalign(128, sizeof(u64) * vertex_count);

  // Initialize the draw library.
  draw_initialize(GRAPH_MODE_AUTO, GRAPH_PSM_32, GRAPH_PSM_32);

  // Create the view_screen matrix.
  create_view_screen(view_screen, graph_get_aspect(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

  // The main loop...
  for (;;) {

   // Spin the teapot a bit.
   object_rotation[0] += 0.008f; while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
   object_rotation[1] += 0.112f; while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

   // Create the local_world matrix.
   create_local_world(local_world, object_position, object_rotation);

   // Create the local_light matrix.
   create_local_light(local_light, object_rotation);

   // Create the world_view matrix.
   create_world_view(world_view, camera_position, camera_rotation);

   // Create the local_screen matrix.
   create_local_screen(local_screen, local_world, world_view, view_screen);

   // Calculate the normal values.
   calculate_normals(temp_normals, vertex_count, normals, local_light);

   // Calculate the lighting values.
   calculate_lights(temp_lights, vertex_count, temp_normals, light_direction, light_colour, light_type, light_count);

   // Calculate the colour values after lighting.
   calculate_colours(temp_colours, vertex_count, colours, temp_lights);

   // Calculate the vertex values.
   calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

   // Generate the XYZ register values.
   draw_generate_xyz(xyz, vertex_count, temp_vertices);

   // Generate the RGBAQ register values.
   draw_generate_rgbaq(rgbaq, vertex_count, temp_vertices, temp_colours);

   // Wait for vsync.
   graph_wait_vsync();

   // Swap the buffers.
   draw_swap();

   // Clear the screen.
   draw_clear(0.00f, 0.00f, 0.00f);

   // Draw the triangles.
   draw_triangles(points, points_count, xyz, rgbaq);

  }

  // End program.
  return 0;

 }
