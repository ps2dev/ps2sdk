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

#ifndef __GRAPH_H__
#define __GRAPH_H__

 #include <tamtypes.h>

 #include <graph_registers.h>

#ifdef __cplusplus
extern "C" {
#endif

 #define GRAPH_MODE_NTSC	0x00	//  640 x  448
 #define GRAPH_MODE_PAL		0x01	//  640 x  512
 #define GRAPH_MODE_HDTV_480P	0x02	//  640 x  480
 #define GRAPH_MODE_HDTV_720P	0x03	// 1280 x  720
 #define GRAPH_MODE_HDTV_1080I	0x04	// 1920 x 1080
 #define GRAPH_MODE_VGA_640	0x05	//  640 x  480
 #define GRAPH_MODE_VGA_800	0x06	//  800 x  600
 #define GRAPH_MODE_VGA_1024	0x07	// 1024 x  768
 #define GRAPH_MODE_VGA_1280	0x08	// 1280 x 1024

 #define GRAPH_PSM_32		0x00	// 32 bits per pixel.
 #define GRAPH_PSM_24		0x01	// 24 bits per pixel.
 #define GRAPH_PSM_16		0x02	// 16 bits per pixel.
 #define GRAPH_PSM_8		0x13	// 8 bits per pixel, palettized.
 #define GRAPH_PSM_4		0x14	// 4 bits per pixel, palettized.
 #define GRAPH_PSM_8H		0x1B	// 8 bits per pixel, palettized.
 #define GRAPH_PSM_4HH		0x24	// 4 bits per pixel, palettized.
 #define GRAPH_PSM_4HL		0x2C	// 4 bits per pixel, palettized.

 typedef struct { int width, height, mode, interlace, size; u64 display; } GRAPH_MODE;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void);

 int graph_shutdown(void);

 /////////////////////////
 // GRAPH GET FUNCTIONS //
 /////////////////////////

 int graph_get_bpp(void);

 int graph_get_height(void);

 int graph_get_interlace(void);

 int graph_get_mode(GRAPH_MODE *mode);

 int graph_get_psm(void);

 int graph_get_size(void);

 int graph_get_width(void);

 int graph_get_zbpp(void);

 int graph_get_zpsm(void);

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_clearbuffer(int red, int green, int blue);

 int graph_set_displaybuffer(int address);

 int graph_set_drawbuffer(int address);

 int graph_set_mode(int mode, int psm, int zpsm);

 int graph_set_zbuffer(int address);

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 int graph_vram_read(int address, int width, int height, int psm, void *data, int data_size);

 int graph_vram_write(int address, int width, int height, int psm, void *data, int data_size);

 //////////////////////////
 // GRAPH WAIT FUNCTIONS //
 //////////////////////////

 int graph_wait_hsync(void);

 int graph_wait_vsync(void);

#ifdef __cplusplus
}
#endif

#endif
