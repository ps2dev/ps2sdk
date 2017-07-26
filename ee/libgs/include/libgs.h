/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * GS library functions.
 */

#ifndef __LIBGS_H__
#define __LIBGS_H__

#include <tamtypes.h>

typedef struct {
	/** Interlace/non-interlace mode. */
	u8 interlace;
	/** Video mode. */
	u8 omode;
	/** FIELD/FRAME value. */
	u8 ffmode;
	/** GS version. */
	u8 version;
} GsGParam_t;

/** Resets the GS and GIF. */
#define GS_INIT_RESET		0
/** Drawing operations are cancelled and primitive data will be discarded. */
#define GS_INIT_DRAW_RESET	1

#define GS_NONINTERLACED	0x00
#define GS_INTERLACED		0x01

/** Read every other line from the beginning with the start of FIELD. */
#define GS_FFMD_FIELD		0x00
/** Read every line from the beginning with the start of FRAME. */
#define GS_FFMD_FRAME		0x01

/*	About the supported video modes:
		As GsSetDefaultDisplayEnv() has been modified to provide functionality that is similar to the Sony sceGsSetDefDispEnv() function,
		it will now automatically in fill in the GS_DISPENV structure with values for the video mode that is specified for GsResetGraph().

	However, as with the Sony function:
		1. Only NTSC, PAL and 480P video modes are supported.
		2. MAGV isn't automatically calculated.

	It is possible to cover these limitations by setting the relevant values after calling GsSetDefaultDisplayEnv(), but I do not know how these values are to be calculated for other video modes.
*/
enum GsVideoModes{
	GS_MODE_NTSC		= 0x02,
	GS_MODE_PAL,

	GS_MODE_DTV_480P 	= 0x50,
};

#define GS_DISABLE	0
#define GS_ENABLE	1

/** types of primitives */
enum GsPrimitiveTypes{
	GS_PRIM_POINT		=0,
	GS_PRIM_LINE,
	GS_PRIM_LINE_STRIP,
	GS_PRIM_TRI,
	GS_PRIM_TRI_STRIP,
	GS_PRIM_TRI_FAN,
	GS_PRIM_SPRITE
};

/** regular Pixel Storage Modes (PSM) */
#define GS_PIXMODE_32		 0
#define GS_PIXMODE_24		 1
#define GS_PIXMODE_16		 2
#define GS_PIXMODE_16S		10

/** clut Pixel Storage Modes (PSM) */
#define GS_CLUT_32			 0
#define GS_CLUT_16			 2
#define GS_CLUT_16S			10

/** texture/image Pixel Storage Modes (PSM) */
#define GS_TEX_32			 0
#define GS_TEX_24			 1
#define GS_TEX_16			 2
#define GS_TEX_16S			10
#define GS_TEX_8			19
#define GS_TEX_4			20
#define GS_TEX_8H			27
#define GS_TEX_4HL			36
#define GS_TEX_4HH			44

/** Z-Buffer Pixel Storage Modes (PSM) */
#define GS_ZBUFF_32			48
#define GS_ZBUFF_24			49
#define GS_ZBUFF_16			50
#define GS_ZBUFF_16S		58

/** Alpha test Methods */
enum GsATestMethods{
	GS_ALPHA_NEVER		=0,
	GS_ALPHA_ALWAYS,
	GS_ALPHA_LESS,
	GS_ALPHA_LEQUAL,
	GS_ALPHA_EQUAL,
	GS_ALPHA_GEQUAL,
	GS_ALPHA_GREATER,
	GS_ALPHA_NOTEQUAL,
};

/** Alpha test failed update Methods */
enum GsATestFailedUpdateMethods{
	/** standard */
	GS_ALPHA_NO_UPDATE	=0,
	GS_ALPHA_FB_ONLY,
	GS_ALPHA_ZB_ONLY,
	GS_ALPHA_RGB_ONLY
};

/** Zbuffer test Methods */
enum GsZTestMethodTypes{
	GS_ZBUFF_NEVER		=0,
	GS_ZBUFF_ALWAYS,
	GS_ZBUFF_GEQUAL,
	GS_ZBUFF_GREATER
};

// Texture Details
/** use near/far formula */
#define GS_TEX_CALC	0
/** fixed value (use K value) */
#define GS_TEX_FIXED	1

enum GsTexMipmaps{
	/** no mipmap */
	GS_TEX_MIPMAP0	=0,
	/** 1 mipmap */
	GS_TEX_MIPMAP1,
	/** 2 mipmaps */
	GS_TEX_MIPMAP2,
	/** 3 mipmaps */
	GS_TEX_MIPMAP3,
	/** 4 mipmaps */
	GS_TEX_MIPMAP4,
	/** 5 mipmaps */
	GS_TEX_MIPMAP5,
	/** 6 mipmaps */
	GS_TEX_MIPMAP6
};

enum GsTexFilterMethods{
	/** UnFiltered */
	GS_TEX_NEAREST			=0,
	/** Filtered */
	GS_TEX_LINEAR,
	GS_TEX_NEAREST_MIPMAP_NEAREST,
	GS_TEX_NEAREST_MIPMAP_LINEAR,
	GS_TEX_LINEAR_MIPMAP_NEAREST,
	GS_TEX_LINEAR_MIPMAP_LINEAR
};

/** use values in MIPTBP1 */
#define GS_TEX_MIPMAP_DEFINE	0
/** auto calculate mipmap address */
#define GS_TEX_MIPMAP_AUTO	1

/** Texture Function (used in TEX0->tex_cc) */
enum GsTexFunctions{
	GS_TEX_MODULATE		=0,	/** brighten texture based on Pixel's Alpha */
	GS_TEX_DECAL,			/** keep texture as is */
	GS_TEX_HIGHLIHGT1,		/** used when highlighting translucent polygons */
	GS_TEX_HIGHLIHGT2		/** used when highlighting opaque polygons */
};

enum GsGifDataFormat{
	GS_GIF_PACKED	=0,
	GS_GIF_REGLIST,
	GS_GIF_IMAGE,
	/** Same operation with the IMAGE mode */
	GS_GIF_DISABLE
};

/*GS Privileged Regs*/
/** Setup CRT Controller */
#define gs_p_pmode			0x12000000
/** Video signal settings, undocumented (don't set!) */
#define gs_p_smode1			0x12000010
/** CRTC Video Settings: PAL/NTCS, Interlace, etc. */
#define gs_p_smode2			0x12000020
/** Setup the CRTC's Read Circuit 1 data source settings */
#define gs_p_dispfb1		0x12000070
/** RC1 display output settings */
#define gs_p_display1		0x12000080
/** Setup the CRTC's Read Circuit 2 data source settings */
#define gs_p_dispfb2		0x12000090
/** RC2 display output settings */
#define gs_p_display2		0x120000a0
#define gs_p_extbuf			0x120000b0
#define gs_p_extdata		0x120000c0
#define gs_p_extwrite		0x120000d0
/** Set CRTC background color */
#define gs_p_bgcolor		0x120000e0
/** System status and reset */
#define gs_p_csr			0x12001000
/** Interrupt Mask Register */
#define gs_p_imr			0x12001010
/** Set direction of data transmission FIFO */
#define gs_p_busdir			0x12001040
/** Signal\label value */
#define gs_p_siglblid		0x12001080


/*GS General Purpose Regs*/
/** Select and configure current drawing primitive */
#define gs_g_prim		0x00
/** Setup current vertex color */
#define gs_g_rgbaq		0x01
/** ST map */
#define gs_g_st			0x02
/** UV map */
#define gs_g_uv			0x03
/** Set vertex position and fog coefflcient (with draw kick) */
#define gs_g_xyzf2		0x04
/** Set vertex coordinate (with draw kick) */
#define gs_g_xyz2		0x05
/** Select current texture in context 1 */
#define gs_g_tex0_1		0x06
/** Select current texture in context 2 */
#define gs_g_tex0_2		0x07
/** Set texture wrap mode in context 1 */
#define gs_g_clamp_1	0x08
/** Set texture wrap mode in context 2 */
#define gs_g_clamp_2	0x09
/** Set fog attributes */
#define gs_g_fog		0x0a
/** Set vertex position and fog coefflcient (no draw kick) */
#define gs_g_xyzf3		0x0c
/** Set vertex position (no draw kick) */
#define gs_g_xyz3		0x0d
#define gs_g_tex1_1		0x14
#define gs_g_tex1_2		0x15
/** Set texture filtering\sampling style in context 1 */
#define gs_g_tex2_1		0x16
/** Set texture filtering\sampling style in context 2 */
#define gs_g_tex2_2		0x17
/** Mapping from Primitive to Window coordinate system (Context 1) */
#define gs_g_xyoffset_1	0x18
/** Mapping from Primitive to Window coordinate system (Context 2) */
#define gs_g_xyoffset_2	0x19
/** gs_g_prim or gs_g_prmode selector */
#define gs_g_prmodecont	0x1a
/** attributes of current drawing primitive */
#define gs_g_prmode		0x1b
#define gs_g_texclut	0x1c
/** Raster odd\even line drawing setting */
#define gs_g_scanmsk	0x22
/** Set mipmap address in context 1(mip level 1-3) */
#define gs_g_miptbp1_1	0x34
/** Set mipmap address in context 1(mip level 1-3) */
#define gs_g_miptbp1_2	0x35
/** Set mipmap address in context 2(mip level 4-6) */
#define gs_g_miptbp2_1	0x36
/** Set mipmap address in context 2(mip level 4-6) */
#define gs_g_miptbp2_2	0x37
/** Texture alpha setting */
#define gs_g_texa		0x3b
/** Set fog far color */
#define gs_g_fogcol		0x3d
/** Flush texture buffer/cache */
#define gs_g_texflush	0x3f
/** Setup clipping rectangle (Context 1) */
#define gs_g_scissor_1	0x40
/** Setup clipping rectangle (Context 2) */
#define gs_g_scissor_2	0x41
/** Alpha blending setting (Context 1) */
#define gs_g_alpha_1	0x42
/** Alpha blending setting (Context 2) */
#define gs_g_alpha_2	0x43
/** Dither matrix values */
#define gs_g_dimx		0x44
/** Enabel dither matrix */
#define gs_g_dthe		0x45
/** Color clamp control */
#define gs_g_colclamp	0x46
/** FrameBuffer\ZBuffer Pixel test contol (Context 1) */
#define gs_g_test_1		0x47
/** FrameBuffer\ZBuffer Pixel test contol (Context 2) */
#define gs_g_test_2		0x48
/** Enable alpha blending */
#define gs_g_pabe		0x49
/** Alpha correction value (Context 1) */
#define gs_g_fba_1		0x4a
/** Alpha correction value (Context 2) */
#define gs_g_fba_2		0x4b
/** Frame buffer settings (Context 1) */
#define gs_g_frame_1	0x4c
/** Frame buffer settings (Context 2) */
#define gs_g_frame_2	0x4d
/** Zbuffer configuration (Context 1) */
#define gs_g_zbuf_1		0x4e
/** Zbuffer configuration (Context 2) */
#define gs_g_zbuf_2		0x4f
/** Texture transmission address & format */
#define gs_g_bitbltbuf	0x50
/** Texture transmission coordinates */
#define gs_g_trxpos		0x51
/** Texture transmission width & height */
#define gs_g_trxreg		0x52
/** Texture transmission direction */
#define gs_g_trxdir		0x53
#define gs_g_hwreg		0x54
#define gs_g_signal		0x60
#define gs_g_finish		0x61
#define gs_g_label		0x62
/** no operation\does nothing\can be used as padding */
#define gs_g_nop		0x7f


