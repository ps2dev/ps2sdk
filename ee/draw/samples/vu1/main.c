/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
# VU1 and libpacket2 showcase.
*/

#include <kernel.h>
#include <malloc.h>
#include <tamtypes.h>
#include <gs_psm.h>
#include <dma.h>
#include <packet2.h>
#include <packet2_utils.h>
#include <graph.h>
#include <draw.h>
#include "zbyszek.c"
#include "mesh_data.c"

// ---
// Variables declared as global for tutorial only!
// ---

/** Data of our texture (24bit, RGB8) */
extern unsigned char zbyszek[];

/** 
 * Data of VU1 micro program (draw_3D.vcl/vsm). 
 * How we can use it: 
 * 1. Upload program to VU1. 
 * 2. Send calculated local_screen matrix once per mesh (3D object) 
 * 3. Set buffers size. (double-buffering described below) 
 * 4. Send packet with: lod, clut, tex buffer, scale vector, rgba, verts and sts. 
 * What this program is doing? 
 * 1. Load local_screen. 
 * 2. Zero clipping flag. 
 * 3. Set current buffer start address from TOP register (xtop command) 
 *      To use pararelism, we set two buffers in the VU1. It means, that when 
 *      VU1 is working with one verts packet, we can load second one into another buffer. 
 *      xtop command is automatically switching buffers. I think that AAA games used 
 *      quad buffers (TOP+TOPS) which can give best performance and no VIF_FLUSH should be needed. 
 * 4. Load rest of data. 
 * 5. Prepare GIF tag. 
 * 6. For every vertex: transform, clip, scale, perspective divide. 
 * 7. Send it to GS via XGKICK command. 
 */
extern u32 VU1Draw3D_CodeStart __attribute__((section(".vudata")));
extern u32 VU1Draw3D_CodeEnd __attribute__((section(".vudata")));

VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};
VECTOR camera_position = {140.00f, 140.00f, 40.00f, 1.00f};
VECTOR camera_rotation = {0.00f, 0.00f, 0.00f, 1.00f};
MATRIX local_world, world_view, view_screen, local_screen;

/** 
 * Packets for sending VU data 
 * Each packet will have: 
 * a) View/Projection matrix (calculated every frame) 
 * b) Cube data (prim,lod,vertices,sts,...) added from zbyszek_packet. 
 */
packet2_t *vif_packets[2] __attribute__((aligned(64)));
packet2_t *curr_vif_packet;

/** Cube data */
packet2_t *zbyszek_packet;

u8 context = 0;

/** Set GS primitive type of drawing. */
prim_t prim;

/** 
 * Color look up table. 
 * Needed for texture. 
 */
clutbuffer_t clut;

/** 
 * Level of details. 
 * Needed for texture. 
 */
lod_t lod;

/** 
 * Helper arrays. 
 * Needed for calculations. 
 */
VECTOR *c_verts __attribute__((aligned(128))), *c_sts __attribute__((aligned(128)));

/** Calculate packet for cube data */
void calculate_cube(texbuffer_t *t_texbuff)
{
	packet2_add_float(zbyszek_packet, 2048.0F);					  // scale
	packet2_add_float(zbyszek_packet, 2048.0F);					  // scale
	packet2_add_float(zbyszek_packet, ((float)0xFFFFFF) / 32.0F); // scale
	packet2_add_s32(zbyszek_packet, faces_count);				  // vertex count
	packet2_utils_gif_add_set(zbyszek_packet, 1);
	packet2_utils_gs_add_lod(zbyszek_packet, &lod);
	packet2_utils_gs_add_texbuff_clut(zbyszek_packet, t_texbuff, &clut);
	packet2_utils_gs_add_prim_giftag(zbyszek_packet, &prim, faces_count, DRAW_STQ2_REGLIST, 3, 0);
	u8 j = 0; // RGBA
	for (j = 0; j < 4; j++)
		packet2_add_u32(zbyszek_packet, 128);
}

/** Calculate cube position and add packet with cube data */
void draw_cube(VECTOR t_object_position, texbuffer_t *t_texbuff)
{
	create_local_world(local_world, t_object_position, object_rotation);
	create_world_view(world_view, camera_position, camera_rotation);
	create_local_screen(local_screen, local_world, world_view, view_screen);
	curr_vif_packet = vif_packets[context];
	packet2_reset(curr_vif_packet, 0);

	// Add matrix at the beggining of VU mem (skip TOP)
	packet2_utils_vu_add_unpack_data(curr_vif_packet, 0, &local_screen, 8, 0);

	u32 vif_added_bytes = 0; // zero because now we will use TOP register (double buffer)
							 // we don't wan't to unpack at 8 + beggining of buffer, but at
							 // the beggining of the buffer

	// Merge packets
	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, zbyszek_packet->base, packet2_get_qw_count(zbyszek_packet), 1);
	vif_added_bytes += packet2_get_qw_count(zbyszek_packet);

	// Add vertices
	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, c_verts, faces_count, 1);
	vif_added_bytes += faces_count; // one VECTOR is size of qword

	// Add sts
	packet2_utils_vu_add_unpack_data(curr_vif_packet, vif_added_bytes, c_sts, faces_count, 1);
	vif_added_bytes += faces_count;

	packet2_utils_vu_add_start_program(curr_vif_packet, 0);
	packet2_utils_vu_add_end_tag(curr_vif_packet);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	dma_channel_send_packet2(curr_vif_packet, DMA_CHANNEL_VIF1, 1);

	// Switch packet, so we can proceed during DMA transfer
	context = !context;
}

