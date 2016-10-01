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

#include <draw.h>
#include <draw3d.h>

#include "flower.c"
#include "mesh_data.c"

extern unsigned char flower[];

VECTOR object_position = {0.00f, 0.00f, 0.00f, 1.00f};
VECTOR object_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

VECTOR camera_position = {0.00f, 0.00f, 100.00f, 1.00f};
VECTOR camera_rotation = {0.00f, 0.00f, 0.00f, 1.00f};

void init_gs(framebuffer_t *frame, zbuffer_t *z, texbuffer_t *texbuf)
{

    // Define a 32-bit 640x512 framebuffer.
    frame->width = 640;
    frame->height = 512;
    frame->mask = 0;
    frame->psm = GS_PSM_32;
    frame->address = graph_vram_allocate(frame->width, frame->height, frame->psm, GRAPH_ALIGN_PAGE);

    // Enable the zbuffer.
    z->enable = DRAW_ENABLE;
    z->mask = 0;
    z->method = ZTEST_METHOD_GREATER_EQUAL;
    z->zsm = GS_ZBUF_32;
    z->address = graph_vram_allocate(frame->width, frame->height, z->zsm, GRAPH_ALIGN_PAGE);

    // Allocate some vram for the texture buffer
    texbuf->width = 256;
    texbuf->psm = GS_PSM_24;
    texbuf->address = graph_vram_allocate(256, 256, GS_PSM_24, GRAPH_ALIGN_BLOCK);

    // Initialize the screen and tie the first framebuffer to the read circuits.
    graph_initialize(frame->address, frame->width, frame->height, frame->psm, 0, 0);
}

void init_drawing_environment(framebuffer_t *frame, zbuffer_t *z)
{

    packet_t *packet = packet_init(20, PACKET_NORMAL);

    // This is our generic qword pointer.
    qword_t *q = packet->data;

    // This will setup a default drawing environment.
    q = draw_setup_environment(q, 0, frame, z);

    // Now reset the primitive origin to 2048-width/2,2048-height/2.
    q = draw_primitive_xyoffset(q, 0, (2048 - 320), (2048 - 256));

    // Finish setting up the environment.
    q = draw_finish(q);

    // Now send the packet, no need to wait since it's the first.
    dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
    dma_wait_fast();

    packet_free(packet);
}