/*GIF Register Descriptors (non-registers) */
/** A+D */
#define gif_rd_ad		0x0e
/** NOP (Not OutPut) */
#define gif_rd_nop		0x0f

/* MISC */

#ifndef QWORD
typedef struct {

	u64 lo;
	u64 hi;

}QWORD			__attribute__((aligned(16)));/*aligned 128bits*/

#endif/*QWORD*/

/* GS Privileged Reg STRUCTs */

typedef struct {
	/** Enable ReadCircuit 1 */
	u32 enable_rc1	:1;
	/** Enable ReadCircuit 2 */
	u32 enable_rc2	:1;
	/** CRT output switching(always 1) */
	u32 crt_out	:3;
	/** Value to use for alpha blend(0=value in 'RC1',1=value in 'blend_value') */
	u32 mmod		:1;
	/** ReadCircuit to output alpha to (0=RC1, 1=RC2) */
	u32 amod		:1;
	/** Blend Method(0=blend with RC2, 0=blend with BG) */
	u32 blend_style:1;
	/** Alpha Blend Value (0-255) */
	u32 blend_value:8;
	/** Output to NFIELD */
	u32 nfld		:1;
	/** Pad with zeros */
	u32 pad1		:15;
	/** ?? */
	u32 exvwins	:10;
	/** ?? */
	u32 exvwine	:10;
	/** ?? */
	u32 exsyncmd	:1;
	/** Pad with zeros */
	u32 pad2		:11;
}GS_PMODE;

typedef struct {
	u64 rc	:3;
	u64 lc	:7;
	u64 t1248	:2;
	u64 slck	:1;
	u64 cmod	:2;
	u64 ex	:1;
	u64 prst	:1;
	u64 sint	:1;
	u64 xpck	:1;
	u64 pck2	:2;
	u64 spml	:4;
	u64 gcont	:1;
	u64 phs	:1;
	u64 pvs	:1;
	u64 pehs	:1;
	u64 pevs	:1;
	u64 clksel	:2;
	u64 nvck	:1;
	u64 slck2	:1;
	u64 vcksel	:2;
	u64 vhp	:2;
	u64 pad	:26;
}GS_SMODE1;

typedef struct {
	u32 interlace   :1;
	u32 field_frame :1;
	u32 vesta_dpms  :2;
	u64 pad2	 :60;
}GS_SMODE2;

typedef struct {
	/** Base pointer in VRam */
	u32 address	:9;
	/** Buffer width in VRam */
	u32 fbw		:6;
	/** Pixel store mode */
	u32 psm	:5;
	/** Pad with zeros */
	u32 pad1		:12;
	/** X Pos in  in VRam */
	u32 x			:11;
	/** Y Pos in VRam */
	u32 y			:11;
	/** Pad with zeros */
	u32 pad2		:10;
}GS_DISPFB;

typedef struct {
	/** Display area X pos */
	u32 display_x	:12;
	/** Display area Y pos */
	u32 display_y	:11;
	/** Horizontal magnification */
	u32 magnify_h	:4;
	/** Vertical   magnification */
	u32 magnify_v	:2;
	/** Pad with zeros */
	u32 pad1	:3;
	/** Display area width */
	u32 display_w	:12;
	/** Display area height */
	u32 display_h	:11;
	/** Pad with zeros */
	u32 pad2     	:9;
}GS_DISPLAY;

typedef struct {
	u64 exbp	: 14;
	u64 exbw	: 6;
	u64 fbin	: 2;
	u64 wffmd	: 1;
	u64 emoda	: 2;
	u64 emodc	: 2;
	u64 pad1	: 5;
	u64 wdx		: 11;
	u64 wdy		: 11;
	u64 pad2	: 10;
}GS_EXTBUF;

typedef struct {
	/** X coord where image is written to */
	u32 x		:12;
	/** Y coord where image is written to */
	u32 y		:11;
	/** Horizontal Smaple Rate(VK units) */
	u32 sample_r_h	:4;
	/** Vertical   Smaple Rate */
	u32 sample_r_v	:2;
	/** Pad with zeros */
	u32 pad1	:3;
	/** Width  of area to write */
	u32 write_w	:12;
	/** Height of area to write */
	u32 write_h	:11;
	/** Pad with zeros */
	u32 pad2	:9;
}GS_EXTDATA;

typedef struct {
	/** Write Control(0=write done, 1=write start) */
	u32     write    :1;
	/** Pad with zeros */
	u32     pad1	 :31;
	/** Pad with zeros */
    u32 pad2;
}GS_EXTWRITE;

typedef struct {
	/** Background Color Red */
	u8	r;
	/** Background Color Green */
	u8	g;
	/** Background Color Blue */
	u8	b;
	/** 0x0 */
	u8	pada;
	/** 0x0 */
	float			padq;
}GS_BGCOLOR;

typedef struct {
	/** Signal event control(write: 0=nothing,1=enable signal event,  read: 0=signal not generated, 1=signal generated) */
	u32 signal_evnt    	:1;
	/** Finish event control(write: 0=nothing,1=enable finish event,  read: 0=finish not generated, 1=finish generated) */
	u32 finish_evnt    	:1;
	/** HSync interrupt ,,   ,,   ,,   ,, */
	u32 hsync_intrupt  	:1;
	/** VSync interrupt ,,    ,,   ,,   ,, */
	u32 vsync_intrupt  	:1;
	/** Write termination control ,,   ,,    ,,   ,, */
	u32 write_terminate	:1;
	/** ?? */
	u32 exhsint		:1;
	/** ?? */
	u32 exvsint		:1;
	/** Pad with zeros */
	u32 pad1		:1;
	/** Flush GS */
	u32 flush		:1;
	/** Reset GS */
	u32 reset		:1;
	/** ?? */
	u32 exverr		:1;
	/** ?? */
	u32 exfield		:1;
	/** NFIELD output */
	u32 nfield		:1;
	/** Currnet displayed field */
	u32 current_field 	:1;
	/** Host interface FIFO status */
	u32 fifo_status		:2;
	/** Revision number of GS */
	u32 gs_rev_number	:8;
	/** id of GS */
	u32 gs_id		:8;
	/** Pad with zeros */
	u32 pad2		:32;

}GS_CSR;

typedef struct {
	/** Pad with zeros */
	u32 pad1		:8;
	/** Signal event interrupt mask */
	u32 signal_mask		:1;
	/** Finish event interrupt mask */
	u32 finish_mask		:1;
	/** HSync interrupt mask */
	u32 hsync_mask		:1;
	/** VSync interrupt mask */
	u32 vsync_mask		:1;
	/** Write termination mask */
	u32 write_mask		:1;
	/** ?? */
	u32 exhs_mask		:1;
	/** ?? */
	u32 exvs_mask		:1;
	/** Pad with zeros */
	u32 pad2		:17;
    /** Pad with zeros */
    u32 pad3;			
}GS_IMR;

typedef struct {
	/** Transmission direction(0=host->local, 1=host<-local) */
	u32	direction	:1;
	/** Pad with zeros */
	u32	p0		:31;
	/** Pad with more zeros */
	u32	p1;
}GS_BUSDIR;

#if 0
typedef struct {
	/** SIGNAL register id */
	u32 id;
	u32 p0;
}GS_SIGLBLID;
#endif

/*
 * GENERAL PURPOSE REG STRUCTS
 *
 * these structs have a size of 64 bits
 */

typedef struct {
	u64 prim_type	:3;
	u64 iip		:1;
	u64 tme		:1;
	u64 fge		:1;
	u64 abe		:1;
	u64 aa1		:1;
	u64 fst		:1;
	u64 ctxt		:1;
	u64 fix		:1;
	u64 pad1		:53;
}GS_PRIM;

typedef struct {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
	float		  q;
}GS_RGBAQ;

typedef struct {
	float			s;
	float			t;
}GS_ST;

typedef struct {
	u64 u		:14;
	u64 pad1	:2;
	u64 v		:14;
	u64 pad2	:34;
}GS_UV;

typedef struct {
	u16	x;
	u16	y;
	u32	z:24;
	u8	f;
}GS_XYZF;

typedef struct {
	u16	x;
	u16	y;
	u32	z;
}GS_XYZ;

typedef struct {
	u64 tb_addr		:14;
	u64 tb_width		:6;
	u64 psm		:6;
	u64 tex_width		:4;
	u64 tex_height	:4;
	u64 tex_cc		:1;
	u64 tex_funtion	:2;
	u64 cb_addr		:14;
	u64 clut_pixmode	:4;
	u64 clut_smode	:1;
	u64 clut_offset	:5;
	u64 clut_loadmode	:3;

}GS_TEX0;

typedef struct {
	u64 wrap_mode_s	:2;
	u64 wrap_mode_t	:2;
	u64 min_clamp_u	:10;
	u64 max_clamp_u	:10;
	u64 min_clamp_v	:10;
	u64 max_clamp_v	:10;
	u64 pad0			:20;
}GS_CLAMP;

