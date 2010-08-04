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

#include <kernel.h>
#include <stdlib.h>
#include <tamtypes.h>
#include <math3d.h>

#include <packet.h>

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <dma.h>

#include <graph.h>
#include <graph_vram.h>

#include <draw.h>
#include <draw3d.h>

#include "mesh_data.c"

VECTOR object_position = { 0.00f, 0.00f, 0.00f, 1.00f };
VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 1.00f };

VECTOR camera_position = { 0.00f, 0.00f, 100.00f, 1.00f };
VECTOR camera_rotation = { 0.00f, 0.00f,   0.00f, 1.00f };

void init_gs(FRAMEBUFFER *frame, ZBUFFER *z)
{

	// Define a 32-bit 640x512 framebuffer.
	frame->width = 640;
	frame->height = 512;
	frame->mask = 0;
	frame->psm = GS_PSM_32;
	frame->address = graph_vram_allocate(frame->width,frame->height, frame->psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	z->enable = DRAW_ENABLE;
	z->mask = 0;
	z->method = ZTEST_METHOD_GREATER_EQUAL;
	z->zsm = GS_ZBUF_32;
	z->address = graph_vram_allocate(frame->width,frame->height,z->zsm, GRAPH_ALIGN_PAGE);

	// Initialize the screen and tie the first framebuffer to the read circuits.
	graph_initialize(frame->address,frame->width,frame->height,frame->psm,0,0);

}

void init_drawing_environment(PACKET *packet, FRAMEBUFFER *frame, ZBUFFER *z)
{

	// This is our generic qword pointer.
	QWORD *q = packet->data;

	// This will setup a default drawing environment.
	q = draw_setup_environment(q,0,frame,z);

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	q = draw_primitive_xyoffset(q,0,(2048-320),(2048-256));

	// Finish setting up the environment.
	q = draw_finish(q);

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_normal(DMA_CHANNEL_GIF,packet->data,q - packet->data, 0, 0);

}

int render(PACKET *packet,FRAMEBUFFER *frame, ZBUFFER *z)
{

	int i;
	int context = 0;

	QWORD *q;
	QWORD *dmatag;

	PRIMITIVE prim;
	COLOR color;

  MATRIX local_world;
  MATRIX world_view;
  MATRIX view_screen;
  MATRIX local_screen;

  VECTOR *temp_vertices;

	XYZ   *verts;
	COLOR *colors;

  // Allocate calculation space.
  temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

  // Allocate register space.
	verts  = memalign(128, sizeof(VERTEX) * vertex_count);
	colors = memalign(128, sizeof(COLOR)  * vertex_count);

	// Define the triangle primitive we want to use.
	prim.type = PRIM_TRIANGLE;
	prim.shading = PRIM_SHADE_GOURAUD;
	prim.mapping = DRAW_DISABLE;
	prim.fogging = DRAW_DISABLE;
	prim.blending = DRAW_DISABLE;
	prim.antialiasing = DRAW_ENABLE;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix = PRIM_UNFIXED;

	color.r = 0x80;
	color.g = 0x80;
	color.b = 0x80;
	color.a = 0x80;
	color.q = 1.0f;

  // Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

	// Wait for any previous dma transfers to finish before starting.
	dma_wait_fast();

  // The main loop...
	for (;;)
	{

   // Spin the cube a bit.
		object_rotation[0] += 0.008f; //while (object_rotation[0] > 3.14f) { object_rotation[0] -= 6.28f; }
		object_rotation[1] += 0.012f; //while (object_rotation[1] > 3.14f) { object_rotation[1] -= 6.28f; }

   // Create the local_world matrix.
   create_local_world(local_world, object_position, object_rotation);

   // Create the world_view matrix.
   create_world_view(world_view, camera_position, camera_rotation);

   // Create the local_screen matrix.
   create_local_screen(local_screen, local_world, world_view, view_screen);

   // Calculate the vertex values.
   calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

		// Convert floating point vertices to fixed point and translate to center of screen.
		draw_convert_xyz(verts, 2048, 2048, 32, vertex_count, (VERTEXF*)temp_vertices);

		// Convert floating point colours to fixed point.
		draw_convert_rgbq(colors, vertex_count, (VERTEXF*)temp_vertices, (COLORF*)colours, color.a);

		// Grab our dmatag pointer for the dma chain.
		dmatag = packet[context].data;

		// Now grab our qword pointer and increment past the dmatag.
		q = dmatag;
		q++;

		// Clear framebuffer but don't update zbuffer.
		q = draw_disable_tests(q,0,z);
		q = draw_clear(q,0,2048.0f-320.0f,2048.0f-256.0f,frame->width,frame->height,0x00,0x00,0x00);
		q = draw_enable_tests(q,0,z);

		// Draw the triangles using triangle primitive type.
		q = draw_prim_start(q,0,&prim, &color);

		for(i = 0; i < points_count; i++)
		{
			q->dw[0] = colors[points[i]].rgbaq;
			q->dw[1] = verts[points[i]].xyz;
			q++;
		}

		q = draw_prim_end(q,2,DRAW_RGBAQ_REGLIST);

		// Setup a finish event.
		q = draw_finish(q);

		// Define our dmatag for the dma chain.
		DMATAG_END(dmatag,q-packet[context].data-1,0,0,0);

		// Now send our current dma chain.
		dma_wait_fast();
		dma_channel_send_chain(DMA_CHANNEL_GIF,packet[context].data, q - packet[context].data, 0, 0);

		// Now switch our packets so we can process data while the DMAC is working.
		context ^= 1;

		// Wait for scene to finish drawing
		draw_wait_finish();

		graph_wait_vsync();

  }

	return 0;

}

int main(int argc, char **argv)
{

	// The buffers to be used.
	FRAMEBUFFER frame;
	ZBUFFER z;

	// The data packets for double buffering dma sends.
	PACKET packets[2];

	packet_allocate(&packets[0],100,0,0);
	packet_allocate(&packets[1],100,0,0);

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(&frame, &z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(packets,&frame,&z);

	render(packets,&frame,&z);

  // End program.
  return 0;

}