void load_texture(texbuffer_t *texbuf)
{

    packet_t *packet = packet_init(50, PACKET_NORMAL);

    qword_t *q = packet->data;

    q = packet->data;

    q = draw_texture_transfer(q, flower, 256, 256, GS_PSM_24, texbuf->address, texbuf->width);
    q = draw_texture_flush(q);

    dma_channel_send_chain(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
    dma_wait_fast();

    packet_free(packet);
}

void setup_texture(texbuffer_t *texbuf)
{

    packet_t *packet = packet_init(10, PACKET_NORMAL);

    qword_t *q = packet->data;

    // Using a texture involves setting up a lot of information.
    clutbuffer_t clut;

    lod_t lod;

    lod.calculation = LOD_USE_K;
    lod.max_level = 0;
    lod.mag_filter = LOD_MAG_NEAREST;
    lod.min_filter = LOD_MIN_NEAREST;
    lod.l = 0;
    lod.k = 0;

    texbuf->info.width = draw_log2(256);
    texbuf->info.height = draw_log2(256);
    texbuf->info.components = TEXTURE_COMPONENTS_RGB;
    texbuf->info.function = TEXTURE_FUNCTION_DECAL;

    clut.storage_mode = CLUT_STORAGE_MODE1;
    clut.start = 0;
    clut.psm = 0;
    clut.load_method = CLUT_NO_LOAD;
    clut.address = 0;

    q = draw_texture_sampling(q, 0, &lod);
    q = draw_texturebuffer(q, 0, texbuf, &clut);

    // Now send the packet, no need to wait since it's the first.
    dma_channel_send_normal(DMA_CHANNEL_GIF, packet->data, q - packet->data, 0, 0);
    dma_wait_fast();

    packet_free(packet);
}

int render(framebuffer_t *frame, zbuffer_t *z)
{

    int i;
    int context = 0;

    packet_t *packets[2];
    packet_t *current;

    qword_t *q;
    u64 *dw;

    MATRIX local_world;
    MATRIX world_view;
    MATRIX view_screen;
    MATRIX local_screen;

    prim_t prim;
    color_t color;

    VECTOR *temp_vertices;

    xyz_t *xyz;
    color_t *rgbaq;
    texel_t *st;

    packets[0] = packet_init(100, PACKET_NORMAL);
    packets[1] = packet_init(100, PACKET_NORMAL);

    // Define the triangle primitive we want to use.
    prim.type = PRIM_TRIANGLE;
    prim.shading = PRIM_SHADE_GOURAUD;
    prim.mapping = DRAW_ENABLE;
    prim.fogging = DRAW_DISABLE;
    prim.blending = DRAW_ENABLE;
    prim.antialiasing = DRAW_DISABLE;
    prim.mapping_type = PRIM_MAP_ST;
    prim.colorfix = PRIM_UNFIXED;

    color.r = 0x80;
    color.g = 0x80;
    color.b = 0x80;
    color.a = 0x40;
    color.q = 1.0f;

    // Allocate calculation space.
    temp_vertices = memalign(128, sizeof(VECTOR) * vertex_count);

    // Allocate register space.
    xyz = memalign(128, sizeof(u64) * vertex_count);
    rgbaq = memalign(128, sizeof(u64) * vertex_count);
    st = memalign(128, sizeof(u64) * vertex_count);

    // Create the view_screen matrix.
    create_view_screen(view_screen, graph_aspect_ratio(), -3.00f, 3.00f, -3.00f, 3.00f, 1.00f, 2000.00f);

    // The main loop...
    for (;;) {

        current = packets[context];

        // Spin the cube a bit.
        object_rotation[0] += 0.008f;
        while (object_rotation[0] > 3.14f) {
            object_rotation[0] -= 6.28f;
        }
        object_rotation[1] += 0.012f;
        while (object_rotation[1] > 3.14f) {
            object_rotation[1] -= 6.28f;
        }

        // Create the local_world matrix.
        create_local_world(local_world, object_position, object_rotation);

        // Create the world_view matrix.
        create_world_view(world_view, camera_position, camera_rotation);

        // Create the local_screen matrix.
        create_local_screen(local_screen, local_world, world_view, view_screen);

        // Calculate the vertex values.
        calculate_vertices(temp_vertices, vertex_count, vertices, local_screen);

        // Generate the XYZ register values.
        draw_convert_xyz(xyz, 2048, 2048, 32, vertex_count, (vertex_f_t *)temp_vertices);

        // Convert floating point colours to fixed point.
        draw_convert_rgbq(rgbaq, vertex_count, (vertex_f_t *)temp_vertices, (color_f_t *)colours, color.a);

        // Generate the ST register values.
        draw_convert_st(st, vertex_count, (vertex_f_t *)temp_vertices, (texel_f_t *)coordinates);

        q = current->data;

        // Clear framebuffer but don't update zbuffer.
        q = draw_disable_tests(q, 0, z);
        q = draw_clear(q, 0, 2048.0f - 320.0f, 2048.0f - 256.0f, frame->width, frame->height, 0x40, 0x40, 0x40);
        q = draw_enable_tests(q, 0, z);

        // Draw the triangles using triangle primitive type.
        // Use a 64-bit pointer to simplify adding data to the packet.
        dw = (u64 *)draw_prim_start(q, 0, &prim, &color);

        for (i = 0; i < points_count; i++) {
            *dw++ = rgbaq[points[i]].rgbaq;
            *dw++ = st[points[i]].uv;
            *dw++ = xyz[points[i]].xyz;
        }

        // Check if we're in middle of a qword or not.
        if ((u32)dw % 16) {

            *dw++ = 0;
        }

        // Only 3 registers rgbaq/st/xyz were used (standard STQ reglist)
        q = draw_prim_end((qword_t *)dw, 3, DRAW_STQ_REGLIST);

        // Setup a finish event.
        q = draw_finish(q);

        // Now send our current dma chain.
        dma_wait_fast();
        dma_channel_send_normal(DMA_CHANNEL_GIF, current->data, q - current->data, 0, 0);

        // Now switch our packets so we can process data while the DMAC is working.
        context ^= 1;

        // Wait for scene to finish drawing
        draw_wait_finish();

        graph_wait_vsync();
    }

    free(packets[0]);
    free(packets[1]);

    // End program.
    return 0;
}

int main(int argc, char **argv)
{

    // The buffers to be used.
    framebuffer_t frame;
    zbuffer_t z;
    texbuffer_t texbuf;

    // Init GIF dma channel.
    dma_channel_initialize(DMA_CHANNEL_GIF, NULL, 0);
    dma_channel_fast_waits(DMA_CHANNEL_GIF);

    // Init the GS, framebuffer, zbuffer, and texture buffer.
    init_gs(&frame, &z, &texbuf);

    // Init the drawing environment and framebuffer.
    init_drawing_environment(&frame, &z);

    // Load the texture into vram.
    load_texture(&texbuf);

    // Setup texture buffer
    setup_texture(&texbuf);

    // Render textured cube
    render(&frame, &z);

    // Sleep
    SleepThread();

    // End program.
    return 0;
}
