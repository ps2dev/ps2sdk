#include <dma.h>

#include <stdio.h>
#include <string.h>
#include <tamtypes.h>

#include <packet.h>
#include <graph.h>
#include <graph_vram.h>
#include <gs_psm.h>
#include <draw.h>

#include <fsfont.h>
#include <fontx.h>

extern unsigned int  image_clut32[];
extern unsigned char  image_pixel[];

int myaddress = 0;
int clutaddress = 0;

FSFONT impress;
FONTX krom_u;
FONTX krom_k;

void draw_init_env(PACKET *packet)
{

	QWORD *q = packet->data;

	FRAMEBUFFER frame;
	ZBUFFER z;

	frame.width = 640;
	frame.height = 448;
	frame.psm = GS_PSM_32;
	frame.mask = 0;
	frame.address = graph_vram_allocate(frame.width,frame.height,frame.psm,GRAPH_ALIGN_PAGE);

	z.enable = 0;
	z.method = ZTEST_METHOD_GREATER;
	z.address = 0;
	z.mask = 1;
	z.zsm = 0;

	graph_initialize(frame.address,640,448,GS_PSM_32,0,0);

	q = draw_setup_environment(q,0,&frame,&z);

	q = draw_finish(q);

	dma_channel_send_normal(DMA_CHANNEL_GIF,packet->data, q - packet->data, 0,0);

}

void init_texture(PACKET *packet)
{

	dma_wait_fast();

	QWORD *q = packet->data;

	myaddress = graph_vram_allocate(512,256,GS_PSM_4, GRAPH_ALIGN_BLOCK);
	clutaddress = graph_vram_allocate(8,2,GS_PSM_32, GRAPH_ALIGN_BLOCK);

	q = packet->data;

	q = draw_texture_transfer(q,image_pixel,512*256/2,512,256,GS_PSM_4,myaddress,512);
	q = draw_texture_transfer(q,image_clut32,8*2*4,8,2,GS_PSM_32,clutaddress,64);
	q = draw_texture_flush(q);

	dma_channel_send_chain(DMA_CHANNEL_GIF,packet->data, q - packet->data, 0,0);

}

void test_something(PACKET *packet)
{

	int context = 0;

	PACKET *current = packet;

	QWORD *q = current->data;

	VERTEX v0;

	COLOR c0;
	COLOR c1;

	TEXEL t0;
	TEXEL t1;

	TEXBUFFER texbuf;
	CLUTBUFFER clut;
	TEXTURE texinfo;
	LOD lod;

	// Use linear filtering for good scaling results
	lod.calculation = LOD_USE_K;
	lod.max_level = 0;
	lod.mag_filter = LOD_MAG_LINEAR;
	lod.min_filter = LOD_MIN_LINEAR;
	lod.l = 0;
	lod.k = 0;

	texinfo.width = draw_log2(512);
	texinfo.height = draw_log2(256);
	texinfo.components = TEXTURE_COMPONENTS_RGBA;
	texinfo.function = TEXTURE_FUNCTION_MODULATE;

	texbuf.width = 512;
	texbuf.psm = GS_PSM_4;
	texbuf.address = myaddress;

	clut.storage_mode = CLUT_STORAGE_MODE1;
	clut.start = 0;

	clut.psm = GS_PSM_32;
	clut.load_method = CLUT_LOAD;
	clut.address = clutaddress;

	t0.u = 0.5f;
	t0.v = 0.5f;

	t1.u = 510.5f;
	t1.v = 254.5f;

	v0.x = 320.0f;
	v0.y = 240.0f;
	v0.z = 4;

	c0.r = 0xFF;
	c0.g = 0xFF;
	c0.b = 0xFF;
	c0.a = 0x80;
	c0.q = 1.0f;

	c1.r = 0xFF;
	c1.g = 0x00;
	c1.b = 0x00;
	c1.a = 0x40;
	c1.q = 1.0f;
	dma_wait_fast();

	// UTF-8
	unsigned char str0[] = { 0x61, 0x62, 0xC2, 0xA9, 0x78, 0xC2, 0xA5, 0xC2, 0xB2, '\0'};

	// Shift-JIS
	unsigned char str1[] = {0x81, 0xBC, 0x93, 0xF1, 0x93, 0xF1, 0x93, 0xF1, 0x81, 0x69, 0x81, 0x40, 0x81,
							0x4F, 0x83, 0xD6, 0x81, 0x4F, 0x81, 0x6A, 0x93, 0xF1, 0x81, 0xBD, 0x0D, '\0' };

	while(1)
	{

		q = draw_clear(q,0,0,0,640.0f,448.0f,0x40,0x40,0x40);

		q = draw_texture_sampling(q,0,&lod);
		q = draw_texturebuffer(q,0,&texbuf,&texinfo,&clut);

		impress.scale = 3.0f;

		q = fontx_print_sjis(q,0,str1,CENTER_ALIGN,&v0,&c0,&krom_u,&krom_k);
		q = fontstudio_print_string(q,0,str0,CENTER_ALIGN,&v0,&c1,&impress);

		q = draw_finish(q);

		dma_wait_fast();
		dma_channel_send_normal(DMA_CHANNEL_GIF,current->data, q - current->data, 0,0);

		draw_wait_finish();

		context ^= 1;

		current = packet + context;
		q = current->data;

		graph_wait_vsync();

	}

}

int main(void)
{
	PACKET packet;
	PACKET packets[2];

	packet_allocate(&packet,100,0,0);
	packet_allocate(&packets[0],10000,0,0);
	packet_allocate(&packets[1],10000,0,0);

	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	load_fontx("rom0:KROM", &krom_u, SINGLE_BYTE, 2, 1, 1);
	load_fontx("rom0:KROM", &krom_k, DOUBLE_BYTE, 2, 1, 1);

	load_fontstudio_ini(&impress,"host:impress.ini", 512, 256,20);

	draw_init_env(&packet);

	init_texture(&packet);

	test_something(packets);

	unload_fontstudio_ini(&impress);

	unload_fontx(&krom_u);
	unload_fontx(&krom_k);

	return 0;
}
