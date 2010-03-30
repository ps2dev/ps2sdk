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

VECTOR camera_position = { 0.00f, 0.00f, 100.00f, 1.00f };
VECTOR camera_rotation = { 0.00f, 0.00f,   0.00f, 1.00f };

VECTOR *temp_normals;
VECTOR *temp_lights;
VECTOR *temp_colours;
VECTOR *temp_vertices;

XYZ *xyz;
COLOR *rgbaq;

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

void init_gs(FRAMEBUFFER *frame, ZBUFFER *z)
{

	// Define a 32-bit 640x512 framebuffer.
	frame->width = 640;
	frame->height = 512;
	frame->mask = 0;
	frame->psm = GS_PSM_32;

	// Allocate some vram for our framebuffer.
	frame->address = graph_vram_allocate(frame->width,frame->height, frame->psm, GRAPH_ALIGN_PAGE);

	frame++;

	frame->width = 640;
	frame->height = 512;
	frame->mask = 0;
	frame->psm = GS_PSM_32;

	// Allocate some vram for our framebuffer.
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

void flip_buffers(PACKET *flip,FRAMEBUFFER *frame)
{

	QWORD *q = flip->data;

	q = draw_framebuffer(q,0,frame);
	q = draw_finish(q);

	dma_wait_fast();
	dma_channel_send_normal_ucab(DMA_CHANNEL_GIF,flip->data,q - flip->data, 0);

	draw_wait_finish();

}

QWORD *render_teapot(QWORD *q,MATRIX view_screen, VECTOR object_position, VECTOR object_rotation, PRIMITIVE *prim, COLOR *color, FRAMEBUFFER *frame, ZBUFFER *z)
{

	int i;

	QWORD *dmatag;

	MATRIX local_world;
	MATRIX local_light;
	MATRIX world_view;

	MATRIX local_screen;

	// Now grab our qword pointer and increment past the dmatag.
	dmatag = q;
	q++;

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

	// Convert floating point vertices to fixed point and translate to center of screen.
	draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count, (VERTEXF*)temp_vertices);

	// Convert floating point colours to fixed point.
	draw_convert_rgbq(rgbaq, vertex_count, (VERTEXF*)temp_vertices, (COLORF*)temp_colours, color->a);

	// Draw the triangles using triangle primitive type.
	q = draw_prim_start(q,0,prim,color);

	for(i = 0; i < points_count; i++)
	{
		q->dw[0] = rgbaq[points[i]].rgbaq;
		q->dw[1] = xyz[points[i]].xyz;
		q++;
	}

	q = draw_prim_end(q,2,DRAW_RGBAQ_REGLIST,1);

	// Define our dmatag for the dma chain.
	DMATAG_CNT(dmatag,q-dmatag-1,0,0,0);


	return q;

}

int render(PACKET *packet, FRAMEBUFFER *frame, ZBUFFER *z)
{

	int context = 0;

	PACKET flip_pkt;

	QWORD *q;
	QWORD *dmatag;

	PRIMITIVE prim;
	COLOR color;

	MATRIX view_screen;

	packet_allocate(&flip_pkt,3,1,0);

	VECTOR object_position = { 0.00f, 0.00f, 0.00f, 1.00f };
	VECTOR object_rotation = { 0.00f, 0.00f, 0.00f, 1.00f };

	// Define the triangle primitive we want to use.
	prim.type = PRIM_TRIANGLE;
	prim.shading = PRIM_SHADE_GOURAUD;
	prim.mapping = DRAW_DISABLE;
	prim.fogging = DRAW_DISABLE;
	prim.blending = DRAW_ENABLE;
	prim.antialiasing = DRAW_DISABLE;
	prim.mapping_type = DRAW_DISABLE;
	prim.colorfix = PRIM_UNFIXED;

	color.r = 0x80;
	color.g = 0x80;
	color.b = 0x80;
	color.a = 0x80;
	color.q = 1.0f;

	// Allocate calculation space.
	temp_normals  = memalign(128, sizeof(VECTOR) * vertex_count);
	temp_lights   = memalign(128, sizeof(VECTOR) * vertex_count);
	temp_colours  = memalign(128, sizeof(VECTOR) * vertex_count);
	temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

	// Allocate register space.
	xyz   = memalign(128, sizeof(u64) * vertex_count);
	rgbaq = memalign(128, sizeof(u64) * vertex_count);

	// Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

	for (;;)
	{

		q = packet[context].data;

		dmatag = q;
		q++;

		// Clear framebuffer without any pixel testing.
		q = draw_disable_tests(q,0,z);
		q = draw_clear(q,0,2048.0f-320.0f,2048.0f-256.0f,frame->width,frame->height,0x00,0x00,0x00);
		q = draw_enable_tests(q,0,z);

		DMATAG_CNT(dmatag,q-dmatag - 1,0,0,0);

		//render teapots
		color.a = 0x40;
		object_position[0] = 30.0f;
		q = render_teapot(q, view_screen, object_position, object_rotation, &prim, &color, frame, z);

		object_position[0] = -30.0f;
		q = render_teapot(q, view_screen, object_position, object_rotation, &prim, &color, frame, z);

		color.a = 0x80;
		object_position[0] = 0.0f;
		object_position[1] = -20.0f;
		q = render_teapot(q, view_screen, object_position, object_rotation, &prim, &color, frame, z);

		object_position[1] = 20.0f;
		q = render_teapot(q, view_screen, object_position, object_rotation, &prim, &color, frame, z);

		object_position[0] = 0.0f;
		object_position[1] = 0.0f;

		dmatag = q;
		q++;

		q = draw_finish(q);

		DMATAG_END(dmatag,q-dmatag-1,0,0,0);

		// Now send our current dma chain.
		dma_wait_fast();
		dma_channel_send_chain(DMA_CHANNEL_GIF,packet[context].data, q - packet[context].data, 0, 0);

		// Either block until a vsync, or keep rendering until there's one available.
   graph_wait_vsync();

		draw_wait_finish();
		graph_set_framebuffer_filtered(frame[context].address,frame[context].width,frame[context].psm,0,0);

		// Switch context.
		context ^= 1;

		// We need to flip buffers outside of the chain, for some reason.
		flip_buffers(&flip_pkt,&frame[context]);

  }

}

int main(int argc, char **argv)
{

	// The buffers to be used.
	FRAMEBUFFER frame[2];
	ZBUFFER z;

	// The data packets for double buffering dma sends.
	PACKET packets[2];

	packet_allocate(&packets[0],40000,0,0);
	packet_allocate(&packets[1],40000,0,0);

	// Init GIF dma channel.
	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	// Init the GS, framebuffer, and zbuffer.
	init_gs(frame, &z);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(packets,frame,&z);

	render(packets,frame,&z);

  // End program.
  return 0;

}
