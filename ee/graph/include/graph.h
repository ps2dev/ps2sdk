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

#ifdef __cplusplus
extern "C" {
#endif

 #define GRAPH_MODE_NTSC	0x00	// NTSC (640x448i)
 #define GRAPH_MODE_PAL		0x01	// PAL  (640x512i)
 #define GRAPH_MODE_HDTV	0x02	// HDTV (720x480p)
 #define GRAPH_MODE_VGA		0x03	// VGA  (640x480p)

 #define GRAPH_BPP_32		0x00	// 32 bits per pixel.
 #define GRAPH_BPP_24		0x01	// 24 bits per pixel, aligned to 32 bits.
 #define GRAPH_BPP_16		0x02	// 16 bits per pixel.
 #define GRAPH_BPP_16S		0x0A	// ??
 #define GRAPH_BPP_8		0x13	// 8 bits per pixel, palettized.
 #define GRAPH_BPP_4		0x14	// 4 bits per pixel, palettized.
 #define GRAPH_BPP_8H		0x1B	// 8 bits per pixel, palettized. Goes well with 24BPP.
 #define GRAPH_BPP_4H		0x24	// 4 bits per pixel, palettized. Goes well with 24BPP.
 #define GRAPH_BPP_4L		0x2C	// 4 bits per pixel, palettized. Goes well with 24BPP.
 #define GRAPH_BPP_32Z		0x30	// ??
 #define GRAPH_BPP_24Z		0x31	// ??
 #define GRAPH_BPP_16Z		0x32	// ??
 #define GRAPH_BPP_16ZS		0x3A	// ??

 typedef struct { int width, height, mode, interlace, dx, dy, magh, magv, dw, dh; } GRAPH_MODE;

 typedef struct { int size, count; u64 *data; } GRAPH_PACKET;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void);

 int graph_shutdown(void);

 ////////////////////////////
 // GRAPH PACKET FUNCTIONS //
 ////////////////////////////

 int graph_packet_allocate(GRAPH_PACKET *packet, int size);

 int graph_packet_clear(GRAPH_PACKET *packet);

 int graph_packet_append(GRAPH_PACKET *packet, u64 data);

 int graph_packet_send(GRAPH_PACKET *packet);

 int graph_packet_free(GRAPH_PACKET *packet);

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_displaymode(int mode, int bpp);

 int graph_set_displaybuffer(int address);

 int graph_set_drawbuffer(int address, int width, int height, int bpp);

 int graph_set_zbuffer(int address, int bpp);

 int graph_set_clearbuffer(int red, int green, int blue, int alpha);

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 int graph_vram_read(int address, int width, int height, int bpp, void *buffer);

 int graph_vram_write(int address, int width, int height, int bpp, void *buffer);

 ///////////////////////////
 // GRAPH VSYNC FUNCTIONS //
 ///////////////////////////

 int graph_vsync_wait(void);

 int graph_vsync_handler_add(void *handler);

 int graph_vsync_handler_remove(void);

#ifdef __cplusplus
}
#endif

#endif