/** Some initialization of GS and VRAM allocation */
void init_gs(framebuffer_t *t_frame, zbuffer_t *t_z, texbuffer_t *t_texbuff)
{
	// Define a 32-bit 640x512 framebuffer.
	t_frame->width = 640;
	t_frame->height = 512;
	t_frame->mask = 0;
	t_frame->psm = GS_PSM_32;
	t_frame->address = graph_vram_allocate(t_frame->width, t_frame->height, t_frame->psm, GRAPH_ALIGN_PAGE);

	// Enable the zbuffer.
	t_z->enable = DRAW_ENABLE;
	t_z->mask = 0;
	t_z->method = ZTEST_METHOD_GREATER_EQUAL;
	t_z->zsm = GS_ZBUF_32;
	t_z->address = graph_vram_allocate(t_frame->width, t_frame->height, t_z->zsm, GRAPH_ALIGN_PAGE);

	// Allocate some vram for the texture buffer
	t_texbuff->width = 128;
	t_texbuff->psm = GS_PSM_24;
	t_texbuff->address = graph_vram_allocate(128, 128, GS_PSM_24, GRAPH_ALIGN_BLOCK);

	// Initialize the screen and tie the first framebuffer to the read circuits.
	graph_initialize(t_frame->address, t_frame->width, t_frame->height, t_frame->psm, 0, 0);
}

/** Some initialization of GS 2 */
void init_drawing_environment(framebuffer_t *t_frame, zbuffer_t *t_z)
{
	packet2_t *packet2 = packet2_create(20, P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);

	// This will setup a default drawing environment.
	packet2_update(packet2, draw_setup_environment(packet2->next, 0, t_frame, t_z));

	// Now reset the primitive origin to 2048-width/2,2048-height/2.
	packet2_update(packet2, draw_primitive_xyoffset(packet2->next, 0, (2048 - 320), (2048 - 256)));

	// Finish setting up the environment.
	packet2_update(packet2, draw_finish(packet2->next));

	// Now send the packet, no need to wait since it's the first.
	dma_channel_send_packet2(packet2, DMA_CHANNEL_GIF, 1);
	dma_wait_fast();

	packet2_free(packet2);
}

/** Send texture data to GS. */
void send_texture(texbuffer_t *texbuf)
{
	packet2_t *packet2 = packet2_create(50, P2_TYPE_NORMAL, P2_MODE_CHAIN, 0);
	packet2_update(packet2, draw_texture_transfer(packet2->next, zbyszek, 128, 128, GS_PSM_24, texbuf->address, texbuf->width));
	packet2_update(packet2, draw_texture_flush(packet2->next));
	dma_channel_send_packet2(packet2, DMA_CHANNEL_GIF, 1);
	dma_wait_fast();
	packet2_free(packet2);
}

/** Send packet which will clear our screen. */
void clear_screen(framebuffer_t *frame, zbuffer_t *z)
{
	packet2_t *clear = packet2_create(35, P2_TYPE_NORMAL, P2_MODE_NORMAL, 0);

	// Clear framebuffer but don't update zbuffer.
	packet2_update(clear, draw_disable_tests(clear->next, 0, z));
	packet2_update(clear, draw_clear(clear->next, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame->width, frame->height, 0x40, 0x40, 0x40));
	packet2_update(clear, draw_enable_tests(clear->next, 0, z));
	packet2_update(clear, draw_finish(clear->next));

	// Now send our current dma chain.
	dma_wait_fast();
	dma_channel_send_packet2(clear, DMA_CHANNEL_GIF, 1);

	packet2_free(clear);

	// Wait for scene to finish drawing
	draw_wait_finish();
}

void set_lod_clut_prim_tex_buff(texbuffer_t *t_texbuff)
{
	lod.calculation = LOD_USE_K;
	lod.max_level = 0;
	lod.mag_filter = LOD_MAG_NEAREST;
	lod.min_filter = LOD_MIN_NEAREST;
	lod.l = 0;
	lod.k = 0;

	clut.storage_mode = CLUT_STORAGE_MODE1;
	clut.start = 0;
	clut.psm = 0;
	clut.load_method = CLUT_NO_LOAD;
	clut.address = 0;

	// Define the triangle primitive we want to use.
	prim.type = PRIM_TRIANGLE;
	prim.shading = PRIM_SHADE_GOURAUD;
	prim.mapping = DRAW_ENABLE;
	prim.fogging = DRAW_DISABLE;
	prim.blending = DRAW_ENABLE;
	prim.antialiasing = DRAW_DISABLE;
	prim.mapping_type = PRIM_MAP_ST;
	prim.colorfix = PRIM_UNFIXED;

	t_texbuff->info.width = draw_log2(128);
	t_texbuff->info.height = draw_log2(128);
	t_texbuff->info.components = TEXTURE_COMPONENTS_RGB;
	t_texbuff->info.function = TEXTURE_FUNCTION_DECAL;
}