typedef struct {
		u64 pad1	:56;
		u8 f;
}GS_FOG;

typedef struct {
	u64 lcm			:1;
	u64 pad1			:1;
	u64 mxl			:3;
	u64 mmag			:1;
	u64 mmin			:3;
	u64 mtba			:1;
	u64 pad2			:9;
	u64 l				:2;
	u64 pad3			:11;
	u64 k				:12;
	u64 pad4			:20;
}GS_TEX1;

typedef struct {
	u64 pad1			:20;
	u64 psm			:6;
	u64 pad2			:11;
	u64 cb_addr		:14;
	u64 clut_psm		:4;
	u64 clut_smode	:1;
	u64 clut_offset	:5;
	u64 clut_loadmode	:3;
}GS_TEX2;

typedef struct {
	u64  offset_x	:16;
	u16 pad1;
	u64  offset_y	:16;
	u16 pad2;
} GS_XYOFFSET;

typedef struct {
	u64 control:1;
	u64 pad1	:63;
}GS_PRMODECONT;

typedef struct {
	u64 pad1	:3;
	u64 iip	:1;
	u64 tme	:1;
	u64 fge	:1;
	u64 abe	:1;
	u64 aa1	:1;
	u64 fst	:1;
	u64 ctxt	:1;
	u64 fix	:1;
	u64 pad2	:53;
}GS_PRMODE;

typedef struct {
	u64 cb_width		:6;
	u64 clut_uoffset	:6;
	u64 clut_voffset	:10;
	u64 pad0:42;
}GS_TEXCLUT;

typedef struct {
	u64 mask:2;
	u64 pad0:62;
}GS_SCANMSK;

typedef struct {
	u64 tb_addr1	:14;
	u64 tb_width1	:6;
	u64 tb_addr2	:14;
	u64 tb_width2	:6;
	u64 tb_addr3	:14;
	u64 tb_width3	:6;
	u64 pad1		:4;
}GS_MIPTBP1;

typedef struct {
	u64 tb_addr4	:14;
	u64 tb_width4	:6;
	u64 tb_addr5	:14;
	u64 tb_width5	:6;
	u64 tb_addr6	:14;
	u64 tb_width6	:6;
	u64 pad0		:4;
}GS_MIPTBP2;

typedef struct {
	u64 alpha_0		: 8;
	u64 pad1			: 7;
	u64 alpha_method	: 1;
	u64 pad2			:16;
	u64 alpha_1		: 8;
	u64 pad3			:24;
}GS_TEXA;

typedef struct {
	u64 r		:8;
	u64 g		:8;
	u64 b		:8;
	u64 pad1	:40;
}GS_FOGCOLOR;

typedef struct {
	/** Pad With Zeros */
	u64 pad1;
} GS_TEXFLUSH;

typedef struct {
	u64 clip_x0 :11;
	u64 pad1    :5;
	u64 clip_x1 :11;
	u64 pad2    :5;
	u64 clip_y0 :11;
	u64 pad3    :5;
	u64 clip_y1 :11;
	u64 pad4    :5;
}GS_SCISSOR;

typedef struct {
	u64 a		:2;
	u64 b		:2;
	u64 c		:2;
	u64 d		:2;
	u64 pad0	:24;
	u64 alpha	:8;
	u64 pad1	:24;
}GS_ALPHA;

typedef struct {
	u64 dimx00:3;
	u64 pad0:1;
	u64 dimx01:3;
	u64 pad1:1;
	u64 dimx02:3;
	u64 pad2:1;
	u64 dimx03:3;
	u64 pad3:1;

	u64 dimx10:3;
	u64 pad4:1;
	u64 dimx11:3;
	u64 pad5:1;
	u64 dimx12:3;
	u64 pad6:1;
	u64 dimx13:3;
	u64 pad7:1;

	u64 dimx20:3;
	u64 pad8:1;
	u64 dimx21:3;
	u64 pad9:1;
	u64 dimx22:3;
	u64 pad10:1;
	u64 dimx23:3;
	u64 pad11:1;

	u64 dimx30:3;
	u64 pad12:1;
	u64 dimx31:3;
	u64 pad13:1;
	u64 dimx32:3;
	u64 pad14:1;
	u64 dimx33:3;
	u64 pad15:1;
} GS_DIMX;

typedef struct {
	u64 enable:1;
	u64 pad01:63;
} GS_DTHE;

typedef struct {
	u64 clamp:1;
	u64 pad01:63;
}GS_COLCLAMP;

typedef struct {
	u64 atest_enable		:1;
	u64 atest_method		:3;
	u64 atest_reference	:8;
	u64 atest_fail_method	:2;
	u64 datest_enable		:1;
	u64 datest_mode		:1;
	u64 ztest_enable		:1;
	u64 ztest_method		:2;
	u64 pad1				:45;
} GS_TEST;

typedef struct {
	u64 enable:1;
	u64 pad0:63;
}GS_PABE;

typedef struct {
	u64 alpha:1;
	u64 pad0:63;
}GS_FBA;

typedef struct {
	u64 fb_addr	:9;
	u64 pad1		:7;
	u64 fb_width	:6;
	u64 pad2		:2;
	u64 psm	:6;
	u64 pad3		:2;
	u64 draw_mask	:32;
} GS_FRAME;

typedef struct {
	u64 fb_addr	:9;
	u64 pad1			:15;
	u64 psm		:4;
	u64 pad2			:4;
	u64 update_mask	:1;
	u64 pad3			:31;
}GS_ZBUF;

typedef struct {
	u64 src_addr	  :14;
	u64 pad1		  :2;
	u64 src_width	  :6;
	u64 pad2		  :2;
	u64 src_pixmode :6;
	u64 pad3		  :2;
	u64 dest_addr	  :14;
	u64 pad4		  :2;
	u64 dest_width  :6;
	u64 pad5		  :2;
	u64 dest_pixmode:6;
	u64 pad6		  :2;
}GS_BITBLTBUF;

typedef struct {
	u64 src_x		:11;
	u64 pad1		:5;
	u64 src_y		:11;
	u64 pad2		:5;
	u64 dest_x	:11;
	u64 pad3		:5;
	u64 dest_y	:11;
	u64 direction	:2;
	u64 pad4		:3;
}GS_TRXPOS;

typedef struct {
	u64 trans_w	:12;
	u64 pad1		:20;
	u64 trans_h	:12;
	u64 pad2		:20;
}GS_TRXREG;


typedef struct {
	u64 trans_dir	:2;
	u64 pad1		:62;
}GS_TRXDIR;

typedef struct {
	u64 data;
}GS_HWREG;

typedef struct {
	u32 signal_id;
	u32 update_mask;
}GS_SIGNAL;

typedef struct {
	u64 pad0;
}GS_FINISH;


typedef struct {
	u32 label_id;
	u32 update_mask;
}GS_LABEL;

typedef struct {
	u64 pad0;
}GS_NOP;

/*
 * GENERAL PURPOSE REG STRUCTS 'WITH' A 64 BIT REG
 *
 * these structs have a size of 128 bits (1 QWORD)
 */

typedef struct {
	GS_PRIM			data;
	u64	reg;
}GS_R_PRIM;

typedef struct {
	GS_RGBAQ		data;
	u64	reg;
}GS_R_RGBAQ;

typedef struct {
	GS_ST			data;
	u64	reg;
}GS_R_ST;

typedef struct {
	GS_UV			data;
	u64	reg;
}GS_R_UV;

typedef struct {
	GS_XYZF			data;
	u64	reg;
}GS_R_XYZF;

typedef struct {
	GS_XYZ			data;
	u64	reg;
}GS_R_XYZ;

typedef struct {
	GS_TEX0			data;
	u64	reg;
}GS_R_TEX0;

typedef struct {
	GS_CLAMP		data;
	u64	reg;
}GS_R_CLAMP;

typedef struct {
	GS_FOG			data;
	u64	reg;
}GS_R_FOG;

typedef struct {
	GS_TEX1			data;
	u64	reg;
}GS_R_TEX1;

typedef struct {
	GS_TEX2			data;
	u64	reg;
}GS_R_TEX2;

typedef struct {
	GS_XYOFFSET		data;
	u64	reg;
} GS_R_XYOFFSET;

typedef struct {
	GS_PRMODECONT	data;
	u64	reg;
}GS_R_PRMODECONT;

typedef struct {
	GS_PRMODE		data;
	u64	reg;
}GS_R_PRMODE;

typedef struct {
	GS_TEXCLUT		data;
	u64	reg;
}GS_R_TEXCLUT;

typedef struct {
	GS_SCANMSK		data;
	u64	reg;
}GS_R_SCANMSK;

typedef struct {
	GS_MIPTBP1		data;
	u64	reg;
} GS_R_MIPTBP1;

typedef struct {
	GS_MIPTBP2		data;
	u64	reg;
}GS_R_MIPTBP2;

typedef struct {
	GS_TEXA			data;
	u64	reg;
}GS_R_TEXA;

typedef struct {
	GS_FOGCOLOR		data;
	u64	reg;
}GS_R_FOGCOLOR;

typedef struct {
	GS_TEXFLUSH		data;
	u64	reg;
} GS_R_TEXFLUSH;

typedef struct {
	GS_SCISSOR		data;
	u64	reg;
}GS_R_SCISSOR;

typedef struct {
	GS_ALPHA		data;
	u64	reg;
}GS_R_ALPHA;

typedef struct {
	GS_DIMX			data;
	u64	reg;
} GS_R_DIMX;

typedef struct {
	GS_DTHE			data;
	u64	reg;
} GS_R_DTHE;

typedef struct {
	GS_COLCLAMP		data;
	u64	reg;
}GS_R_COLCLAMP;

typedef struct {
	GS_TEST			data;
	u64	reg;
} GS_R_TEST;

typedef struct {
	GS_PABE			data;
	u64	reg;
}GS_R_PABE;

typedef struct {
	GS_FBA			data;
	u64	reg;
}GS_R_FBA;

typedef struct {
	GS_FRAME		data;
	u64	reg;
} GS_R_FRAME;

typedef struct {
	GS_ZBUF			data;
	u64	reg;
}GS_R_ZBUF;

