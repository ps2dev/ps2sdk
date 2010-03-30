#ifndef __GS_PSM_H__
#define __GS_PSM_H__

 // Pixel-Storage Methods
 #define GS_PSM_32		0x00	// 32 bits per pixel.
 #define GS_PSM_24		0x01	// 24 bits per pixel.
 #define GS_PSM_16		0x02	// 16 bits per pixel.
 #define GS_PSM_16S		0x0A	// 16 bits per pixel.
 #define GS_PSM_PS24	0x12	// 24 bits per pixel. 
 #define GS_PSM_8		0x13	// 8 bits per pixel, palettized.
 #define GS_PSM_4		0x14	// 4 bits per pixel, palettized.
 #define GS_PSM_8H		0x1B	// 8 bits per pixel, 24 to 32
 #define GS_PSM_4HL		0x24	// 4 bits per pixel, 28 to 32 
 #define GS_PSM_4HH		0x2C	// 4 bits per pixel, 24 to 27 
 #define GS_PSMZ_32		0x30	// 32 bits per pixel. 
 #define GS_PSMZ_24		0x31	// 24 bits per pixel.
 #define GS_PSMZ_16		0x32	// 16 bits per pixel.
 #define GS_PSMZ_16S	0x3A	// 16 bits per pixel.

  // ZBuffer Setting
 #define GS_ZBUF_32		0x00	// 32 bit zbuffer
 #define GS_ZBUF_24		0x01	// 24 bit zbuffer
 #define GS_ZBUF_16		0x02	// 16 bit zbuffer
 #define GS_ZBUF_16S	0x0A	// 32/24 bit compatible 16 bit zbuffer
 
#endif /*__GS_PSM_H__*/
