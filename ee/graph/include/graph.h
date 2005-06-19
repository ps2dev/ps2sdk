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

 #include <graph_registers.h>

 typedef struct { int width, height, mode, interlace, size; u64 display; } GRAPH_MODE;

#ifdef __cplusplus
extern "C" {
#endif

 #define GRAPH_MODE_NTSC		0	//  640 x  448
 #define GRAPH_MODE_PAL			1	//  640 x  512
 #define GRAPH_MODE_HDTV_480P		2	//  640 x  480
 #define GRAPH_MODE_HDTV_720P		3	// 1280 x  720
 #define GRAPH_MODE_HDTV_1080I		4	// 1920 x 1080
 #define GRAPH_MODE_VGA_640		5	//  640 x  480 @ 60hz
 #define GRAPH_MODE_VGA_640_60		5	//  640 x  480 @ 60hz
 #define GRAPH_MODE_VGA_640_72		6	//  640 x  480 @ 72hz
 #define GRAPH_MODE_VGA_640_75		7	//  640 x  480 @ 75hz
 #define GRAPH_MODE_VGA_640_85		8	//  640 x  480 @ 85hz
 #define GRAPH_MODE_VGA_800		10	//  800 x  600 @ 60hz
 #define GRAPH_MODE_VGA_800_56		9	//  800 x  600 @ 56hz
 #define GRAPH_MODE_VGA_800_60		10	//  800 x  600 @ 60hz
 #define GRAPH_MODE_VGA_800_72		11	//  800 x  600 @ 72hz
 #define GRAPH_MODE_VGA_800_75		12	//  800 x  600 @ 75hz
 #define GRAPH_MODE_VGA_800_85		13	//  800 x  600 @ 85hz
 #define GRAPH_MODE_VGA_1024		14	// 1024 x  768 @ 60hz
 #define GRAPH_MODE_VGA_1024_60		14	// 1024 x  768 @ 60hz
 #define GRAPH_MODE_VGA_1024_70		15	// 1024 x  768 @ 70hz
 #define GRAPH_MODE_VGA_1024_75		16	// 1024 x  768 @ 75hz
 #define GRAPH_MODE_VGA_1024_85		17	// 1024 x  768 @ 85hz
 #define GRAPH_MODE_VGA_1280		18	// 1280 x 1024 @ 60hz
 #define GRAPH_MODE_VGA_1280_60		18	// 1280 x 1024 @ 60hz
 #define GRAPH_MODE_VGA_1280_75		19	// 1280 x 1024 @ 75hz
 #define GRAPH_MODE_AUTO		99	// Automatic NTSC or PAL mode setting.

 #define GRAPH_PSM_32			0x00	// 32 bits per pixel.
 #define GRAPH_PSM_24			0x01	// 24 bits per pixel.
 #define GRAPH_PSM_16			0x02	// 16 bits per pixel.
 #define GRAPH_PSM_16S			0x0A	// 16 bits per pixel.
 #define GRAPH_PSM_8			0x13	// 8 bits per pixel, palettized.
 #define GRAPH_PSM_4			0x14	// 4 bits per pixel, palettized.
 #define GRAPH_PSM_8H			0x1B	// 8 bits per pixel, palettized.
 #define GRAPH_PSM_4HH			0x24	// 4 bits per pixel, palettized.
 #define GRAPH_PSM_4HL			0x2C	// 4 bits per pixel, palettized.

 #define GRAPH_NONINTERLACED		0x00	// The current mode is non-interlaced.
 #define GRAPH_INTERLACED		0x01	// The current mode is interlaced.

 #define GRAPH_FIELD_EVEN		0x00	// Use even fields only.
 #define GRAPH_FIELD_ODD		0x01	// Use odd fields only.
 #define GRAPH_FIELD_BOTH		0x02	// Use both field types.

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void);
 // Initialize the graphics library and hardware.

 int graph_shutdown(void);
 // Shut down the graphics library and hardware.

 /////////////////////////
 // GRAPH GET FUNCTIONS //
 /////////////////////////

 float graph_get_aspect(void);
 // Returns the current aspect ratio. (value : 1)

 int graph_get_bpp(void);
 // Returns the framebuffer bits-per-pixel.

 int graph_get_displaybuffer(void);
 // Returns the vram address of the display buffer, in bytes.

 int graph_get_displayfield(void);
 // Returns the currently displayed field value.
 // Used mainly for interlaced video modes.

 int graph_get_drawbuffer(void);
 // Returns the vram address of the draw buffer, in bytes.

 int graph_get_drawfield(void);
 // Returns the current draw field value.
 // Used mainly for interlaced video modes.

 int graph_get_height(void);
 // Returns the framebuffer height, in pixels.

 int graph_get_interlace(void);
 // Returns the interlacing value for the current mode.

 int graph_get_psm(void);
 // Returns the framebuffer pixel storage method.

 int graph_get_size(void);
 // Returns the size of the framebuffer, in bytes.

 int graph_get_width(void);
 // Returns the framebuffer width, in pixels.

 int graph_get_zbpp(void);
 // Returns the zbuffer bits-per-pixel.

 int graph_get_zbuffer(void);
 // Returns the vram address of the zbuffer, in bytes.

 int graph_get_zpsm(void);
 // Returns the zbuffer pixel storage method.

 int graph_get_zsize(void);
 // Returns the size of the zbuffer, in bytes.

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_clearbuffer(int red, int green, int blue);
 // Clears the draw buffer with the specified colour.

 int graph_set_displaybuffer(int address);
 // Sets the vram address of the display buffer, in bytes.

 int graph_set_drawbuffer(int address);
 // Sets the vram address of the draw buffer, in bytes.

 int graph_set_drawfield(int field);
 // Sets the draw field value.
 // Used mainly for interlaced video modes.

 int graph_set_mode(int mode, int psm, int zpsm);
 // Sets the graphics mode.

 int graph_set_texture(int address, int width, int height, int psm);
 // Sets the texture information.

 int graph_set_zbuffer(int address);
 // Sets the vram address of the zbuffer, in bytes.

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 int graph_vram_read(int address, int width, int height, int psm, void *data, int data_size);
 // Uploads data to the vram. Address is specified in bytes.

 int graph_vram_write(int address, int width, int height, int psm, void *data, int data_size);
 // Uploads data to vram. Address is specified in bytes.

 //////////////////////////
 // GRAPH WAIT FUNCTIONS //
 //////////////////////////

 int graph_wait_hsync(void);
 // Wait for a horizontal sync event to occur.

 int graph_wait_vsync(void);
 // Wait for a vertical sync event to occur.

#ifdef __cplusplus
}
#endif

#endif