typedef struct {
	GS_BITBLTBUF	data;
	u64	reg;
}GS_R_BITBLTBUF;

typedef struct {
	GS_TRXPOS		data;
	u64	reg;
}GS_R_TRXPOS;

typedef struct {
	GS_TRXREG		data;
	u64	reg;
}GS_R_TRXREG;

typedef struct {
	GS_TRXDIR		data;
	u64	reg;
}GS_R_TRXDIR;

typedef struct {
	GS_HWREG		data;
	u64	reg;
}GS_R_HWREG;

typedef struct {
	GS_SIGNAL		data;
	u64	reg;
}GS_R_SIGNAL;

typedef struct {
	GS_FINISH		data;
	u64	reg;
}GS_R_FINISH;

typedef struct {
	GS_LABEL		data;
	u64	reg;
}GS_R_LABEL;

typedef struct {
	GS_NOP			data;
	u64	reg;
}GS_R_NOP;

/* Set Funtion For GS Privileged Regs */

#define GS_SET_PMODE(enable_rc1, enable_rc2, mmod, amod, blend_style, blend_value) \
			*(vu64 *)gs_p_pmode =			\
		(u64)((enable_rc1	) & 0x00000001) <<  0 | \
		(u64)((enable_rc2	) & 0x00000001) <<  1 | \
		(u64)((1		) & 0x00000007) <<  2 | \
		(u64)((mmod		) & 0x00000001) <<  5 | \
		(u64)((amod		) & 0x00000001) <<  6 | \
		(u64)((blend_style) & 0x00000001) <<  7 | \
		(u64)((blend_value) & 0x000000FF) <<  8

/** Set by SetGsCrt(). DO NOT SET MANUALLY!! */
#define GS_SET_SMODE1(rc, lc, t1248, slck, cmod, ex, prst, sint, xpck,		\
			pck2, spml, gcont, phs, pvs, pehs, pevs, clksel,	\
			nvck, slck2, vcksel, vhp) \
			*(vu64 *)gs_p_smode1 =		\
		(u64)((rc	) & 0x00000007) <<  0 | \
		(u64)((lc	) & 0x0000007F) <<  3 | \
		(u64)((t1248	) & 0x00000003) << 10 | \
		(u64)((slck	) & 0x00000001) << 12 | \
		(u64)((cmod	) & 0x00000003) << 13 | \
		(u64)((ex	) & 0x00000001) << 15 | \
		(u64)((prst	) & 0x00000001) << 16 | \
		(u64)((sint	) & 0x00000001) << 17 | \
		(u64)((xpck	) & 0x00000001) << 18 | \
		(u64)((pck2	) & 0x00000003) << 19 | \
		(u64)((spml	) & 0x0000000F) << 21 | \
		(u64)((gcont	) & 0x00000001) << 25 | \
		(u64)((phs	) & 0x00000001) << 26 | \
		(u64)((pvs	) & 0x00000001) << 27 | \
		(u64)((pehs	) & 0x00000001) << 28 | \
		(u64)((pevs	) & 0x00000001) << 29 | \
		(u64)((clksel	) & 0x00000003) << 30 | \
		(u64)((nvck	) & 0x00000001) << 32 | \
		(u64)((slck2	) & 0x00000001) << 33 | \
		(u64)((vcksel	) & 0x00000003) << 34 | \
		(u64)((vhp	) & 0x00000003) << 36

#define GS_SET_SMODE2(interlace, field_frame, vesta_dpms) \
			*(vu64 *)gs_p_smode2 =		\
		(u64)((interlace	) & 0x00000001) <<  0 | \
		(u64)((field_frame	) & 0x00000001) <<  1 | \
		(u64)((vesta_dpms	) & 0x00000003) <<  2

#define GS_SET_DISPFB1(address, width, psm, x, y) \
			*(vu64 *)gs_p_dispfb1=\
		(u64)((address	) & 0x000001FF) <<  0 | \
		(u64)((width	) & 0x0000003F) <<  9 | \
		(u64)((psm	) & 0x0000001F) << 15 | \
		(u64)((x	) & 0x000007FF) << 32 | \
		(u64)((y	) & 0x000007FF) << 43

#define GS_SET_DISPFB2(address, width, psm, x, y) \
			*(vu64 *)gs_p_dispfb2=\
		(u64)((address	) & 0x000001FF) <<  0 | \
		(u64)((width		) & 0x0000003F) <<  9 | \
		(u64)((psm	) & 0x0000001F) << 15 | \
		(u64)((x			) & 0x000007FF) << 32 | \
		(u64)((y			) & 0x000007FF) << 43

#define GS_SET_DISPLAY1(display_x, display_y,magnify_h,magnify_v,display_w,display_h) \
			*(vu64 *)gs_p_display1 =		\
		(u64)((display_x) & 0x00000FFF) <<  0 |	\
		(u64)((display_y) & 0x000007FF) << 12 |	\
		(u64)((magnify_h) & 0x0000000F) << 23 |	\
		(u64)((magnify_v) & 0x00000003) << 27 |	\
		(u64)((display_w) & 0x00000FFF) << 32 |	\
		(u64)((display_h) & 0x000007FF) << 44

#define GS_SET_DISPLAY2(display_x, display_y,magnify_h,magnify_v,display_w,display_h) \
			*(vu64 *)gs_p_display2 =		\
		(u64)((display_x) & 0x00000FFF) <<  0 |	\
		(u64)((display_y) & 0x000007FF) << 12 |	\
		(u64)((magnify_h) & 0x0000000F) << 23 |	\
		(u64)((magnify_v) & 0x00000003) << 27 |	\
		(u64)((display_w) & 0x00000FFF) << 32 |	\
		(u64)((display_h) & 0x000007FF) << 44

#define GS_SET_EXTBUF(A,B,C,D,E,F,G,H) \
			*(vu64 *)gs_p_extbuf =	\
		(u64)((A) & 0x00003FFF) <<  0 | \
		(u64)((B) & 0x0000003F) << 14 | \
		(u64)((C) & 0x00000003) << 20 | \
		(u64)((D) & 0x00000001) << 22 | \
		(u64)((E) & 0x00000003) << 23 | \
		(u64)((F) & 0x00000003) << 25 | \
		(u64)((G) & 0x000007FF) << 32 | \
		(u64)((H) & 0x000007FF) << 43

#define GS_SET_EXTDATA(x, y, sample_r_h, sample_r_v, write_w, write_h) \
			*(vu64 *)gs_p_extdata =		\
		(u64)((x		) & 0x00000FFF) <<  0 | \
		(u64)((y		) & 0x000007FF) << 12 | \
		(u64)((sample_r_h	) & 0x0000000F) << 23 | \
		(u64)((sample_r_v	) & 0x00000003) << 27 | \
		(u64)((write_w		) & 0x00000FFF) << 32 | \
		(u64)((write_h		) & 0x000007FF) << 44

#define GS_SET_EXTWRITE(write)\
			*(vu64 *)gs_p_extwrite = \
		(u64)((write) & 0x00000001)

#define GS_SET_BGCOLOR(r,g,b) \
			*(vu64 *)gs_p_bgcolor =	\
		(u64)((r) & 0x000000FF) <<  0 | \
		(u64)((g) & 0x000000FF) <<  8 | \
		(u64)((b) & 0x000000FF) << 16

#define GS_SET_CSR(signal_evnt,finish_evnt,hsync_intrupt,vsync_intrupt,write_terminate,flush,reset,nfield,current_field,fifo_status,gs_rev_number,gs_id) \
			*(vu64 *)gs_p_csr =				\
		(u64)((signal_evnt	) & 0x00000001) <<  0 | \
		(u64)((finish_evnt	) & 0x00000001) <<  1 | \
		(u64)((hsync_intrupt	) & 0x00000001) <<  2 | \
		(u64)((vsync_intrupt	) & 0x00000001) <<  3 | \
		(u64)((write_terminate) & 0x00000001) <<  4 | \
		(u64)((flush		) & 0x00000001) <<  8 | \
		(u64)((reset		) & 0x00000001) <<  9 | \
		(u64)((nfield		) & 0x00000001) << 12 | \
		(u64)((current_field	) & 0x00000001) << 13 | \
		(u64)((fifo_status	) & 0x00000003) << 14 | \
		(u64)((gs_rev_number	) & 0x000000FF) << 16 | \
		(u64)((gs_id		) & 0x000000FF) << 24

#define GS_SET_IMR(signal_mask, finish_mask, hsync_mask, vsync_mask, write_mask, exhs_mask, exvs_mask) \
			*(vu64 *)gs_p_imr =			\
		(u64)((signal_mask) & 0x00000001) <<  8 | \
		(u64)((finish_mask) & 0x00000001) <<  9 | \
		(u64)((hsync_mask	) & 0x00000001) << 10 | \
		(u64)((vsync_mask	) & 0x00000001) << 11 | \
		(u64)((write_mask	) & 0x00000001) << 12 | \
		(u64)((exhs_mask	) & 0x00000001) << 13 | \
		(u64)((exvs_mask	) & 0x00000001) << 14

#define GS_SET_BUSDIR(direction) \
			*(vu64 *)gs_p_busdir = \
		(u64)((direction) & 0x00000001)

#define GS_SET_SIGLBLID(signal_id, label_id) \
			*(vu64 *)gs_p_siglblid =	\
		(u64)((signal_id	) & 0xFFFFFFFF) <<  0 | \
		(u64)((label_id		) & 0xFFFFFFFF) << 32

/*
 * These are use to SET the individual values
 * in each of the readable Privileged registers.
 */

#define GS_SET_CSR_signal_evnt(val) \
			GS_SET_CSR(val,0,0,0,0,0,0,0,0,0,0,0)

#define GS_SET_CSR_finish_evnt(val) \
			GS_SET_CSR(0,val,0,0,0,0,0,0,0,0,0,0)

#define GS_SET_CSR_hsync_intrupt(val) \
			GS_SET_CSR(0,0,val,0,0,0,0,0,0,0,0,0)

#define GS_SET_CSR_vsync_intrupt(val) \
			GS_SET_CSR(0,0,0,val,0,0,0,0,0,0,0,0)

#define GS_SET_CSR_write_terminate(val) \
			GS_SET_CSR(0,0,0,0,val,0,0,0,0,0,0,0)

