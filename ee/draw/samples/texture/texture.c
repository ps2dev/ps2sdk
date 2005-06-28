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

 //////////////////
 // MAIN PROGRAM //
 //////////////////

 int main(int argc, char **argv) {

  MATRIX local_world;
  MATRIX world_view;
  MATRIX view_screen;
  MATRIX local_screen;

  VECTOR *temp_vertices;

  u64 *xyz;
  u64 *rgbaq;
  u64 *st;

  FILE *infile;
  int texture_size;
  u8 *texture_data;

  // Allocate calculation space.
  temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

  // Allocate register space.
  xyz   = memalign(128, sizeof(u64) * vertex_count);
  rgbaq = memalign(128, sizeof(u64) * vertex_count);
  st    = memalign(128, sizeof(u64) * vertex_count);

  // Initialize the draw library.
  draw_initialize(GRAPH_MODE_AUTO, GRAPH_PSM_32, GRAPH_PSM_32);

  // Create the view_screen matrix.
  create_view_screen(view_screen, graph_get_aspect(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

  // Open the texture file.
  infile = fopen("host:texture.raw", "r");

  // Determine the texture data size.
  fseek(infile, 0, SEEK_END); texture_size = ftell(infile); fseek(infile, 0, SEEK_SET);

  // Allocate space for the texture data.
  texture_data = memalign(128, texture_size);

  // Load the texture data.
  fread(texture_data, texture_size, 1, infile);

  // Upload the texture to vram.
  graph_vram_write(3145728, 256, 256, GRAPH_PSM_24, texture_data, texture_size);

  // Free the texture data space.
  free(texture_data);

  // Close the texture file.
  fclose(infile);

  // The main loop...
  for (;;) {

   // Spin the cube a bit.
   object_rotation[0] += 0.008f; while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
   object_rotation[1] += 0.012f; while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

   // Create the local_world matrix.
   create_local_world(local_world, object_position, object_rotation);

   // Create the world_view matrix.
   create_world_view(world_view, camera_position, camera_rotation);

   // Create the local_screen matrix.
   create_local_screen(local_screen, local_world, world_view, view_screen);

   // Calculate the vertex values.
   calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

   // Generate the XYZ register values.
   draw_generate_xyz(xyz, vertex_count, temp_vertices);

   // Generate the RGBAQ register values.
   draw_generate_rgbaq(rgbaq, vertex_count, temp_vertices, colours);

   // Generate the ST register values.
   draw_generate_st(st, vertex_count, temp_vertices, coordinates);

   // Wait for vsync.
   graph_wait_vsync();

   // Swap the buffers.
   draw_swap();

   // Clear the screen.
   draw_clear(0.50f, 0.50f, 0.50f);

   // Set texture texture information.
   graph_set_texture(3145728, 256, 256, GRAPH_PSM_24);

   // Draw the textured triangles.
   draw_triangles_textured(points, points_count, xyz, rgbaq, st);

  }

  // End program.
  return 0;

 }
