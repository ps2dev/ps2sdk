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

#ifndef __GRAPH_REGISTERS_H__
#define __GRAPH_REGISTERS_H__

 #include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

 //////////////////////////
 // GS CONTROL REGISTERS //
 //////////////////////////

 #define GS_REG_BGCOLOR		*(volatile u64 *)0x120000E0	// Background Color Setting
 #define GS_REG_BUSDIR		*(volatile u64 *)0x12001040	// Host I/F Bus Switching
 #define GS_REG_CSR		*(volatile u64 *)0x12001000	// System Status
 #define GS_REG_DISPFB1		*(volatile u64 *)0x12000070	// Setting For Rectangular Area Read Output Circuit 1
 #define GS_REG_DISPFB2		*(volatile u64 *)0x12000090	// Setting For Rectangular Area Read Output Circuit 2
 #define GS_REG_DISPLAY1	*(volatile u64 *)0x12000080	// Setting For Rectangular Area Read Output Circuit 1
 #define GS_REG_DISPLAY2	*(volatile u64 *)0x120000A0	// Setting For Rectangular Area Read Output Circuit 2
 #define GS_REG_EXTBUF		*(volatile u64 *)0x120000B0	// Setting For Feedback Buffer Write Buffer
 #define GS_REG_EXTDATA		*(volatile u64 *)0x120000C0	// Feedback Write Setting
 #define GS_REG_EXTWRITE	*(volatile u64 *)0x120000D0	// Feedback Write Function Control
 #define GS_REG_IMR		*(volatile u64 *)0x12001010	// Interrupt Mask Control
 #define GS_REG_PMODE		*(volatile u64 *)0x12000000	// PCRTC Mode Setting
 #define GS_REG_SIGLBLID	*(volatile u64 *)0x12001080	// Signal ID Value Read
 #define GS_REG_SMODE1		*(volatile u64 *)0x12000010	// VHP,VCKSEL,SLCK2,NVCK,CLKSEL,PEVS,PEHS,PVS,PHS,GCONT,SPML,PCK2,XPCK,SINT,PRST,EX,CMOD,SLCK,T1248,LC,RC
 #define GS_REG_SMODE2		*(volatile u64 *)0x12000020	// Setting For Modes Related to Video Synchronization
 #define GS_REG_SRFSH		*(volatile u64 *)0x12000030	// DRAM Refresh Settings
 #define GS_REG_SYNCH1		*(volatile u64 *)0x12000040	// HS,HSVS,HSEQ,HBP,HFP
 #define GS_REG_SYNCH2		*(volatile u64 *)0x12000050	// HB,HF
 #define GS_REG_SYNCHV		*(volatile u64 *)0x12000060	// VS,VDP,VBPE,VBP,VFPE,VFP

 #define GS_SET_BGCOLOR(A,B,C) \
  (u64)((A) & 0x000000FF) <<  0 | (u64)((B) & 0x000000FF) <<  8 | \
  (u64)((C) & 0x000000FF) << 16

 #define GS_SET_BUSDIR(A) (u64)((A) & 0x00000001)

 #define GS_SET_CSR(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u64)((A) & 0x00000001) <<  0 | (u64)((B) & 0x00000001) <<  1 | \
  (u64)((C) & 0x00000001) <<  2 | (u64)((D) & 0x00000001) <<  3 | \
  (u64)((E) & 0x00000001) <<  4 | (u64)((F) & 0x00000001) <<  8 | \
  (u64)((G) & 0x00000001) <<  9 | (u64)((H) & 0x00000001) << 12 | \
  (u64)((I) & 0x00000001) << 13 | (u64)((J) & 0x00000003) << 14 | \
  (u64)((K) & 0x000000FF) << 16 | (u64)((L) & 0x000000FF) << 24

 #define GS_SET_DISPFB(A,B,C,D,E) \
  (u64)((A) & 0x000001FF) <<  0 | (u64)((B) & 0x0000003F) <<  9 | \
  (u64)((C) & 0x0000001F) << 15 | (u64)((D) & 0x000007FF) << 32 | \
  (u64)((E) & 0x000007FF) << 43

 #define GS_SET_DISPLAY(A,B,C,D,E,F) \
  (u64)((A) & 0x00000FFF) <<  0 | (u64)((B) & 0x000007FF) << 12 | \
  (u64)((C) & 0x0000000F) << 23 | (u64)((D) & 0x00000003) << 27 | \
  (u64)((E) & 0x00000FFF) << 32 | (u64)((F) & 0x000007FF) << 44

 #define GS_SET_EXTBUF(A,B,C,D,E,F,G,H) \
  (u64)((A) & 0x00003FFF) <<  0 | (u64)((B) & 0x0000003F) << 14 | \
  (u64)((C) & 0x00000003) << 20 | (u64)((D) & 0x00000001) << 22 | \
  (u64)((E) & 0x00000003) << 23 | (u64)((F) & 0x00000003) << 25 | \
  (u64)((G) & 0x000007FF) << 32 | (u64)((H) & 0x000007FF) << 43

 #define GS_SET_EXTDATA(A,B,C,D,E,F) \
  (u64)((A) & 0x00000FFF) <<  0 | (u64)((B) & 0x000007FF) << 12 | \
  (u64)((C) & 0x0000000F) << 23 | (u64)((D) & 0x00000003) << 27 | \
  (u64)((E) & 0x00000FFF) << 32 | (u64)((F) & 0x000007FF) << 44

 #define GS_SET_EXTWRITE(A) (u64)((A) & 0x00000001)

 #define GS_SET_IMR(A,B,C,D,E) \
  (u64)((A) & 0x00000001) <<  8 | (u64)((B) & 0x00000001) <<  9 | \
  (u64)((C) & 0x00000001) << 10 | (u64)((D) & 0x00000001) << 11 | \
  (u64)((E) & 0x00000001) << 12 | (u64)((1) & 0x00000001) << 13 | \
  (u64)((1) & 0x00000001) << 14

 #define GS_SET_PMODE(A,B,C,D,E,F) \
  (u64)((A) & 0x00000001) <<  0 | (u64)((B) & 0x00000001) <<  1 | \
  (u64)((1) & 0x00000007) <<  2 | (u64)((C) & 0x00000001) <<  5 | \
  (u64)((D) & 0x00000001) <<  6 | (u64)((E) & 0x00000001) <<  7 | \
  (u64)((F) & 0x000000FF) <<  8

 #define GS_SET_SIGLBLID(A,B) \
  (u64)((A) & 0xFFFFFFFF) <<  0 | (u64)((B) & 0xFFFFFFFF) << 32

 #define GS_SET_SMODE1(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) <<  3 | \
  (u64)((C) & 0x0000FFFF) << 10 | (u64)((D) & 0x0000FFFF) << 12 | \
  (u64)((E) & 0x0000FFFF) << 13 | (u64)((F) & 0x0000FFFF) << 15 | \
  (u64)((G) & 0x0000FFFF) << 16 | (u64)((H) & 0x0000FFFF) << 17 | \
  (u64)((I) & 0x0000FFFF) << 18 | (u64)((J) & 0x0000FFFF) << 19 | \
  (u64)((K) & 0x0000FFFF) << 21 | (u64)((L) & 0x0000FFFF) << 25 | \
  (u64)((M) & 0x0000FFFF) << 26 | (u64)((N) & 0x0000FFFF) << 27 | \
  (u64)((O) & 0x0000FFFF) << 28 | (u64)((P) & 0x0000FFFF) << 29 | \
  (u64)((Q) & 0x0000FFFF) << 30 | (u64)((R) & 0x0000FFFF) << 32 | \
  (u64)((S) & 0x0000FFFF) << 33 | (u64)((T) & 0x0000FFFF) << 34 | \
  (u64)((U) & 0x0000FFFF) << 36

 #define GS_SET_SMODE2(A,B,C) \
  (u64)((A) & 0x00000001) <<  0 | (u64)((B) & 0x00000001) <<  1 | \
  (u64)((C) & 0x00000003) <<  2

 #define GS_SET_SRFSH(A) (u64)((A) & 0x00000000)

 #define GS_SET_SYNCH1(A,B,C,D,E) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 11 | \
  (u64)((C) & 0x0000FFFF) << 22 | (u64)((D) & 0x0000FFFF) << 32 | \
  (u64)((E) & 0x0000FFFF) << 43

 #define GS_SET_SYNCH2(A,B) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 11

 #define GS_SET_SYNCHV(A,B,C,D,E,F) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 10 | \
  (u64)((C) & 0x0000FFFF) << 20 | (u64)((D) & 0x0000FFFF) << 32 | \
  (u64)((E) & 0x0000FFFF) << 42 | (u64)((F) & 0x0000FFFF) << 53

 ///////////////////////////
 // GIF CONTROL REGISTERS //
 ///////////////////////////

 #define GIF_REG_CTRL		*(volatile u32 *)0x10003000	// Control Register
 #define GIF_REG_MODE		*(volatile u32 *)0x10003010	// Mode Setting Register
 #define GIF_REG_STAT		*(volatile u32 *)0x10003020	// Status Register
 #define GIF_REG_TAG0		*(volatile u32 *)0x10003040	// GIFtag Save Register
 #define GIF_REG_TAG1		*(volatile u32 *)0x10003050	// GIFtag Save Register
 #define GIF_REG_TAG2		*(volatile u32 *)0x10003060	// GIFtag Save Register
 #define GIF_REG_TAG3		*(volatile u32 *)0x10003070	// GIFtag Save Register
 #define GIF_REG_CNT		*(volatile u32 *)0x10003080	// Count Register
 #define GIF_REG_P3CNT		*(volatile u32 *)0x10003090	// PATH3 Count Register
 #define GIF_REG_P3TAG		*(volatile u32 *)0x100030A0	// PATH3 Tag Register

 #define GIF_SET_CTRL(A,B) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  3

 #define GIF_SET_MODE(A,B) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  2

 #define GIF_SET_STAT(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  5 | (u32)((F) & 0x00000001) <<  6 | \
  (u32)((G) & 0x00000001) <<  7 | (u32)((H) & 0x00000001) <<  8 | \
  (u32)((I) & 0x00000001) <<  9 | (u32)((J) & 0x00000003) << 10 | \
  (u32)((K) & 0x00000001) << 12 | (u32)((L) & 0x0000001F) << 24

 #define GIF_SET_TAG0(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x00000001) <<  9 | (u32)((H) & 0x00000001) << 12 | \
  (u32)((I) & 0x00000001) << 13 | (u32)((J) & 0x00000001) << 14 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 24

 #define GIF_SET_TAG1(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x00000001) <<  9 | (u32)((H) & 0x00000001) << 12 | \
  (u32)((I) & 0x00000001) << 13 | (u32)((J) & 0x00000001) << 14 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 24

 #define GIF_SET_TAG2(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x00000001) <<  9 | (u32)((H) & 0x00000001) << 12 | \
  (u32)((I) & 0x00000001) << 13 | (u32)((J) & 0x00000001) << 14 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 24

 #define GIF_SET_TAG3(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x00000001) <<  9 | (u32)((H) & 0x00000001) << 12 | \
  (u32)((I) & 0x00000001) << 13 | (u32)((J) & 0x00000001) << 14 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 24

 #define GIF_SET_CNT(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u32)((A) & 0x00000001) <<  0 | (u32)((B) & 0x00000001) <<  1 | \
  (u32)((C) & 0x00000001) <<  2 | (u32)((D) & 0x00000001) <<  3 | \
  (u32)((E) & 0x00000001) <<  4 | (u32)((F) & 0x00000001) <<  8 | \
  (u32)((G) & 0x00000001) <<  9 | (u32)((H) & 0x00000001) << 12 | \
  (u32)((I) & 0x00000001) << 13 | (u32)((J) & 0x00000001) << 14 | \
  (u32)((K) & 0x00000001) << 16 | (u32)((L) & 0x00000001) << 24

 //////////////////////////
 // GIF PACKET REGISTERS //
 //////////////////////////

 #define GIF_REG_ALPHA_1	0x42	// Alpha blending setting. (Context 1)
 #define GIF_REG_ALPHA_2	0x43	// Alpha blending setting. (Context 2)
 #define GIF_REG_BITBLTBUF	0x50	// Setting for transmissions between buffers.
 #define GIF_REG_CLAMP_1	0x08	// Texture wrap mode. (Context 1)
 #define GIF_REG_CLAMP_2	0x09	// Texture wrap mode. (Context 2)
 #define GIF_REG_COLCLAMP	0x46	// Color clamp control.
 #define GIF_REG_DIMX		0x44	// Dither matrix setting.
 #define GIF_REG_DTHE		0x45	// Dither control.
 #define GIF_REG_FBA_1		0x4A	// Alpha correction value. (Context 1)
 #define GIF_REG_FBA_2		0x4B	// Alpha correction value. (Context 2)
 #define GIF_REG_FINISH		0x61	// Finish event occurence request.
 #define GIF_REG_FOG		0x0A	// Vertex fog value setting.
 #define GIF_REG_FOGCOL		0x3D	// Distant fog color setting.
 #define GIF_REG_FRAME_1	0x4C	// Frame buffer setting. (Context 1)
 #define GIF_REG_FRAME_2	0x4D	// Frame buffer setting. (Context 2)
 #define GIF_REG_HWREG		0x54	// Data port for transmission between buffers.
 #define GIF_REG_LABEL		0x62	// Label event occurence request.
 #define GIF_REG_MIPTBP1_1	0x34	// Mipmap information setting for levels 1 - 3. (Context 1)
 #define GIF_REG_MIPTBP1_2	0x35	// Mipmap information setting for levels 1 - 3. (Context 2)
 #define GIF_REG_MIPTBP2_1	0x36	// Mipmap information setting for levels 4 - 6. (Context 1)
 #define GIF_REG_MIPTBP2_2	0x37	// Mipmap information setting for levels 4 - 6. (Context 2)
 #define GIF_REG_PABE		0x49	// Alpha blending control in units of pixels.
 #define GIF_REG_PRIM		0x00	// Drawing primitive setting.
 #define GIF_REG_PRMODE		0x1B	// Setting for attributes of drawing primitives.
 #define GIF_REG_PRMODECONT	0x1A	// Specification of primitive attribute setting method.
 #define GIF_REG_RGBAQ		0x01	// Vertex color setting.
 #define GIF_REG_SCANMSK	0x22	// Raster address mask setting.
 #define GIF_REG_SCISSOR_1	0x40	// Setting for scissoring area. (Context 1)
 #define GIF_REG_SCISSOR_2	0x41	// Setting for scissoring area. (Context 2)
 #define GIF_REG_SIGNAL		0x60	// Signal event occurence request.
 #define GIF_REG_ST		0x02	// Specification of vertex texture coordinates.
 #define GIF_REG_TEST_1		0x47	// Pixel test control. (Context 1)
 #define GIF_REG_TEST_2		0x48	// Pixel test control. (Context 2)
 #define GIF_REG_TEX0_1		0x06	// Texture information setting. (Context 1)
 #define GIF_REG_TEX0_2		0x07	// Texture information setting. (Context 2)
 #define GIF_REG_TEX1_1		0x14	// Texture information setting. (Context 1)
 #define GIF_REG_TEX1_2		0x15	// Texture information setting. (Context 2)
 #define GIF_REG_TEX2_1		0x16	// Texture information setting. (Context 1)
 #define GIF_REG_TEX2_2		0x17	// Texture information setting. (Context 2)
 #define GIF_REG_TEXA		0x3B	// Texture alpha value setting.
 #define GIF_REG_TEXCLUT	0x1C	// Clut position specification.
 #define GIF_REG_TEXFLUSH	0x3F	// Texture page buffer disabling.
 #define GIF_REG_TRXDIR		0x53	// Activation of transmission area in buffers.
 #define GIF_REG_TRXPOS		0x51	// Specification of transmission area in buffers.
 #define GIF_REG_TRXREG		0x52	// Specification of transmission area in buffers.
 #define GIF_REG_UV		0x03	// Specification of vertex texture coordinates.
 #define GIF_REG_XYOFFSET_1	0x18	// Offset value setting. (Context 1)
 #define GIF_REG_XYOFFSET_2	0x19	// Offset value setting. (Context 2)
 #define GIF_REG_XYZ2		0x05	// Setting for vertex coordinate values.
 #define GIF_REG_XYZ3		0x0D	// Setting for vertex coordinate values. (Without Drawing Kick)
 #define GIF_REG_XYZF2		0x04	// Setting for vertex coordinate values.
 #define GIF_REG_XYZF3		0x0C	// Setting for vertex coordinate values. (Without Drawing Kick)
 #define GIF_REG_ZBUF_1		0x4E	// Z-Buffer setting. (Context 1)
 #define GIF_REG_ZBUF_2		0x4F	// Z-Buffer setting. (Context 2)

 #define GIF_SET_ALPHA(A,B,C,D,E) \
  (u64)((A) & 0x00000003) <<  0 | (u64)((B) & 0x00000003) <<  2 | \
  (u64)((C) & 0x00000003) <<  4 | (u64)((D) & 0x00000003) <<  6 | \
  (u64)((E) & 0x000000FF) << 32

 #define GIF_SET_BITBLTBUF(A,B,C,D,E,F) \
  (u64)((A) & 0x00003FFF) <<  0 | (u64)((B) & 0x0000003F) << 16 | \
  (u64)((C) & 0x0000003F) << 24 | (u64)((D) & 0x00003FFF) << 32 | \
  (u64)((E) & 0x0000003F) << 48 | (u64)((F) & 0x0000003F) << 56

 #define GIF_SET_CLAMP(A,B,C,D,E,F) \
  (u64)((A) & 0x00000003) <<  0 | (u64)((B) & 0x00000003) <<  2 | \
  (u64)((C) & 0x000003FF) <<  4 | (u64)((D) & 0x000003FF) << 14 | \
  (u64)((E) & 0x000003FF) << 24 | (u64)((F) & 0x000003FF) << 34

 #define GIF_SET_COLCLAMP(A) (u64)((A) & 0x00000001)

 #define GIF_SET_DIMX(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) \
  (u64)((A) & 0x00000003) <<  0 | (u64)((B) & 0x00000003) <<  4 | \
  (u64)((C) & 0x00000003) <<  8 | (u64)((D) & 0x00000003) << 12 | \
  (u64)((E) & 0x00000003) << 16 | (u64)((F) & 0x00000003) << 20 | \
  (u64)((G) & 0x00000003) << 24 | (u64)((H) & 0x00000003) << 28 | \
  (u64)((I) & 0x00000003) << 32 | (u64)((J) & 0x00000003) << 36 | \
  (u64)((K) & 0x00000003) << 40 | (u64)((L) & 0x00000003) << 44 | \
  (u64)((M) & 0x00000003) << 48 | (u64)((N) & 0x00000003) << 52 | \
  (u64)((O) & 0x00000003) << 56 | (u64)((P) & 0x00000003) << 60

 #define GIF_SET_DTHE(A) (u64)((A) & 0x00000001)

 #define GIF_SET_FBA(A) (u64)((A) & 0x00000001)

 #define GIF_SET_FOG(A) (u64)((A) & 0x000000FF) << 56

 #define GIF_SET_FOGCOL(A,B,C) \
  (u64)((A) & 0x000000FF) <<  0 | (u64)((B) & 0x000000FF) <<  8 | \
  (u64)((C) & 0x000000FF) << 16

 #define GIF_SET_FRAME(A,B,C,D) \
  (u64)((A) & 0x000001FF) <<  0 | (u64)((B) & 0x0000003F) << 16 | \
  (u64)((C) & 0x0000003F) << 24 | (u64)((D) & 0xFFFFFFFF) << 32

 #define GIF_SET_HWREG(A) (u64)((A) & 0xFFFFFFFFFFFFFFFF)

 #define GIF_SET_LABEL(A,B) \
  (u64)((A) & 0xFFFFFFFF) <<  0 | (u64)((B) & 0xFFFFFFFF) << 32

 #define GIF_SET_MIPTBP1(A,B,C,D,E,F) \
  (u64)((A) & 0x000003FF) <<  0 | (u64)((B) & 0x0000003F) << 14 | \
  (u64)((C) & 0x000003FF) << 20 | (u64)((D) & 0x0000003F) << 34 | \
  (u64)((E) & 0x000003FF) << 40 | (u64)((F) & 0x0000003F) << 54

 #define GIF_SET_MIPTBP2(A,B,C,D,E,F) \
  (u64)((A) & 0x000003FF) <<  0 | (u64)((B) & 0x0000003F) << 14 | \
  (u64)((C) & 0x000003FF) << 20 | (u64)((D) & 0x0000003F) << 34 | \
  (u64)((E) & 0x000003FF) << 40 | (u64)((F) & 0x0000003F) << 54

 #define GIF_SET_PABE(A) (u64)((A) & 0x00000001)

 #define GIF_SET_PRIM(A,B,C,D,E,F,G,H,I) \
  (u64)((A) & 0x00000007) <<  0 | (u64)((B) & 0x00000001) <<  3 | \
  (u64)((C) & 0x00000001) <<  4 | (u64)((D) & 0x00000001) <<  5 | \
  (u64)((E) & 0x00000001) <<  6 | (u64)((F) & 0x00000001) <<  7 | \
  (u64)((G) & 0x00000001) <<  8 | (u64)((H) & 0x00000001) <<  9 | \
  (u64)((I) & 0x00000001) << 10

 #define GIF_SET_PRMODE(A,B,C,D,E,F,G,H) \
  (u64)((A) & 0x00000001) <<  3 | (u64)((B) & 0x00000001) <<  4 | \
  (u64)((C) & 0x00000001) <<  5 | (u64)((D) & 0x00000001) <<  6 | \
  (u64)((E) & 0x00000001) <<  7 | (u64)((F) & 0x00000001) <<  8 | \
  (u64)((G) & 0x00000001) <<  9 | (u64)((H) & 0x00000001) << 10

 #define GIF_SET_PRMODECONT(A) (u64)((A) & 0x00000001)

 #define GIF_SET_RGBAQ(A,B,C,D,E) \
  (u64)((A) & 0x000000FF) <<  0 | (u64)((B) & 0x000000FF) <<  8 | \
  (u64)((C) & 0x000000FF) << 16 | (u64)((D) & 0x000000FF) << 24 | \
  (u64)((E) & 0xFFFFFFFF) << 32

 #define GIF_SET_SCANMSK(A) (u64)((A) & 0x00000003)

 #define GIF_SET_SCISSOR(A,B,C,D) \
  (u64)((A) & 0x000007FF) <<  0 | (u64)((B) & 0x000007FF) << 16 | \
  (u64)((C) & 0x000007FF) << 32 | (u64)((D) & 0x000007FF) << 48

 #define GIF_SET_SIGNAL(A,B) \
  (u64)((A) & 0xFFFFFFFF) <<  0 | (u64)((B) & 0xFFFFFFFF) << 32

 #define GIF_SET_ST(A,B) \
  (u64)((A) & 0xFFFFFFFF) <<  0 | (u64)((B) & 0xFFFFFFFF) << 32

 #define GIF_SET_TEST(A,B,C,D,E,F,G,H) \
  (u64)((A) & 0x00000001) <<  0 | (u64)((B) & 0x00000007) <<  1 | \
  (u64)((C) & 0x000000FF) <<  4 | (u64)((D) & 0x00000003) << 12 | \
  (u64)((E) & 0x00000001) << 14 | (u64)((F) & 0x00000001) << 15 | \
  (u64)((G) & 0x00000001) << 16 | (u64)((H) & 0x00000003) << 17

 #define GIF_SET_TEX0(A,B,C,D,E,F,G,H,I,J,K,L) \
  (u64)((A) & 0x00003FFF) <<  0 | (u64)((B) & 0x0000003F) << 14 | \
  (u64)((C) & 0x0000003F) << 20 | (u64)((D) & 0x0000000F) << 26 | \
  (u64)((E) & 0x0000000F) << 30 | (u64)((F) & 0x00000001) << 34 | \
  (u64)((G) & 0x00000003) << 35 | (u64)((H) & 0x00003FFF) << 37 | \
  (u64)((I) & 0x0000000F) << 51 | (u64)((J) & 0x00000001) << 55 | \
  (u64)((K) & 0x0000001F) << 56 | (u64)((L) & 0x00000007) << 61

 #define GIF_SET_TEX1(A,B,C,D,E,F,G) \
  (u64)((A) & 0x00000001) <<  0 | (u64)((B) & 0x00000007) <<  2 | \
  (u64)((C) & 0x00000001) <<  5 | (u64)((D) & 0x00000007) <<  6 | \
  (u64)((E) & 0x00000001) <<  9 | (u64)((F) & 0x00000003) << 19 | \
  (u64)((G) & 0x000007FF) << 32

 #define GIF_SET_TEX2(A,B,C,D,E,F) \
  (u64)((A) & 0x0000003F) << 20 | (u64)((B) & 0x00003FFF) << 37 | \
  (u64)((C) & 0x0000000F) << 51 | (u64)((D) & 0x00000001) << 55 | \
  (u64)((E) & 0x0000001F) << 56 | (u64)((F) & 0x00000007) << 61

 #define GIF_SET_TEXA(A,B,C) \
  (u64)((A) & 0x000000FF) <<  0 | (u64)((B) & 0x00000001) << 15 | \
  (u64)((C) & 0x000000FF) << 32

 #define GIF_SET_TEXCLUT(A,B,C) \
  (u64)((A) & 0x0000003F) <<  0 | (u64)((B) & 0x0000003F) <<  6 | \
  (u64)((C) & 0x00000FFF) << 12

 #define GIF_SET_TRXDIR(A) (u64)((A) & 0x00000003)

 #define GIF_SET_TRXPOS(A,B,C,D,E) \
  (u64)((A) & 0x000007FF) <<  0 | (u64)((B) & 0x000007FF) << 16 | \
  (u64)((C) & 0x000007FF) << 32 | (u64)((D) & 0x000007FF) << 48 | \
  (u64)((E) & 0x00000003) << 59

 #define GIF_SET_TRXREG(A,B) \
  (u64)((A) & 0x00000FFF) <<  0 | (u64)((B) & 0x00000FFF) << 32

 #define GIF_SET_UV(A,B) \
  (u64)((A) & 0x00003FFF) <<  0 | (u64)((B) & 0x00003FFF) << 16

 #define GIF_SET_XYOFFSET(A,B) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 32

 #define GIF_SET_XYZ(A,B,C) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 16 | \
  (u64)((C) & 0xFFFFFFFF) << 32

 #define GIF_SET_XYZF(A,B,C,D) \
  (u64)((A) & 0x0000FFFF) <<  0 | (u64)((B) & 0x0000FFFF) << 16 | \
  (u64)((C) & 0x00FFFFFF) << 32 | (u64)((D) & 0x000000FF) << 56

 #define GIF_SET_ZBUF(A,B,C) \
  (u64)((A) & 0x000001FF) <<  0 | (u64)((B) & 0x0000000F) << 24 | \
  (u64)((C) & 0x00000001) << 32

 ///////////////////////
 // GIF TAG REGISTERS //
 ///////////////////////

 #define GIF_TAG_PACKED		0x00
 #define GIF_TAG_REGLIST	0x01
 #define GIF_TAG_IMAGE		0x02

 #define GIF_SET_TAG(A,B,C,D,E,F) \
  (u64)((A) & 0x00007FFF) <<  0 | (u64)((B) & 0x00000001) << 15 | \
  (u64)((C) & 0x00000001) << 46 | (u64)((D) & 0x000007FF) << 47 | \
  (u64)((E) & 0x00000003) << 58 | (u64)((F) & 0x0000000F) << 60

#ifdef __cplusplus
}
#endif

#endif