#define GS_SET_CSR_flush(val) \
			GS_SET_CSR(0,0,0,0,0,val,0,0,0,0,0,0)

#define GS_SET_CSR_reset(val) \
			GS_SET_CSR(0,0,0,0,0,0,val,0,0,0,0,0)

/*nfield		(r)*/
/*current_field (r)*/
/*fifo_status	(r)*/
/*gs_rev_number (r)*/
/*gs_id			(r)*/

/*
 * These are use to GET the individual values
 * in each of the readable Privileged registers.
 */

#define GS_GET_CSR_signal_evnt \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 0))

#define GS_GET_CSR_finish_evnt \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 1))

#define GS_GET_CSR_hsync_intrupt \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 2))

#define GS_GET_CSR_vsync_intrupt \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 3))

#define GS_GET_CSR_write_terminate \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 4))
/*flush (w)*/

/*reset (w)*/

#define GS_GET_CSR_write_nfield \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 12))

#define GS_GET_CSR_current_field \
			(*((vu64 *)(gs_p_csr)) & (0x00000001 << 13))

#define GS_GET_CSR_fifo_status \
			(*((vu64 *)(gs_p_csr)) & (0x00000003 << 14))

#define GS_GET_CSR_gs_rev_number \
			(*((vu64 *)(gs_p_csr)) & (0x000000FF << 16))

#define GS_GET_CSR_gs_id \
			(*((vu64 *)(gs_p_csr)) & (0x000000FF << 24))

/* 'SET' GENERAL PURPOSE REG STRUCTS 'WITHOUT' REG */

#define gs_setPRIM(p, _prim_type,_iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)\
	(p)->prim_type = _prim_type,	\
	(p)->iip = _iip,				\
	(p)->tme = _tme,				\
	(p)->fge = _fge,				\
	(p)->abe = _abe,				\
	(p)->aa1 = _aa1,				\
	(p)->fst = _fst,				\
	(p)->ctxt = _ctxt,				\
	(p)->fix	= _fix

#define gs_setRGBAQ(p, _r,_g,_b,_a,_q)	\
	(p)->r = _r,						\
	(p)->g = _g,						\
	(p)->b = _b,						\
	(p)->a = _a,						\
	(p)->q = _q

#define gs_setST(p, _s,_t)	\
	(p)->s = _s,			\
	(p)->t = _t

#define gs_setUV(p, _u,_v)	\
	(p)->u = _u,			\
	(p)->v = _v

#define gs_setXYZF2(p, _x,_y,_z,_f)	\
	(p)->x = _x,						\
	(p)->y = _y,						\
	(p)->z = _z,						\
	(p)->f = _f

#define gs_setXYZF3(p, _x,_y,_z,_f)	\
	(p)->x = _x,						\
	(p)->y = _y,						\
	(p)->z = _z,						\
	(p)->f = _f

#define gs_setXYZ2(p, _x,_y,_z)	\
	(p)->x = _x,					\
	(p)->y = _y,					\
	(p)->z = _z

#define gs_setXYZ3(p, _x,_y,_z)	\
	(p)->x = _x,					\
	(p)->y = _y,					\
	(p)->z = _z

#define gs_setTEX0_1(p, _tb_addr, _tb_width, _psm, _tex_width, _tex_height, _tex_cc, _tex_funtion, _cb_addr, _clut_pixmode, _clut_smode, _clut_offset, _clut_loadmode)\
	(p)->tb_addr		= _tb_addr,			\
	(p)->tb_width		= _tb_width,		\
	(p)->psm		= _psm,		\
	(p)->tex_width		= _tex_width,		\
	(p)->tex_height		= _tex_height,		\
	(p)->tex_cc			= _tex_cc,			\
	(p)->tex_funtion	= _tex_funtion,		\
	(p)->cb_addr		= _cb_addr,			\
	(p)->clut_pixmode	= _clut_pixmode,	\
	(p)->clut_smode		= _clut_smode,		\
	(p)->clut_offset	= _clut_offset,		\
	(p)->clut_loadmode	= _clut_loadmode

#define gs_setTEX0_2			gs_setTEX0_1

#define gs_setCLAMP_1(p, wms,wmt,minu,maxu,minv,maxv)	\
	(p)->wms  = wms,		\
	(p)->wmt  = wmt,		\
	(p)->minu = minu,		\
	(p)->maxu = maxu,		\
	(p)->minv = minv,		\
	(p)->maxv = maxv

#define gs_setCLAMP_2			gs_setCLAMP_1

#define gs_setFOG(p, _f)	\
	(p)->f  = _f

#define gs_setTEX1_1(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k)	\
	(p)->lcm  = _lcm,		\
	(p)->mxl  = _mxl,		\
	(p)->mmag = _mmag,		\
	(p)->mmin = _mmin,		\
	(p)->mtba = _mtba,		\
	(p)->l    = _l,			\
	(p)->k    = _k

#define gs_setTEX1_2		gs_setTEX1_1

#define gs_setTEX2_1(p, _psm,_cbp,_cpsm,_csm,_csa,_cld)	\
	(p)->psm  = _psm,		\
	(p)->cbp  = _cbp,		\
	(p)->cpsm = _cpsm,		\
	(p)->csm  = _csm,		\
	(p)->csa  = _csa,		\
	(p)->cld  = _cld

#define gs_setTEX2_2		gs_setTEX2_1

#define gs_set_XYOFFSET_1(p, _offset_x,_offset_y)	\
	(p)->offset_x  = _offset_x,						\
	(p)->offset_y  = _offset_y

#define gs_set_XYOFFSET_2		gs_set_XYOFFSET_1

#define gs_set_PRMODECONT(p, _control)	\
	(p)->control   = _control

#define gs_setPRMODE(p, _iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)	\
	(p)->iip	= _iip,		\
	(p)->tme	= _tme,		\
	(p)->fge	= _fge,		\
	(p)->abe	= _abe,		\
	(p)->aa1	= _aa1,		\
	(p)->fst	= _fst,		\
	(p)->ctxt	= _ctxt,	\
	(p)->fix	= _fix

#define gs_setTEXCLUT(p, _cbw,_cou,_cov)	\
	(p)->cbw	= _cbw,						\
	(p)->cou	= _cou,						\
	(p)->cov	= _cov

#define gs_setSCANMSK(p, _msk)	\
	(p)->msk	= _msk

#define gs_setMIPTBP1_1(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
	(p)->tbp1	= _tbp1,		\
	(p)->tbw1	= _tbw1,		\
	(p)->tbp2	= _tbp2,		\
	(p)->tbw2	= _tbw2,		\
	(p)->tbp3	= _tbp3,		\
	(p)->tbw3	= _tbw3

#define gs_setMIPTBP1_2			gs_setMIPTBP1_1

#define gs_setMIPTBP2_1(p, _tbp4, _tbw4, _tbp5, _tbw5, _tbp6, _tbw6)	\
	(p)->tbp4	= _tbp4,		\
	(p)->tbw4	= _tbw4,		\
	(p)->tbp5	= _tbp5,		\
	(p)->tbw5	= _tbw5,		\
	(p)->tbp6	= _tbp6,		\
	(p)->tbw6	= _tbw6

#define gs_setMIPTBP2_2			gs_setMIPTBP2_1

#define gs_setTEXA(p, _alpha_0, _alpha_method, _alpha_1)	\
	(p)->alpha_0		= _alpha_0,						\
	(p)->alpha_method	= _alpha_method,						\
	(p)->alpha_1		= _alpha_1

#define gs_setFOGCOLOR(p, _r,_g,_b)		\
	(p)->r	= _r,							\
	(p)->g	= _g,							\
	(p)->b	= _b

#define gs_setTEXFLUSH(p)

#define gs_setSCISSOR_1(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1)	\
	(p)->clip_x0	= _clip_x0,										\
	(p)->clip_x1	= _clip_x1,										\
	(p)->clip_y0	= _clip_y0,										\
	(p)->clip_y1	= _clip_y1

#define gs_setSCISSOR_2			gs_setSCISSOR_1

#define gs_setALPHA_1(p, _a,_b,_c,_d,_alpha)\
	(p)->a		= _a,						\
	(p)->b		= _b,						\
	(p)->c		= _c,						\
	(p)->d		= _d,						\
	(p)->alpha	= _alpha

#define gs_setALPHA_2				gs_setALPHA_1

#define gs_setDIMX(p, _dimx00,_dimx01,_dimx02,_dimx03,_dimx10,_dimx11,_dimx12,_dimx13,_dimx20,_dimx21,_dimx22,_dimx23,_dimx30,_dimx31,_dimx32,_dimx33)	\
	(p)->dimx00		= _dimx00,						\
	(p)->dimx01		= _dimx01,						\
	(p)->dimx02		= _dimx02,						\
	(p)->dimx03		= _dimx03,						\
	(p)->dimx10		= _dimx10,						\
	(p)->dimx11		= _dimx11,						\
	(p)->dimx12		= _dimx12,						\
	(p)->dimx13		= _dimx13,						\
	(p)->dimx20		= _dimx20,						\
	(p)->dimx21		= _dimx21,						\
	(p)->dimx22		= _dimx22,						\
	(p)->dimx23		= _dimx23,						\
	(p)->dimx30		= _dimx30,						\
	(p)->dimx31		= _dimx31,						\
	(p)->dimx32		= _dimx32,						\
	(p)->dimx33		= _dimx33

#define gs_setDTHE(p, _enable)	\
	(p)->enable	= _enable

#define gs_setCOLCLAMP(p, _clamp)	\
	(p)->clamp	= _clamp

#define gs_setTEST_1(p, _atest_enable, _atest_method, _atest_reference, _atest_fail_method, _datest_enable, _datest_mode, _ztest_enable, _ztest_method)	\
	(p)->atest_enable		= _atest_enable,			\
	(p)->atest_method		= _atest_method,			\
	(p)->atest_reference	= _atest_reference,			\
	(p)->atest_fail_method	= _atest_fail_method,		\
	(p)->datest_enable		= _datest_enable,			\
	(p)->datest_mode		= _datest_mode,				\
	(p)->ztest_enable		= _ztest_enable,			\
	(p)->ztest_method		= _ztest_method

#define gs_setTEST_2			gs_setTEST_1

#define gs_setPABE(p, _enable)	\
	(p)->enable	= _enable