void render(framebuffer_t *t_frame, zbuffer_t *t_z, texbuffer_t *t_texbuff)
{
	int i, j;

	set_lod_clut_prim_tex_buff(t_texbuff);

	/** 
	 * Allocate some space for object position calculating. 
	 * c_ prefix = calc_
	 */
	c_verts = (VECTOR *)memalign(128, sizeof(VECTOR) * faces_count);
	c_sts = (VECTOR *)memalign(128, sizeof(VECTOR) * faces_count);

	VECTOR c_zbyszek_position;

	for (i = 0; i < faces_count; i++)
	{
		c_verts[i][0] = vertices[faces[i]][0];
		c_verts[i][1] = vertices[faces[i]][1];
		c_verts[i][2] = vertices[faces[i]][2];
		c_verts[i][3] = vertices[faces[i]][3];

		c_sts[i][0] = sts[faces[i]][0];
		c_sts[i][1] = sts[faces[i]][1];
		c_sts[i][2] = sts[faces[i]][2];
		c_sts[i][3] = sts[faces[i]][3];
	}

	// Create the view_screen matrix.
	create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);
	calculate_cube(t_texbuff);

	// The main loop...
	for (;;)
	{
		// Spin the cube a bit.
		object_rotation[0] += 0.008f;
		while (object_rotation[0] > 3.14f)
		{
			object_rotation[0] -= 6.28f;
		}
		object_rotation[1] += 0.012f;
		while (object_rotation[1] > 3.14f)
		{
			object_rotation[1] -= 6.28f;
		}

		camera_position[2] += .5F;
		camera_rotation[2] += 0.002f;
		if (camera_position[2] >= 400.0F)
		{
			camera_position[2] = 40.0F;
			camera_rotation[2] = 0.00f;
		}

		clear_screen(t_frame, t_z);

		for (i = 0; i < 8; i++)
		{
			c_zbyszek_position[0] = i * 40.0F;
			for (j = 0; j < 8; j++)
			{
				c_zbyszek_position[1] = j * 40.0F;
				draw_cube(c_zbyszek_position, t_texbuff);
			}
		}

		graph_wait_vsync();
	}
}

void vu1_set_double_buffer_settings()
{
	packet2_t *packet2 = packet2_create(1, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	packet2_utils_vu_add_double_buffer(packet2, 8, 496);
	packet2_utils_vu_add_end_tag(packet2);
	dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	packet2_free(packet2);
}

void vu1_upload_micro_program()
{
	u32 packet_size =
		packet2_utils_get_packet_size_for_program(&VU1Draw3D_CodeStart, &VU1Draw3D_CodeEnd) + 1; // + 1 for end tag
	packet2_t *packet2 = packet2_create(packet_size, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	packet2_vif_add_micro_program(packet2, 0, &VU1Draw3D_CodeStart, &VU1Draw3D_CodeEnd);
	packet2_utils_vu_add_end_tag(packet2);
	dma_channel_send_packet2(packet2, DMA_CHANNEL_VIF1, 1);
	dma_channel_wait(DMA_CHANNEL_VIF1, 0);
	packet2_free(packet2);
}

int main(int argc, char *argv[])
{

	// Init DMA channels.
	dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
	dma_channel_initialize(DMA_CHANNEL_VIF1, NULL, 0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);
	dma_channel_fast_waits(DMA_CHANNEL_VIF1);

	// Initialize vif packets
	zbyszek_packet = packet2_create(10, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	vif_packets[0] = packet2_create(11, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);
	vif_packets[1] = packet2_create(11, P2_TYPE_NORMAL, P2_MODE_CHAIN, 1);

	vu1_upload_micro_program();
	vu1_set_double_buffer_settings();

	// The buffers to be used.
	framebuffer_t frame;
	zbuffer_t z;
	texbuffer_t texbuff;

	// Init the GS, framebuffer, zbuffer, and texture buffer.
	init_gs(&frame, &z, &texbuff);

	// Init the drawing environment and framebuffer.
	init_drawing_environment(&frame, &z);

	// Load the texture into vram.
	send_texture(&texbuff);

	// Render textured cube
	render(&frame, &z, &texbuff);

	packet2_free(vif_packets[0]);
	packet2_free(vif_packets[1]);
	packet2_free(zbyszek_packet);

	// Sleep
	SleepThread();

	// End program.
	return 0;
}
