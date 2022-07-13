#include <dma.h>

#include <stdio.h>
#include <string.h>
#include <tamtypes.h>

#include <packet.h>
#include <graph.h>
#include <gs_psm.h>
#include <draw.h>
#include <kernel.h>

#include <font.h>

extern unsigned int  image_clut32[];
extern unsigned char  image_pixel[];

int myaddress = 0;
int clutaddress = 0;

fsfont_t impress;
fontx_t krom_u;
fontx_t krom_k;

void draw_init_env()
{

	framebuffer_t frame;
	zbuffer_t z;

	packet_t *packet = packet_init(16,PACKET_NORMAL);

	qword_t *q = packet->data;

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
	dma_wait_fast();

	packet_free(packet);

}

void init_texture()
{
	packet_t *packet = packet_init(50,PACKET_NORMAL);

	qword_t *q;

	myaddress = graph_vram_allocate(512,256,GS_PSM_4, GRAPH_ALIGN_BLOCK);
	clutaddress = graph_vram_allocate(8,2,GS_PSM_32, GRAPH_ALIGN_BLOCK);

	q = packet->data;

	q = draw_texture_transfer(q,image_pixel,512,256,GS_PSM_4,myaddress,512);
	q = draw_texture_transfer(q,image_clut32,8,2,GS_PSM_32,clutaddress,64);
	q = draw_texture_flush(q);

	dma_channel_send_chain(DMA_CHANNEL_GIF,packet->data, q - packet->data, 0,0);
	dma_wait_fast();

	packet_free(packet);

}

void run_demo(packet_t *packet)
{

	int context = 0;

	vertex_t v0;

	color_t c0;
	color_t c1;

	texbuffer_t texbuf;
	clutbuffer_t clut;
	lod_t lod;

	packet_t *packets[2];
	packet_t *current;

	packets[0] = packet_init(10000,PACKET_NORMAL);
	packets[1] = packet_init(10000,PACKET_NORMAL);

	// Use linear filtering for good scaling results
	lod.calculation = LOD_USE_K;
	lod.max_level = 0;
	lod.mag_filter = LOD_MAG_LINEAR;
	lod.min_filter = LOD_MIN_LINEAR;
	lod.l = 0;
	lod.k = 0;

	texbuf.width = 512;
	texbuf.psm = GS_PSM_4;
	texbuf.address = myaddress;

	texbuf.info.width = draw_log2(512);
	texbuf.info.height = draw_log2(256);
	texbuf.info.components = TEXTURE_COMPONENTS_RGBA;
	texbuf.info.function = TEXTURE_FUNCTION_MODULATE;

	clut.storage_mode = CLUT_STORAGE_MODE1;
	clut.start = 0;

	clut.psm = GS_PSM_32;
	clut.load_method = CLUT_LOAD;
	clut.address = clutaddress;

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

	// UTF-8
	unsigned char str0[] = { 0x61, 0x62, 0xC2, 0xA9, 0x78, 0xC2, 0xA5, 0xC2, 0xB2, '\0'};

	// Shift-JIS
	unsigned char str1[] = {0x81, 0xBC, 0x93, 0xF1, 0x93, 0xF1, 0x93, 0xF1, 0x81, 0x69, 0x81, 0x40, 0x81,
							0x4F, 0x83, 0xD6, 0x81, 0x4F, 0x81, 0x6A, 0x93, 0xF1, 0x81, 0xBD, 0x0D, '\0' };

	while(1)
	{
		qword_t *q;

		current = packets[context];
		q = current->data;

		q = draw_clear(q,0,0,0,640.0f,448.0f,0x40,0x40,0x40);

		q = draw_texture_sampling(q,0,&lod);
		q = draw_texturebuffer(q,0,&texbuf,&clut);

		impress.scale = 3.0f;

		q = fontx_print_sjis(q,0,str1,CENTER_ALIGN,&v0,&c0,&krom_u,&krom_k);
		q = fontstudio_print_string(q,0,str0,CENTER_ALIGN,&v0,&c1,&impress);

		q = draw_finish(q);

		dma_wait_fast();
		dma_channel_send_normal(DMA_CHANNEL_GIF,current->data, q - current->data, 0,0);

		draw_wait_finish();

		context ^= 1;

		graph_wait_vsync();

	}

	free(packets[0]);
	free(packets[1]);
}

int main(void)
{
	char *ini;
	packet_t packet;

	dma_channel_initialize(DMA_CHANNEL_GIF,NULL,0);
	dma_channel_fast_waits(DMA_CHANNEL_GIF);

	fontx_load("rom0:KROM", &krom_u, SINGLE_BYTE, 2, 1, 1);
	fontx_load("rom0:KROM", &krom_k, DOUBLE_BYTE, 2, 1, 1);

	if((ini = fontstudio_load_ini("host:impress.ini")) != NULL)
	{
		fontstudio_parse_ini(&impress, ini, 512, 256);
		free(ini);

		draw_init_env();

		init_texture();

		run_demo(&packet);

		fontstudio_unload_ini(&impress);

		fontx_unload(&krom_u);
		fontx_unload(&krom_k);
	} else {
		printf("Error: cannot load ini file.\n");
	}

	SleepThread();

	return 0;
}