#define gs_setFBA(p, _alpha)	\
	(p)->alpha	= _alpha

#define gs_setFRAME_1(p, _fb_addr,_fb_width,_psm,_draw_mask)	\
	(p)->fb_addr		= _fb_addr,				\
	(p)->fb_width		= _fb_width,			\
	(p)->psm		= _psm,			\
	(p)->draw_mask		= _draw_mask

#define gs_setFRAME_2				gs_setFRAME_1

#define gs_setZBUF_1(p, _fb_addr,_psm,_update_mask)	\
	(p)->fb_addr		= _fb_addr,						\
	(p)->psm		= _psm,					\
	(p)->update_mask	= _update_mask

#define gs_setZBUF_2			gs_setZBUF_1

#define gs_setBITBLTBUF(p, _src_addr,_src_width,_src_pixmode,_dest_addr,_dest_width,_dest_pixmode)	\
	(p)->src_addr		= _src_addr,				\
	(p)->src_width		= _src_width,				\
	(p)->src_pixmode	= _src_pixmode,				\
	(p)->dest_addr		= _dest_addr,				\
	(p)->dest_width		= _dest_width,				\
	(p)->dest_pixmode	= _dest_pixmode

#define gs_setTRXPOS(p, _src_x,_src_y,_dest_x,_dest_y,_direction)	\
	(p)->src_x		= _src_x,				\
	(p)->src_y		= _src_y,				\
	(p)->dest_x		= _dest_x,				\
	(p)->dest_y		= _dest_y,				\
	(p)->direction	= _direction

#define gs_setTRXREG(p, _trans_w,_trans_h)	\
	(p)->trans_w	= _trans_w,				\
	(p)->trans_h	= _trans_h

#define gs_setTRXDIR(p, _trans_dir)	\
	(p)->trans_dir	= _trans_dir

#define gs_setHWREG(p, _data)	\
	(p)->data	= _data

#define gs_setSIGNAL(p, _signal_id,_update_mask)	\
	(p)->signal_id	= _signal_id,					\
	(p)->update_mask= _update_mask

#define gs_setFINISH(p) \
	(p)->pad0 = 0

#define gs_setLABEL(p, _label_id,_update_mask)	\
	(p)->label_id	= _label_id,				\
	(p)->update_mask= _update_mask

#define gs_setNOP(p) \
	(p)->pad0 = 0

/* 'SET' GENERAL PURPOSE REG STRUCTs & REGs */

#define gs_setR_PRIM(p, _prim_type,_iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)\
						gs_setPRIM(&p->data, _prim_type,_iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix),\
						p->reg = gs_g_prim

#define gs_setR_RGBAQ(p, _r,_g,_b,_a,_q)	\
						gs_setRGBAQ(&p->data, _r,_g,_b,_a,_q),\
						(p)->reg = gs_g_rgbaq

#define gs_setR_ST(p, _s,_t)	\
						gs_setST(&p->data, _s,_t),\
						(p)->reg = gs_g_st

#define gs_setR_UV(p, _u,_v)	\
						gs_setUV(&p->data, _u,_v),\
						(p)->reg = gs_g_uv

#define gs_setR_XYZF2(p, _x,_y,_z,_f)	\
						gs_setXYZF2(&p->data, _x,_y,_z,_f),\
						(p)->reg = gs_g_xyzf2

#define gs_setR_XYZF3(p, _x,_y,_z,_f)	\
						gs_setR_XYZF3(&p->data, _x,_y,_z,_f),\
						(p)->reg = gs_g_xyzf3

#define gs_setR_XYZ2(p, _x,_y,_z)	\
						gs_setXYZ2(&p->data, _x,_y,_z),\
						(p)->reg = 	gs_g_xyz2

#define gs_setR_XYZ3(p, _x,_y,_z)	\
						gs_setXYZ3(&p->data, _x,_y,_z),\
						(p)->reg = 	gs_g_xyz3

#define gs_setR_TEX0_1(p, _fb_addr,_fb_width,_psm,_tex_width,_tex_height,_col_comp,_tex_cc,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld)\
							gs_setTEX0_1(&p->data, _fb_addr,_fb_width,_psm,_tex_width,_tex_height,_col_comp,_tex_cc,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld),\
							(p)->reg = 	gs_g_tex0_1

#define gs_setR_TEX0_2(p, _fb_addr,_fb_width,_psm,_tex_width,_tex_height,_col_comp,_tex_cc,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld)\
							gs_setTEX0_2(&p->data, _fb_addr,_fb_width,_psm,_tex_width,_tex_height,_col_comp,_tex_cc,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld),\
							(p)->reg = 	gs_g_tex0_2

#define gs_setR_CLAMP_1(p, wms,wmt,minu,maxu,minv,maxv)	\
							gs_setCLAMP_1(&p->data, wms,wmt,minu,maxu,minv,maxv),\
							(p)->reg = 	gs_g_clamp_1

#define gs_setR_CLAMP_2(p, wms,wmt,minu,maxu,minv,maxv)	\
							gs_setCLAMP_2(&p->data, wms,wmt,minu,maxu,minv,maxv),\
							(p)->reg = 	gs_g_clamp_2

#define gs_setR_FOG(p, _f)	\
							gs_setFOG(&p->data, _f),\
							(p)->reg = 	gs_g_fog

#define gs_setR_TEX1_1(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k)	\
							gs_setTEX1_1(&p->data, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k),\
							(p)->reg = 	gs_g_tex1_1

#define gs_setR_TEX1_2(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k)	\
							gs_setTEX1_2(&p->data, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k),\
							(p)->reg = 	gs_g_tex1_2

#define gs_setR_TEX2_1(p, _psm,_cbp,_cpsm,_csm,_csa,_cld)	\
							gs_setTEX2_1(&p->data, _psm,_cbp,_cpsm,_csm,_csa,_cld),\
							(p)->reg = 	gs_g_tex2_1

#define gs_setR_TEX2_2(p, _psm,_cbp,_cpsm,_csm,_csa,_cld)	\
							gs_setTEX2_2(&p->data, _psm,_cbp,_cpsm,_csm,_csa,_cld),\
							(p)->reg = 	gs_g_tex2_2

#define gs_setR_XYOFFSET_1(p, _offset_x,_offset_y)	\
							gs_set_XYOFFSET_1(&p->data, _offset_x,_offset_y),\
							(p)->reg = 	gs_g_xyoffset_1

#define gs_setR_XYOFFSET_2(p, _offset_x,_offset_y)	\
							gs_set_XYOFFSET_2(&p->data, _offset_x,_offset_y),\
							(p)->reg = 	gs_g_xyoffset_2

#define gs_setR_PRMODECONT(p, _control)	\
							gs_set_PRMODECONT(&p->data, _control),\
							(p)->reg = 	gs_g_prmodecont

#define gs_setR_PRMODE(p, _iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)	\
							gs_setPRMODE(&p->data, _iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix),\
							(p)->reg = 	gs_g_prmode

#define gs_setR_TEXCLUT(p, _cbw,_cou,_cov)	\
							gs_setTEXCLUT(&p->data, _cbw,_cou,_cov),\
							(p)->reg = 	gs_g_texclut

#define gs_setR_SCANMSK(p, _msk)	\
							gs_setSCANMSK(&p->data, _msk),\
							(p)->reg = 	gs_g_scanmsk

#define gs_setR_MIPTBP1_1(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
							gs_setMIPTBP1_1(&p->data, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3),\
							(p)->reg = 	gs_g_miptbp1_1

#define gs_setR_MIPTBP1_2(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
							gs_setMIPTBP1_2(&p->data, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3),\
							(p)->reg = 	gs_g_miptbp1_2

#define gs_setR_MIPTBP2_1(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6)	\
							gs_setR_MIPTBP2_1(&p->data, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6),\
							(p)->reg = 	gs_g_miptbp2_1

#define gs_setR_MIPTBP2_2(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6)	\
							gs_setMIPTBP2_2(&p->data, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6),\
							(p)->reg = 	gs_g_miptbp2_2

#define gs_setR_TEXA(p, _ta0,_aem,_ta1)	\
							gs_setTEXA(&p->data, _ta0,_aem,_ta1),\
							(p)->reg = 	gs_g_texa

#define gs_setR_FOGCOLOR(p, _r,_g,_b)		\
							gs_setFOGCOLOR(&p->data, _r,_g,_b),\
							(p)->reg = 	gs_g_fogcol

#define gs_setR_TEXFLUSH(p)\
							(p)->reg = 	gs_g_texflush

#define gs_setR_SCISSOR_1(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1)	\
							gs_setSCISSOR_1(&p->data, _clip_x0,_clip_x1,_clip_y0,_clip_y1),\
							(p)->reg = 	gs_g_scissor_1

#define gs_setR_SCISSOR_2(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1)	\
							gs_setSCISSOR_2(&p->data, _clip_x0,_clip_x1,_clip_y0,_clip_y1),\
							(p)->reg = 	gs_g_scissor_2

#define gs_setR_ALPHA_1(p, _a,_b,_c,_d,_fix)	\
							gs_setALPHA_1(&p->data, _a,_b,_c,_d,_fix),\
							(p)->reg = 	gs_g_alpha_1

#define gs_setR_ALPHA_2(p, _a,_b,_c,_d,_fix)	\
							gs_setALPHA_2(&p->data, _a,_b,_c,_d,_fix),\
							(p)->reg = 	gs_g_alpha_2

#define gs_setR_DIMX(p, _dimx00,_dimx01,_dimx02,_dimx03,_dimx10,_dimx11,_dimx12,_dimx13,_dimx20,_dimx21,_dimx22,_dimx23,_dimx30,_dimx31,_dimx32,_dimx33)	\
							gs_setDIMX(&p->data, _dimx00,_dimx01,_dimx02,_dimx03,_dimx10,_dimx11,_dimx12,_dimx13,_dimx20,_dimx21,_dimx22,_dimx23,_dimx30,_dimx31,_dimx32,_dimx33),\
							(p)->reg = 	gs_g_dimx

#define gs_setR_DTHE(p, _enable)	\
							gs_setDTHE(&p->data, _enable),\
							(p)->reg = 	gs_g_dthe

#define gs_setR_COLCLAMP(p, _clamp)	\
							gs_setCOLCLAMP(&p->data, _clamp),\
							(p)->reg = gs_g_colclamp

#define gs_setR_TEST_1(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST)	\
							gs_setTEST_1(&p->data, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST),\
							(p)->reg = gs_g_test_1

#define gs_setR_TEST_2(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST)	\
							gs_setTEST_2(&p->data, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST),\
							(p)->reg = gs_g_test_2

#define gs_setR_PABE(p, _enable)	\
							gs_setPABE(&p->data, _enable),\
							(p)->reg = gs_g_pabe

#define gs_setR_FBA_1(p, _alpha)	\
							gs_setFBA(&p->data, _alpha),\
							(p)->reg = gs_g_fba_1

#define gs_setR_FBA_2(p, _alpha)	\
							gs_setFBA(&p->data, _alpha),\
							(p)->reg = gs_g_fba_2

#define gs_setR_FRAME_1(p, _fb_addr,_fb_width,_psm,_draw_mask)	\
							gs_setFRAME_1(&p->data, _fb_addr,_fb_width,_psm,_draw_mask),\
							(p)->reg = gs_g_frame_1

#define gs_setR_FRAME_2(p, _fb_addr,_fb_width,_psm,_draw_mask)	\
							gs_setFRAME_2(&p->data, _fb_addr,_fb_width,_psm,_draw_mask),\
							(p)->reg = gs_g_frame_2

#define gs_setR_ZBUF_1(p, _fb_addr,_psm,_update_mask)	\
							gs_setZBUF_1(&p->data, _fb_addr,_psm,_update_mask),\
							(p)->reg = gs_g_zbuf_1

#define gs_setR_ZBUF_2(p, _fb_addr,_psm,_update_mask)	\
							gs_setZBUF_2(&p->data, _fb_addr,_psm,_update_mask),\
							(p)->reg = gs_g_zbuf_2

#define gs_setR_BITBLTBUF(p, _src_addr,_src_width,_src_pixmode,_dest_addr,_dest_width,_dest_pixmode)	\
							gs_setBITBLTBUF(&p->data, _src_addr,_src_width,_src_pixmode,_dest_addr,_dest_width,_dest_pixmode),\
							(p)->reg = gs_g_bitbltbuf

#define gs_setR_TRXPOS(p, _src_x,_src_y,_dest_x,_dest_y,_direction)	\
							gs_setTRXPOS(&p->data, _src_x,_src_y,_dest_x,_dest_y,_direction),\
							(p)->reg = gs_g_trxpos

#define gs_setR_TRXREG(p, _trans_w,_trans_h)	\
							gs_setTRXREG(&p->data, _trans_w,_trans_h),\
							(p)->reg = gs_g_trxreg

#define gs_setR_TRXDIR(p, _trans_dir)	\
							gs_setTRXDIR(&p->data, _trans_dir),\
							(p)->reg = gs_g_trxdir

#define gs_setR_HWREG(p, _data)	\
							gs_setHWREG(&p->data, _data),\
							(p)->reg = gs_g_hwreg

#define gs_setR_SIGNAL(p, _signal_id,_update_mask)	\
							gs_setSIGNAL(&p->data, _signal_id,_update_mask),\
							(p)->reg = gs_g_signal

#define gs_setR_FINISH(p)\
							(p)->reg = gs_g_finish

#define gs_setR_LABEL(p, _label_id,_update_mask)	\
							gs_setLABEL(&p->data, _label_id,_update_mask),\
							(p)->reg = gs_g_label

#define gs_setR_NOP(p)	\
							gs_setNOP(&p->data),\
							(p)->reg = gs_g_label

/* MISC */

/** SOURCE CHAIN TAG for DMA CHAIN MODE */
typedef struct _GS_GIF_DMACHAIN_TAG{
	u64	qwc	:16;
	u64	pad1	:10;
	u64	pce	:2;
	u64	id	:3;
	u64	irq	:1;
	u64	addr	:31;
	u64	spr	:1;
	u64	pad2	:64;
}GS_GIF_DMACHAIN_TAG		__attribute__ ((aligned(16)));/*aligned 128bits*/

typedef struct {
	u64 nloop	:15;
	u64 eop	:1;
	u64 pad1	:30;
	u64 pre	:1;
	u64 prim	:11;
	u64 flg	:2;
	u64 nreg	:4;
	u64 reg	:64;
}GS_GIF_TAG;

#define gs_setGIF_TAG(p, _nloop,_eop,_pre,_prim,_flg,_nreg,_reg)\
	(p)->nloop	= _nloop,				\
	(p)->eop	= _eop,					\
	(p)->pre	= _pre,					\
	(p)->prim	= _prim,				\
	(p)->flg	= _flg,					\
	(p)->nreg	= _nreg,				\
	(p)->reg	= _reg

/* MID LEVEL DEFINES */

/* settings for GsSetCRTCSettings() */

/*A Default setting*/
#define CRTC_SETTINGS_DEFAULT1		CRTC_SETTINGS_EN1|CRTC_SETTINGS_BLENDVAL|CRTC_SETTINGS_OUTRC1|CRTC_SETTINGS_STYLERC1
#define CRTC_SETTINGS_DEFAULT2		CRTC_SETTINGS_EN2|CRTC_SETTINGS_BLENDVAL|CRTC_SETTINGS_OUTRC1|CRTC_SETTINGS_STYLERC1

/** Enable RC1(ReadCircuit 1) */
#define CRTC_SETTINGS_EN1			((u64)(1)<<0)
/** Enable RC2(ReadCircuit 1) */
#define CRTC_SETTINGS_EN2			((u64)(1)<<1)
/** Enable RC1 & R2 */
#define CRTC_SETTINGS_ENBOTH		CRTC_SETTINGS_EN1|CRTC_SETTINGS_EN2
/** Use Alpha value from rc1 for blending */
#define CRTC_SETTINGS_BLENDRC1		((u64)(0)<<5)
/** Use Alpha value from alpha_value of GsSetCRTCSettings() for blending */
#define CRTC_SETTINGS_BLENDVAL		((u64)(1)<<5)
/** Output Final image to RC1 */
#define CRTC_SETTINGS_OUTRC1		((u64)(0)<<6)
/** Output Final image to RC2 */
#define CRTC_SETTINGS_OUTRC2		((u64)(1)<<6)
/** Blend With The Out Put of RC1 */
#define CRTC_SETTINGS_STYLERC1		((u64)(0)<<7)
/** Blend With The Out Put of BG(background) */
#define CRTC_SETTINGS_STYLEBG		((u64)(1)<<7)

typedef struct {
	short	x;
	short	y;
	short	w;
	short	h;
}GS_RECT;

typedef struct {
	u16	x;
	u16	y;
	u16	w;
	u16	h;
}GS_URECT;

typedef struct {
	int		x;
	int		y;
	int		w;
	int		h;
}GS_RECT32;

typedef struct {
	short	x;
	short	y;
}GS_POS;

typedef struct
{
	u16	vram_addr;
	u8	psm;
	u8	update_mask;
}GS_ZENV;

/** Screen Draw Environment */
typedef struct {
	/* Draw offset X */
	u16	offset_x;
	/* Draw offset Y */
	u16	offset_y;
	/* Draw Clip rect */
	GS_URECT	clip;
	/* Vram Address in frame buffer */
	u16	vram_addr;
	/* Width of vram (1=64) */
	u8	fbw;
	/* Pixel Mode / PSM */
	u8	psm;
	/* X offset in vram; */
	u16	vram_x;
	/* Y offset in vram; */
	u16	vram_y;
	/* Draw Mask (0=draw, 1=no draw) */
	u32	draw_mask;
	/* Set To 1 If You Want The Draw Environment's Backgroud to Clear When GsPutDrawEnv() is called */
	u8	auto_clear;
	/* Color to use to clear backgroud */
	GS_RGBAQ		bg_color;

}GS_DRAWENV;

/** Screen Display Environment */
typedef struct {
	GS_DISPLAY	disp;
	GS_DISPFB	dispfb;
}GS_DISPENV __attribute__ ((aligned(8)));/* Aligned, 64bits*/

/** a pixel */
typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** point Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex coordinate */
	GS_R_XYZ		xyz0;
}GS_POINT_P;					/*Size= 3 QWords*/

typedef struct
{
	/** point color */
	GS_R_RGBAQ		rgbaq0;
	/** Primative vertex coordinate */
	GS_R_XYZ		xyz0;
}GS_POINT;						/*Size= 2 QWords*/

/** A Flat Line */
typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** line color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex coordinate */
	GS_R_XYZ		xyz0;
	/** vertex coordinate */
	GS_R_XYZ		xyz1;
}GS_LINE_F1_P;					/*Size = 4 QWords*/

typedef struct
{
	/** line color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex coordinate */
	GS_R_XYZ		xyz0;
	/** vertex coordinate */
	GS_R_XYZ		xyz1;
}GS_LINE_F1;					/*Size = 3 QWords*/

/** A Gouraud Line */
typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** vertex 0 color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
}GS_LINE_G1_P;					/*Size= 5 QWords*/

typedef struct
{
	/** vertex 0 color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
}GS_LINE_G1;					/*Size= 4 QWords*/

/** A Flat Triangle / Triangle Strip / Triangle Fan */
typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** tringle color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0; 
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_F3_P;				/*Size= 5 QWords*/

typedef struct
{
	/** triangle color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_F3;				/*Size= 4 QWords*/

typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim; 
	/** triangle color */
	GS_R_RGBAQ		rgbaq0; 
	/** vertex 0 texture coordinate */
	GS_R_UV			uv0; 
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0; 
	/** vertex 1 texture coordinate */
	GS_R_UV			uv1; 
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1; 
	/** vertex 2 texture coordinate */
	GS_R_UV			uv2; 
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2; 
}GS_TRIANGLE_FT3_P;				/*Size= 8 QWords*/

typedef struct
{
	/** triangle color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_UV			uv0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 texture coordinate */
	GS_R_UV			uv1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 texture coordinate */
	GS_R_UV			uv2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_FT3;				/*Size= 7 QWords*/

typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** triangle color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_ST			st0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 texture coordinate */
	GS_R_ST			st1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 texture coordinate */
	GS_R_ST			st2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_FT3ST_P;			/*Size= 8 QWords*/

typedef struct
{
	/** triangle color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_ST			st0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 texture coordinate */
	GS_R_ST			st1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 texture coordinate */
	GS_R_ST			st2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_FT3ST;				/*Size= 7 QWords*/

/** A Gouraud Triangle / Tri-Strip / Tri-Fan */
typedef struct
{
	/** primitive attributes */
	GS_R_PRIM		prim;
	/** vertex 0 color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 color */
	GS_R_RGBAQ		rgbaq2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_G3_P;				/*Size= 7 QWords*/

typedef struct
{
	/** vertex 0 color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** vertex 1 color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** vertex 2 color */
	GS_R_RGBAQ		rgbaq2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_G3;				/*Size= 6 QWords*/

typedef struct
{
	/** Primitive Attributes */
	GS_R_PRIM		prim;
	/** Vertex 0 Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_ST			st0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** Vertex 1 Color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 texture coordinate */
	GS_R_ST			st1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** Vertex 2 Color */
	GS_R_RGBAQ		rgbaq2;
	/** vertex 2 texture coordinate */
	GS_R_ST			st2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_GT3ST_P;			/*Size= 10 QWords*/

typedef struct
{
	/** Vertex 0 Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_ST			st0;
	/** vertex 0 coordinate */
	GS_R_XYZ		xyz0;
	/** Vertex 1 Color */
	GS_R_RGBAQ		rgbaq1;
	/** vertex 1 texture coordinate */
	GS_R_ST			st1;
	/** vertex 1 coordinate */
	GS_R_XYZ		xyz1;
	/** Vertex 2 Color */
	GS_R_RGBAQ		rgbaq2;
	/** vertex 2 texture coordinate */
	GS_R_ST			st2;
	/** vertex 2 coordinate */
	GS_R_XYZ		xyz2;
}GS_TRIANGLE_GT3ST;			/*Size= 9 QWords*/

/** A Untextured Sprite */
typedef struct
{
	/** Primitive Attributes */
	GS_R_PRIM		prim;
	/** sprite Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 1 coordinate(upper left) */
	GS_R_XYZ		xyz0;
	/** vertex 2 coordinate(lower right) */
	GS_R_XYZ		xyz1;
}GS_SPRITE_F4_P;			/*Size= 4 QWords*/

typedef struct
{
	/** Vertex 1 Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 1 coordinate(upper left) */
	GS_R_XYZ		xyz0;
	/** vertex 2 coordinate(lower right) */
	GS_R_XYZ		xyz1;
}GS_SPRITE_F4;				/*Size= 3 QWords*/

/*A Textured Sprite*/
typedef struct
{
	/** Primitive Attributes */
	GS_R_PRIM		prim;
	/** sprite Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 Texture Coordinate(upper left) */
	GS_R_UV			uv0;
	/** vertex 0 coordinate(upper left) */
	GS_R_XYZ		xyz0;
	/** vertex 1 Texture Coordinate(lower right) */
	GS_R_UV			uv1;
	/** vertex 1 coordinate(lower right) */
	GS_R_XYZ		xyz1;
}GS_SPRITE_FT4_P;			/*Size= 6 QWords*/

typedef struct
{
	/** sprite Color */
	GS_R_RGBAQ		rgbaq0;
	/** vertex 0 texture coordinate */
	GS_R_UV			uv0;
	/** vertex 0 coordinate(upper left) */
	GS_R_XYZ		xyz0;
	/** vertex 1 texture coordinate */
	GS_R_UV			uv1;
	/** vertex 1 coordinate(lower right) */
	GS_R_XYZ		xyz1;
}GS_SPRITE_FT4;				/*Size= 5 QWords*/

#define	GS_PACKET_DATA_QWORD_MAX	32000

typedef struct
{
	GS_GIF_DMACHAIN_TAG	tag;
	QWORD			data[GS_PACKET_DATA_QWORD_MAX];
}GS_GIF_PACKET	__attribute__ ((aligned(16)));/* Aligned, 128bits*/

typedef struct
{
	u32	packet_count;
	u32	packet_offset;
	u32	qword_offset;
	GS_GIF_PACKET	*packets;
}GS_PACKET_TABLE;

typedef struct
{
	/** X Offset in Vram Address */
	u16	x; 
	/** X Offset in Vram Address */
	u16	y; 
	/** Height of image */
	u16	width;
	/** Width  of image */
	u16	height;
	/** Address in frame buffer */
	u16	vram_addr;
	/** Width of vram (1=64) */
	u8	vram_width;
	/** Pixel Mode / PSM */
	u8	psm;
}GS_IMAGE;

#if 0
typedef struct
{
	u8	 img_psm;
	u16	 img_width;
	u16	 img_height;
	u32	*img_addr;

	u8	 clt_psm;
	u16	 clt_csm;
	u16	 clt_width;
	u16	 clt_height;
	u32	*clt_addr;
}GS_EE_IMAGE;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* COMMONLY UDED, LOW LEVEL FUNCTIONS */

/* These Use Gif-Dma To Transfer*/
int GsSetXYOffset1(u16 x, u16 y);
int GsSetXYOffset2(u16 x, u16 y);
int GsSetScissor1(u16 upper_x, u16 upper_y, u16 lower_x, u16 lower_y);
int GsSetScissor2(u16 upper_x, u16 upper_y, u16 lower_x, u16 lower_y);
int GsSetFrame1(u16 framebuffer_addr, u8 framebuffer_width, u8 psm, u32 draw_mask);
int GsSetFrame2(u16 framebuffer_addr, u8 framebuffer_width, u8 psm, u32 draw_mask);
int GsTextureFlush(void);
int GsSetPixelTest1(u8 enable_alpha_test, u8 alpha_test_method, u8 alpha_reference, u8 alpha_fail_method, u8 enable_dest_alpha_test, u8 dest_alpha_test_mode, u8 enable_zbuff_test, u8 alpha_zbuff_method);
int GsSetPixelTest2(u8 enable_alpha_test, u8 alpha_test_method, u8 alpha_reference, u8 alpha_fail_method, u8 enable_dest_alpha_test, u8 dest_alpha_test_mode, u8 enable_zbuff_test, u8 alpha_zbuff_method);
int GsSelectTexure1(u16 tex_addr, u8 addr_width, u8 tex_pixmode, u16 tex_width, u16 tex_height, u16 clut_addr, u8 clut_pixmode, u8 clut_storagemode,u8 clut_offset);
int GsSelectTexure2(u16 tex_addr, u8 addr_width, u8 tex_pixmode, u16 tex_width, u16 tex_height, u16 clut_addr, u8 clut_pixmode, u8 clut_storagemode,u8 clut_offset);
void GsSetFogColor(u8 r, u8 g, u8 b);
void GsEnableColorClamp(u16 enable);

/* NORMAL FUNCTIONS */

GsGParam_t *GsGetGParam(void);
void GsResetGraph(short int mode, short int interlace, short int omode, short int ffmode);
void GsResetPath(void);
void GsSetCRTCSettings(u64 settings, u8 alpha_value);

/* Initialise structs with defaults Based On Input*/
void GsSetDefaultDrawEnv(GS_DRAWENV *drawenv, u16 psm, u16 w, u16 h);
void GsSetDefaultDrawEnvAddress(GS_DRAWENV *drawenv, u16 vram_addr);
void GsSetDefaultDisplayEnv(GS_DISPENV *dispenv, u16 psm, u16 w, u16 h, u16 dx, u16 dy);
void GsSetDefaultDisplayEnvAddress(GS_DISPENV *dispenv, u16 vram_addr);
void GsSetDefaultZBufferEnv(GS_ZENV *zenv, u8 update_mask);
void GsSetDefaultZBufferEnvAddress(GS_ZENV *zenv, u16 vram_addr, u8 psm);

/* Execute struct's data (Environments)*/
void GsPutDrawEnv1(GS_DRAWENV		*drawenv);
void GsPutDrawEnv2(GS_DRAWENV		*drawenv);
void GsPutDisplayEnv1(GS_DISPENV	*dispenv);
void GsPutDisplayEnv2(GS_DISPENV	*dispenv);
void GsPutZBufferEnv1(GS_ZENV *zenv);
void GsPutZBufferEnv2(GS_ZENV *zenv);
/** clear draw buffer with GS_DRAWENV->bg_color color (contex 1) */
void GsClearDrawEnv1(GS_DRAWENV	*drawenv);
/** clear draw buffer with GS_DRAWENV->bg_color color (contex 2) */
void GsClearDrawEnv2(GS_DRAWENV	*drawenv);

/* Gif packet execution*/
QWORD *GsGifPacketsAlloc(GS_PACKET_TABLE *table, u32  num_qwords);
void GsGifPacketsClear(GS_PACKET_TABLE *table);
int GsGifPacketsExecute(GS_PACKET_TABLE	*table, u16 wait);

/* Texture/Image Funtions*/
int GsLoadImage(const void *source_addr, GS_IMAGE *dest);

void GsOverridePrimAttributes(s8 override, s8 iip, s8 tme, s8 fge, s8 abe, s8 aa1, s8 fst, s8 ctxt, s8 fix);
void GsEnableDithering(u8 enable, int mode);
void GsEnableAlphaTransparency1(u16 enable,u16 method,u8 alpha_ref,u16 fail_method);
void GsEnableAlphaTransparency2(u16 enable,u16 method,u8 alpha_ref,u16 fail_method);
void GsEnableZbuffer1(u16 enable,u16 test_method);
void GsEnableZbuffer2(u16 enable,u16 test_method);
void GsEnableAlphaBlending1(u16 enable);
void GsEnableAlphaBlending2(u16 enable);

void GsDrawSync(int mode);
void GsHSync(int mode);
void GsVSync(int mode);

/* Vram Allocation */
int    GsVramAllocFrameBuffer(s16 w, s16 h, s16 psm);
int    GsVramAllocTextureBuffer(s16 w, s16 h, s16 psm);
/** free texture buffer without freeing frame buffer */
void GsVramFreeAllTextureBuffer(void);
void GsVramFreeAll(void);

/* Just used for buffer swapping*/
int    GsDbGetDrawBuffer(void);
int    GsDbGetDisplayBuffer(void);
void GsDbSwapBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* __LIBGS_H__ */
