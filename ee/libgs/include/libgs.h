/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#ifndef _LIBGS_H_
#define _LIBGS_H_

/**/
typedef struct {
	unsigned char interlace;	//Interlace/non-interlace mode.
	unsigned char omode;		//Video mode.
	unsigned char ffmode;		//FIELD/FRAME value.
	unsigned char version;		//GS version.
} GsGParam_t;

#define GS_INIT_RESET		0	//Resets the GS and GIF.
#define GS_INIT_DRAW_RESET	1	//Drawing operations are cancelled and primitive data will be discarded.

#define GS_NONINTERLACED	0x00
#define GS_INTERLACED		0x01

#define GS_FFMD_FIELD		0x00	//Read every other line from the beginning with the start of FIELD.
#define GS_FFMD_FRAME		0x01	//Read every line from the beginning with the start of FRAME.

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

//types of primitives
enum GsPrimitiveTypes{
	GS_PRIM_POINT		=0,
	GS_PRIM_LINE,
	GS_PRIM_LINE_STRIP,
	GS_PRIM_TRI,
	GS_PRIM_TRI_STRIP,
	GS_PRIM_TRI_FAN,
	GS_PRIM_SPRITE
};

// regular Pixel Storage Modes (PSM)
#define GS_PIXMODE_32		 0
#define GS_PIXMODE_24		 1
#define GS_PIXMODE_16		 2
#define GS_PIXMODE_16S		10

// clut Pixel Storage Modes (PSM)
#define GS_CLUT_32			 0
#define GS_CLUT_16			 2
#define GS_CLUT_16S			10

// texture/image Pixel Storage Modes (PSM)
#define GS_TEX_32			 0
#define GS_TEX_24			 1
#define GS_TEX_16			 2
#define GS_TEX_16S			10
#define GS_TEX_8			19
#define GS_TEX_4			20
#define GS_TEX_8H			27
#define GS_TEX_4HL			36
#define GS_TEX_4HH			44

// Z-Buffer Pixel Storage Modes (PSM)
#define GS_ZBUFF_32			48
#define GS_ZBUFF_24			49
#define GS_ZBUFF_16			50
#define GS_ZBUFF_16S		58

// Alpha test Methods
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

// Alpha test failed update Methods
enum GsATestFailedUpdateMethods{
	GS_ALPHA_NO_UPDATE	=0,	//standard
	GS_ALPHA_FB_ONLY,
	GS_ALPHA_ZB_ONLY,
	GS_ALPHA_RGB_ONLY
};

// Zbuffer test Methods
enum GsZTestMethodTypes{
	GS_ZBUFF_NEVER		=0,
	GS_ZBUFF_ALWAYS,
	GS_ZBUFF_GEQUAL,
	GS_ZBUFF_GREATER
};

//Texture Details
#define GS_TEX_CALC	0	//use near/far formula
#define GS_TEX_FIXED	1	//fixed value (use K value)

enum GsTexMipmaps{
	GS_TEX_MIPMAP0	=0,	//no mipmap
	GS_TEX_MIPMAP1,		//1 mipmap
	GS_TEX_MIPMAP2,		//2 mipmaps
	GS_TEX_MIPMAP3,		//...
	GS_TEX_MIPMAP4,		//...
	GS_TEX_MIPMAP5,		//...
	GS_TEX_MIPMAP6		//...
};

enum GsTexFilterMethods{
	GS_TEX_NEAREST			=0,	//UnFiltered
	GS_TEX_LINEAR,				//Filtered
	GS_TEX_NEAREST_MIPMAP_NEAREST,		//
	GS_TEX_NEAREST_MIPMAP_LINEAR,		//
	GS_TEX_LINEAR_MIPMAP_NEAREST,		//
	GS_TEX_LINEAR_MIPMAP_LINEAR		//
};

#define GS_TEX_MIPMAP_DEFINE	0	//use values in MIPTBP1
#define GS_TEX_MIPMAP_AUTO	1	//auto calculate mipmap address

//Texture Function (used in TEX0->tex_cc)
enum GsTexFunctions{
	GS_TEX_MODULATE		=0,	// brighten texture based on Pixel's Alpha
	GS_TEX_DECAL,			// keep texture as is
	GS_TEX_HIGHLIHGT1,		// used when highlighting translucent polygons
	GS_TEX_HIGHLIHGT2		// used when highlighting opaque	  polygons
};

enum GsGifDataFormat{
	GS_GIF_PACKED	=0,
	GS_GIF_REGLIST,
	GS_GIF_IMAGE,
	GS_GIF_DISABLE		//Same operation with the IMAGE mode
};

/*GS Privileged Regs*/
#define gs_p_pmode			0x12000000	// Setup CRT Controller
#define gs_p_smode1			0x12000010	// Video signal settings, undocumented (don't set!)
#define gs_p_smode2			0x12000020	// CRTC Video Settings: PAL/NTCS, Interlace, etc.
#define gs_p_dispfb1		0x12000070	// Setup the CRTC's Read Circuit 1 data source settings
#define gs_p_display1		0x12000080	// RC1 display output settings
#define gs_p_dispfb2		0x12000090	// Setup the CRTC's Read Circuit 2 data source settings
#define gs_p_display2		0x120000a0	// RC2 display output settings
#define gs_p_extbuf			0x120000b0	// ...
#define gs_p_extdata		0x120000c0	// ...
#define gs_p_extwrite		0x120000d0	// ...
#define gs_p_bgcolor		0x120000e0	// Set CRTC background color
#define gs_p_csr			0x12001000	// System status and reset
#define gs_p_imr			0x12001010	// Interrupt Mask Register
#define gs_p_busdir			0x12001040	// Set direction of data transmission FIFO
#define gs_p_siglblid		0x12001080	// Signal\label value

/*GS General Purpose Regs*/
#define gs_g_prim		0x00	// Select and configure current drawing primitive
#define gs_g_rgbaq		0x01	// Setup current vertex color
#define gs_g_st			0x02	// ST map
#define gs_g_uv			0x03	// UV map
#define gs_g_xyzf2		0x04	// Set vertex position and fog coefflcient (with draw kick)
#define gs_g_xyz2		0x05	// Set vertex coordinate (with draw kick)
#define gs_g_tex0_1		0x06	// Select current texture in context 1
#define gs_g_tex0_2		0x07	// Select current texture in context 2
#define gs_g_clamp_1	0x08	// Set texture wrap mode in context 1
#define gs_g_clamp_2	0x09	// Set texture wrap mode in context 2
#define gs_g_fog		0x0a	// Set fog attributes
#define gs_g_xyzf3		0x0c	// Set vertex position and fog coefflcient (no draw kick)
#define gs_g_xyz3		0x0d	// Set vertex position (no draw kick)
#define gs_g_tex1_1		0x14	// ...
#define gs_g_tex1_2		0x15	// ...
#define gs_g_tex2_1		0x16	// Set texture filtering\sampling style in context 1
#define gs_g_tex2_2		0x17	// Set texture filtering\sampling style in context 2
#define gs_g_xyoffset_1	0x18	// Mapping from Primitive to Window coordinate system (Context 1)
#define gs_g_xyoffset_2	0x19	// Mapping from Primitive to Window coordinate system (Context 2)
#define gs_g_prmodecont	0x1a	// gs_g_prim or gs_g_prmode selector
#define gs_g_prmode		0x1b	// attributes of current drawing primitive
#define gs_g_texclut	0x1c	// ...
#define gs_g_scanmsk	0x22	// Raster odd\even line drawing setting
#define gs_g_miptbp1_1	0x34	// Set mipmap address in context 1(mip level 1-3)
#define gs_g_miptbp1_2	0x35	// Set mipmap address in context 1(mip level 1-3)
#define gs_g_miptbp2_1	0x36	// Set mipmap address in context 2(mip level 4-6)
#define gs_g_miptbp2_2	0x37	// Set mipmap address in context 2(mip level 4-6)
#define gs_g_texa		0x3b	// Texture alpha setting
#define gs_g_fogcol		0x3d	// Set fog far color
#define gs_g_texflush	0x3f	// Flush texture buffer/cache
#define gs_g_scissor_1	0x40	// Setup clipping rectangle (Context 1)
#define gs_g_scissor_2	0x41	// Setup clipping rectangle (Context 2)
#define gs_g_alpha_1	0x42	// Alpha blending setting (Context 1)
#define gs_g_alpha_2	0x43	// Alpha blending setting (Context 2)
#define gs_g_dimx		0x44	// Dither matrix values
#define gs_g_dthe		0x45	// Enabel dither matrix
#define gs_g_colclamp	0x46	// Color clamp control
#define gs_g_test_1		0x47	// FrameBuffer\ZBuffer Pixel test contol (Context 1)
#define gs_g_test_2		0x48	// FrameBuffer\ZBuffer Pixel test contol (Context 2)
#define gs_g_pabe		0x49	// Enable alpha blending
#define gs_g_fba_1		0x4a	// Alpha correction value (Context 1)
#define gs_g_fba_2		0x4b	// Alpha correction value (Context 2)
#define gs_g_frame_1	0x4c	// Frame buffer settings (Context 1)
#define gs_g_frame_2	0x4d	// Frame buffer settings (Context 2)
#define gs_g_zbuf_1		0x4e	// Zbuffer configuration (Context 1)
#define gs_g_zbuf_2		0x4f	// Zbuffer configuration (Context 2)
#define gs_g_bitbltbuf	0x50	// Texture transmission address & format
#define gs_g_trxpos		0x51	// Texture transmission coordinates
#define gs_g_trxreg		0x52	// Texture transmission width & height
#define gs_g_trxdir		0x53	// Texture transmission direction
#define gs_g_hwreg		0x54	// ...
#define gs_g_signal		0x60	// ...
#define gs_g_finish		0x61	// ...
#define gs_g_label		0x62	// ...
#define gs_g_nop		0x7f	// no operation\does nothing\can be used as padding

/*GIF Register Descriptors (non-registers) */
#define gif_rd_ad		0x0e	// A+D
#define gif_rd_nop		0x0f	// NOP (Not OutPut)

/*----------------------------------------------------
--	MISC											--
--													--
--													--
----------------------------------------------------*/

#ifndef QWORD
typedef struct {

	unsigned long lo;
	unsigned long hi;

}QWORD			__attribute__((aligned(16)));/*aligned 128bits*/

#endif/*QWORD*/

/*----------------------------------------------------
--	GS Privileged Reg STRUCTs						--
--													--
--													--
----------------------------------------------------*/

/*PMODE*/
typedef struct {
	unsigned enable_rc1	:1;		// Enable ReadCircuit 1
	unsigned enable_rc2	:1;		// Enable ReadCircuit 2
	unsigned crt_out	:3;		// CRT output switching(always 1)
	unsigned mmod		:1;		// Value to use for alpha blend(0=value in 'RC1',1=value in 'blend_value')
	unsigned amod		:1;		// ReadCircuit to output alpha to (0=RC1, 1=RC2)
	unsigned blend_style:1;		// Blend Method(0=blend with RC2, 0=blend with BG)
	unsigned blend_value:8;		// Alpha Blend Value (0-255)
	unsigned nfld		:1;		// Output to NFIELD
	unsigned pad1		:15;	// Pad with zeros
	unsigned exvwins	:10;	// ??
	unsigned exvwine	:10;	// ??
	unsigned exsyncmd	:1;		// ??
	unsigned pad2		:11;	// Pad with zeros
}GS_PMODE;

/*SMODE1*/
typedef struct {
	unsigned long rc	:3;
	unsigned long lc	:7;
	unsigned long t1248	:2;
	unsigned long slck	:1;
	unsigned long cmod	:2;
	unsigned long ex	:1;
	unsigned long prst	:1;
	unsigned long sint	:1;
	unsigned long xpck	:1;
	unsigned long pck2	:2;
	unsigned long spml	:4;
	unsigned long gcont	:1;
	unsigned long phs	:1;
	unsigned long pvs	:1;
	unsigned long pehs	:1;
	unsigned long pevs	:1;
	unsigned long clksel	:2;
	unsigned long nvck	:1;
	unsigned long slck2	:1;
	unsigned long vcksel	:2;
	unsigned long vhp	:2;
	unsigned long pad	:26;
}GS_SMODE1;

/*SMODE2*/
typedef struct {
	unsigned interlace   :1;
	unsigned field_frame :1;
	unsigned vesta_dpms  :2;
	unsigned long pad2	 :60;
}GS_SMODE2;

/*DISPFB*/
typedef struct {
	unsigned address	:9;	// Base pointer in VRam
	unsigned fbw		:6;	// Buffer width in VRam
	unsigned psm	:5;	// Pixel store mode
	unsigned pad1		:12;// Pad with zeros
	unsigned x			:11;// X Pos in  in VRam
	unsigned y			:11;// Y Pos in  in VRam
	unsigned pad2		:10;// Pad with zeros
}GS_DISPFB;

/*DISPLAY*/
typedef struct {
	unsigned display_x:12;	// Display area X pos
	unsigned display_y:11;	// Display area Y pos
	unsigned magnify_h:4;	// Horizontal magnification
	unsigned magnify_v:2;	// Vertical   magnification
	unsigned pad1	  :3;	// Pad with zeros
	unsigned display_w:12;	// Display area width
	unsigned display_h:11;	// Display area height
	unsigned pad2     :9;	// Pad with zeros
}GS_DISPLAY;

/*EXTBUF*/
typedef struct {
	unsigned long exbp	: 14;
	unsigned long exbw	: 6;
	unsigned long fbin	: 2;
	unsigned long wffmd	: 1;
	unsigned long emoda	: 2;
	unsigned long emodc	: 2;
	unsigned long pad1	: 5;
	unsigned long wdx	: 11;
	unsigned long wdy	: 11;
	unsigned long pad2	: 10;
}GS_EXTBUF;

/*EXTDATA*/
typedef struct {
	unsigned x            :12;	// X coord where image is written to
	unsigned y            :11;	// Y coord where image is written to
	unsigned sample_r_h   :4;	// Horizontal Smaple Rate(VK units)
	unsigned sample_r_v   :2;	// Vertical   Smaple Rate
	unsigned pad1		  :3;	// Pad with zeros
	unsigned write_w      :12;	// Width  of area to write
	unsigned write_h      :11;	// Height of area to write
	unsigned pad2		  :9;	// Pad with zeros
}GS_EXTDATA;

/*EXTWRITE*/
typedef struct {
	unsigned     write    :1;	// Write Control(0=write done, 1=write start)
	unsigned     pad1	  :31;	// Pad with zeros
    unsigned int pad2;			// Pad with zeros
}GS_EXTWRITE;

/*BGCOLOR*/
typedef struct {
	unsigned char	r;		// Background Color Red
	unsigned char	g;		// Background Color Green
	unsigned char	b;		// Background Color Blue
	unsigned char	pada;	// 0x0
	float			padq;	// 0x0
}GS_BGCOLOR;

/*CSR*/
typedef struct {
	unsigned signal_evnt    :1;	// Signal event control(write: 0=nothing,1=enable signal event,  read: 0=signal not generated, 1=signal generated)
	unsigned finish_evnt    :1;	// Finish event control(write: 0=nothing,1=enable finish event,  read: 0=finish not generated, 1=finish generated)
	unsigned hsync_intrupt  :1;	// HSync interrupt ,,   ,,   ,,   ,,
	unsigned vsync_intrupt  :1;	// VSync interrupt ,,    ,,   ,,   ,,
	unsigned write_terminate:1;	// Write termination control ,,   ,,    ,,   ,,
	unsigned exhsint		:1;	// ??
	unsigned exvsint		:1;	// ??
	unsigned pad1			:1; // Pad with zeros
	unsigned flush			:1;	// Flush GS
	unsigned reset			:1;	// Reset GS
	unsigned exverr			:1;	// ??
	unsigned exfield		:1;	// ??
	unsigned nfield			:1;	// NFIELD output
	unsigned current_field  :1;	// Currnet displayed field
	unsigned fifo_status	:2;	// Host interface FIFO status
	unsigned gs_rev_number	:8;	// Revision number of GS
	unsigned gs_id			:8;	// id of GS
	unsigned pad2			:32;// Pad with zeros
}GS_CSR;

/*IMR*/
typedef struct {
	unsigned pad1			:8;	// Pad with zeros
	unsigned signal_mask	:1;	// Signal event interrupt mask
	unsigned finish_mask	:1;	// Finish event interrupt mask
	unsigned hsync_mask		:1;	// HSync interrupt mask
	unsigned vsync_mask		:1;	// VSync interrupt mask
	unsigned write_mask		:1;	// Write termination mask
	unsigned exhs_mask		:1;	// ??
	unsigned exvs_mask		:1;	// ??
	unsigned pad2			:17;// Pad with zeros
    unsigned int pad3;			// Pad with zeros
}GS_IMR;

/*BUSDIR*/
typedef struct {
	unsigned		direction		:1;	// Transmission direction(0=host->local, 1=host<-local)
	unsigned		p0				:31;// Pad with zeros
    unsigned int	p1;			// Pad with more zeros
}GS_BUSDIR;

/*SIGLBLID*/
/*typedef struct {
	unsigned int id;	// SIGNAL register id
	unsigned int p0;
}GS_SIGLBLID;
*/

/*--------------------------------------------------------
--	GENERAL PURPOSE REG STRUCTS							--
--														--
--	these structs have a size of 64 bits 				--
----------------------------------------------------------*/

typedef struct {
	unsigned long prim_type	:3;
	unsigned long iip		:1;
	unsigned long tme		:1;
	unsigned long fge		:1;
	unsigned long abe		:1;
	unsigned long aa1		:1;
	unsigned long fst		:1;
	unsigned long ctxt		:1;
	unsigned long fix		:1;
	unsigned long pad1		:53;
}GS_PRIM;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	float		  q;
}GS_RGBAQ;

typedef struct {
	float			s;
	float			t;
}GS_ST;

typedef struct {
	unsigned long u		:14;
	unsigned long pad1	:2;
	unsigned long v		:14;
	unsigned long pad2	:34;
}GS_UV;

typedef struct {
	unsigned short	x;
	unsigned short	y;
	unsigned int	z:24;
	unsigned char	f;
}GS_XYZF;

typedef struct {
	unsigned short	x;
	unsigned short	y;
	unsigned int	z;
}GS_XYZ;

typedef struct {
	unsigned long tb_addr		:14;
	unsigned long tb_width		:6;
	unsigned long psm		:6;
	unsigned long tex_width		:4;
	unsigned long tex_height	:4;
	unsigned long tex_cc		:1;
	unsigned long tex_funtion	:2;
	unsigned long cb_addr		:14;
	unsigned long clut_pixmode	:4;
	unsigned long clut_smode	:1;
	unsigned long clut_offset	:5;
	unsigned long clut_loadmode	:3;

}GS_TEX0;

typedef struct {
	unsigned long wrap_mode_s	:2;
	unsigned long wrap_mode_t	:2;
	unsigned long min_clamp_u	:10;
	unsigned long max_clamp_u	:10;
	unsigned long min_clamp_v	:10;
	unsigned long max_clamp_v	:10;
	unsigned long pad0			:20;
}GS_CLAMP;

typedef struct {
		unsigned long pad1	:56;
		unsigned char f;
}GS_FOG;

typedef struct {
	unsigned long lcm			:1;
	unsigned long pad1			:1;
	unsigned long mxl			:3;
	unsigned long mmag			:1;
	unsigned long mmin			:3;
	unsigned long mtba			:1;
	unsigned long pad2			:9;
	unsigned long l				:2;
	unsigned long pad3			:11;
	unsigned long k				:12;
	unsigned long pad4			:20;
}GS_TEX1;

typedef struct {
	unsigned long pad1			:20;
	unsigned long psm			:6;
	unsigned long pad2			:11;
	unsigned long cb_addr		:14;
	unsigned long clut_psm		:4;
	unsigned long clut_smode	:1;
	unsigned long clut_offset	:5;
	unsigned long clut_loadmode	:3;
}GS_TEX2;

typedef struct {
	unsigned long  offset_x	:16;
	unsigned short pad1;
	unsigned long  offset_y	:16;
	unsigned short pad2;
} GS_XYOFFSET;

typedef struct {
	unsigned long control:1;
	unsigned long pad1	:63;
}GS_PRMODECONT;

typedef struct {
	unsigned long pad1	:3;
	unsigned long iip	:1;
	unsigned long tme	:1;
	unsigned long fge	:1;
	unsigned long abe	:1;
	unsigned long aa1	:1;
	unsigned long fst	:1;
	unsigned long ctxt	:1;
	unsigned long fix	:1;
	unsigned long pad2	:53;
}GS_PRMODE;

typedef struct {
	unsigned long cb_width		:6;
	unsigned long clut_uoffset	:6;
	unsigned long clut_voffset	:10;
	unsigned long pad0:42;
}GS_TEXCLUT;

typedef struct {
	unsigned long mask:2;
	unsigned long pad0:62;
}GS_SCANMSK;

typedef struct {
	unsigned long tb_addr1	:14;
	unsigned long tb_width1	:6;
	unsigned long tb_addr2	:14;
	unsigned long tb_width2	:6;
	unsigned long tb_addr3	:14;
	unsigned long tb_width3	:6;
	unsigned long pad1		:4;
}GS_MIPTBP1;

typedef struct {
	unsigned long tb_addr4	:14;
	unsigned long tb_width4	:6;
	unsigned long tb_addr5	:14;
	unsigned long tb_width5	:6;
	unsigned long tb_addr6	:14;
	unsigned long tb_width6	:6;
	unsigned long pad0		:4;
}GS_MIPTBP2;

typedef struct {
	unsigned long alpha_0		: 8;
	unsigned long pad1			: 7;
	unsigned long alpha_method	: 1;
	unsigned long pad2			:16;
	unsigned long alpha_1		: 8;
	unsigned long pad3			:24;
}GS_TEXA;

typedef struct {
	unsigned long r		:8;
	unsigned long g		:8;
	unsigned long b		:8;
	unsigned long pad1	:40;
}GS_FOGCOLOR;

typedef struct {
	unsigned long pad1;			// Pad With Zeros
} GS_TEXFLUSH;

typedef struct {
	unsigned long clip_x0 :11;
	unsigned long pad1    :5;
	unsigned long clip_x1 :11;
	unsigned long pad2    :5;
	unsigned long clip_y0 :11;
	unsigned long pad3    :5;
	unsigned long clip_y1 :11;
	unsigned long pad4    :5;
}GS_SCISSOR;

typedef struct {
	unsigned long a		:2;
	unsigned long b		:2;
	unsigned long c		:2;
	unsigned long d		:2;
	unsigned long pad0	:24;
	unsigned long alpha	:8;
	unsigned long pad1	:24;
}GS_ALPHA;

typedef struct {
	unsigned long dimx00:3;
	unsigned long pad0:1;
	unsigned long dimx01:3;
	unsigned long pad1:1;
	unsigned long dimx02:3;
	unsigned long pad2:1;
	unsigned long dimx03:3;
	unsigned long pad3:1;

	unsigned long dimx10:3;
	unsigned long pad4:1;
	unsigned long dimx11:3;
	unsigned long pad5:1;
	unsigned long dimx12:3;
	unsigned long pad6:1;
	unsigned long dimx13:3;
	unsigned long pad7:1;

	unsigned long dimx20:3;
	unsigned long pad8:1;
	unsigned long dimx21:3;
	unsigned long pad9:1;
	unsigned long dimx22:3;
	unsigned long pad10:1;
	unsigned long dimx23:3;
	unsigned long pad11:1;

	unsigned long dimx30:3;
	unsigned long pad12:1;
	unsigned long dimx31:3;
	unsigned long pad13:1;
	unsigned long dimx32:3;
	unsigned long pad14:1;
	unsigned long dimx33:3;
	unsigned long pad15:1;
} GS_DIMX;

typedef struct {
	unsigned long enable:1;
	unsigned long pad01:63;
} GS_DTHE;

typedef struct {
	unsigned long clamp:1;
	unsigned long pad01:63;
}GS_COLCLAMP;

typedef struct {
	unsigned long atest_enable		:1;
	unsigned long atest_method		:3;
	unsigned long atest_reference	:8;
	unsigned long atest_fail_method	:2;
	unsigned long datest_enable		:1;
	unsigned long datest_mode		:1;
	unsigned long ztest_enable		:1;
	unsigned long ztest_method		:2;
	unsigned long pad1				:45;
} GS_TEST;

typedef struct {
	unsigned long enable:1;
	unsigned long pad0:63;
}GS_PABE;

typedef struct {
	unsigned long alpha:1;
	unsigned long pad0:63;
}GS_FBA;

typedef struct {
	unsigned long fb_addr	:9;
	unsigned long pad1		:7;
	unsigned long fb_width	:6;
	unsigned long pad2		:2;
	unsigned long psm	:6;
	unsigned long pad3		:2;
	unsigned long draw_mask	:32;
} GS_FRAME;

typedef struct {
	unsigned long fb_addr	:9;
	unsigned long pad1			:15;
	unsigned long psm		:4;
	unsigned long pad2			:4;
	unsigned long update_mask	:1;
	unsigned long pad3			:31;
}GS_ZBUF;

typedef struct {
	unsigned long src_addr	  :14;
	unsigned long pad1		  :2;
	unsigned long src_width	  :6;
	unsigned long pad2		  :2;
	unsigned long src_pixmode :6;
	unsigned long pad3		  :2;
	unsigned long dest_addr	  :14;
	unsigned long pad4		  :2;
	unsigned long dest_width  :6;
	unsigned long pad5		  :2;
	unsigned long dest_pixmode:6;
	unsigned long pad6		  :2;
}GS_BITBLTBUF;

typedef struct {
	unsigned long src_x		:11;
	unsigned long pad1		:5;
	unsigned long src_y		:11;
	unsigned long pad2		:5;
	unsigned long dest_x	:11;
	unsigned long pad3		:5;
	unsigned long dest_y	:11;
	unsigned long direction	:2;
	unsigned long pad4		:3;
}GS_TRXPOS;

typedef struct {
	unsigned long trans_w	:12;
	unsigned long pad1		:20;
	unsigned long trans_h	:12;
	unsigned long pad2		:20;
}GS_TRXREG;


typedef struct {
	unsigned long trans_dir	:2;
	unsigned long pad1		:62;
}GS_TRXDIR;

typedef struct {
	unsigned long data;
}GS_HWREG;

typedef struct {
	unsigned int signal_id;
	unsigned int update_mask;
}GS_SIGNAL;

typedef struct {
	unsigned long pad0;
}GS_FINISH;


typedef struct {
	unsigned int label_id;
	unsigned int update_mask;
}GS_LABEL;

typedef struct {
	unsigned long pad0;
}GS_NOP;

/*--------------------------------------------------------
--	GENERAL PURPOSE REG STRUCTS	'WITH' A 64 BIT REG		--
--														--
--	these structs have a size of 128 bits (1 QWORD)		--
----------------------------------------------------------*/

typedef struct {
	GS_PRIM			data;
	unsigned long	reg;
}GS_R_PRIM;

typedef struct {
	GS_RGBAQ		data;
	unsigned long	reg;
}GS_R_RGBAQ;

typedef struct {
	GS_ST			data;
	unsigned long	reg;
}GS_R_ST;

typedef struct {
	GS_UV			data;
	unsigned long	reg;
}GS_R_UV;

typedef struct {
	GS_XYZF			data;
	unsigned long	reg;
}GS_R_XYZF;

typedef struct {
	GS_XYZ			data;
	unsigned long	reg;
}GS_R_XYZ;

typedef struct {
	GS_TEX0			data;
	unsigned long	reg;
}GS_R_TEX0;

typedef struct {
	GS_CLAMP		data;
	unsigned long	reg;
}GS_R_CLAMP;

typedef struct {
	GS_FOG			data;
	unsigned long	reg;
}GS_R_FOG;

typedef struct {
	GS_TEX1			data;
	unsigned long	reg;
}GS_R_TEX1;

typedef struct {
	GS_TEX2			data;
	unsigned long	reg;
}GS_R_TEX2;

typedef struct {
	GS_XYOFFSET		data;
	unsigned long	reg;
} GS_R_XYOFFSET;

typedef struct {
	GS_PRMODECONT	data;
	unsigned long	reg;
}GS_R_PRMODECONT;

typedef struct {
	GS_PRMODE		data;
	unsigned long	reg;
}GS_R_PRMODE;

typedef struct {
	GS_TEXCLUT		data;
	unsigned long	reg;
}GS_R_TEXCLUT;

typedef struct {
	GS_SCANMSK		data;
	unsigned long	reg;
}GS_R_SCANMSK;

typedef struct {
	GS_MIPTBP1		data;
	unsigned long	reg;
} GS_R_MIPTBP1;

typedef struct {
	GS_MIPTBP2		data;
	unsigned long	reg;
}GS_R_MIPTBP2;

typedef struct {
	GS_TEXA			data;
	unsigned long	reg;
}GS_R_TEXA;

typedef struct {
	GS_FOGCOLOR		data;
	unsigned long	reg;
}GS_R_FOGCOLOR;

typedef struct {
	GS_TEXFLUSH		data;
	unsigned long	reg;
} GS_R_TEXFLUSH;

typedef struct {
	GS_SCISSOR		data;
	unsigned long	reg;
}GS_R_SCISSOR;

typedef struct {
	GS_ALPHA		data;
	unsigned long	reg;
}GS_R_ALPHA;

typedef struct {
	GS_DIMX			data;
	unsigned long	reg;
} GS_R_DIMX;

typedef struct {
	GS_DTHE			data;
	unsigned long	reg;
} GS_R_DTHE;

typedef struct {
	GS_COLCLAMP		data;
	unsigned long	reg;
}GS_R_COLCLAMP;

typedef struct {
	GS_TEST			data;
	unsigned long	reg;
} GS_R_TEST;

typedef struct {
	GS_PABE			data;
	unsigned long	reg;
}GS_R_PABE;

typedef struct {
	GS_FBA			data;
	unsigned long	reg;
}GS_R_FBA;

typedef struct {
	GS_FRAME		data;
	unsigned long	reg;
} GS_R_FRAME;

typedef struct {
	GS_ZBUF			data;
	unsigned long	reg;
}GS_R_ZBUF;

typedef struct {
	GS_BITBLTBUF	data;
	unsigned long	reg;
}GS_R_BITBLTBUF;

typedef struct {
	GS_TRXPOS		data;
	unsigned long	reg;
}GS_R_TRXPOS;

typedef struct {
	GS_TRXREG		data;
	unsigned long	reg;
}GS_R_TRXREG;

typedef struct {
	GS_TRXDIR		data;
	unsigned long	reg;
}GS_R_TRXDIR;

typedef struct {
	GS_HWREG		data;
	unsigned long	reg;
}GS_R_HWREG;

typedef struct {
	GS_SIGNAL		data;
	unsigned long	reg;
}GS_R_SIGNAL;

typedef struct {
	GS_FINISH		data;
	unsigned long	reg;
}GS_R_FINISH;

typedef struct {
	GS_LABEL		data;
	unsigned long	reg;
}GS_R_LABEL;

typedef struct {
	GS_NOP			data;
	unsigned long	reg;
}GS_R_NOP;

/*----------------------------------------------------
--	Set Funtion For GS Privileged Regs				--
--													--
--													--
----------------------------------------------------*/

#define GS_SET_PMODE(enable_rc1, enable_rc2, mmod, amod, blend_style, blend_value) \
			*(volatile unsigned long *)gs_p_pmode =			\
		(unsigned long)((enable_rc1	) & 0x00000001) <<  0 | \
		(unsigned long)((enable_rc2	) & 0x00000001) <<  1 | \
		(unsigned long)((1			) & 0x00000007) <<  2 | \
		(unsigned long)((mmod		) & 0x00000001) <<  5 | \
		(unsigned long)((amod		) & 0x00000001) <<  6 | \
		(unsigned long)((blend_style) & 0x00000001) <<  7 | \
		(unsigned long)((blend_value) & 0x000000FF) <<  8

//Set by SetGsCrt(). DO NOT SET MANUALLY!!
#define GS_SET_SMODE1(rc, lc, t1248, slck, cmod, ex, prst, sint, xpck,		\
			pck2, spml, gcont, phs, pvs, pehs, pevs, clksel,	\
			nvck, slck2, vcksel, vhp) \
			*(volatile unsigned long *)gs_p_smode1 =		\
		(unsigned long)((rc	) & 0x00000007) <<  0 | \
		(unsigned long)((lc	) & 0x0000007F) <<  3 | \
		(unsigned long)((t1248	) & 0x00000003) << 10 | \
		(unsigned long)((slck	) & 0x00000001) << 12 | \
		(unsigned long)((cmod	) & 0x00000003) << 13 | \
		(unsigned long)((ex	) & 0x00000001) << 15 | \
		(unsigned long)((prst	) & 0x00000001) << 16 | \
		(unsigned long)((sint	) & 0x00000001) << 17 | \
		(unsigned long)((xpck	) & 0x00000001) << 18 | \
		(unsigned long)((pck2	) & 0x00000003) << 19 | \
		(unsigned long)((spml	) & 0x0000000F) << 21 | \
		(unsigned long)((gcont	) & 0x00000001) << 25 | \
		(unsigned long)((phs	) & 0x00000001) << 26 | \
		(unsigned long)((pvs	) & 0x00000001) << 27 | \
		(unsigned long)((pehs	) & 0x00000001) << 28 | \
		(unsigned long)((pevs	) & 0x00000001) << 29 | \
		(unsigned long)((clksel	) & 0x00000003) << 30 | \
		(unsigned long)((nvck	) & 0x00000001) << 32 | \
		(unsigned long)((slck2	) & 0x00000001) << 33 | \
		(unsigned long)((vcksel	) & 0x00000003) << 34 | \
		(unsigned long)((vhp	) & 0x00000003) << 36

#define GS_SET_SMODE2(interlace, field_frame, vesta_dpms) \
			*(volatile unsigned long *)gs_p_smode2 =		\
		(unsigned long)((interlace	) & 0x00000001) <<  0 | \
		(unsigned long)((field_frame) & 0x00000001) <<  1 | \
		(unsigned long)((vesta_dpms	) & 0x00000003) <<  2

#define GS_SET_DISPFB1(address, width, psm, x, y) \
			*(volatile unsigned long *)gs_p_dispfb1=\
		(unsigned long)((address	) & 0x000001FF) <<  0 | \
		(unsigned long)((width		) & 0x0000003F) <<  9 | \
		(unsigned long)((psm	) & 0x0000001F) << 15 | \
		(unsigned long)((x			) & 0x000007FF) << 32 | \
		(unsigned long)((y			) & 0x000007FF) << 43

#define GS_SET_DISPFB2(address, width, psm, x, y) \
			*(volatile unsigned long *)gs_p_dispfb2=\
		(unsigned long)((address	) & 0x000001FF) <<  0 | \
		(unsigned long)((width		) & 0x0000003F) <<  9 | \
		(unsigned long)((psm	) & 0x0000001F) << 15 | \
		(unsigned long)((x			) & 0x000007FF) << 32 | \
		(unsigned long)((y			) & 0x000007FF) << 43

#define GS_SET_DISPLAY1(display_x, display_y,magnify_h,magnify_v,display_w,display_h) \
			*(volatile unsigned long *)gs_p_display1 =		\
		(unsigned long)((display_x) & 0x00000FFF) <<  0 |	\
		(unsigned long)((display_y) & 0x000007FF) << 12 |	\
		(unsigned long)((magnify_h) & 0x0000000F) << 23 |	\
		(unsigned long)((magnify_v) & 0x00000003) << 27 |	\
		(unsigned long)((display_w) & 0x00000FFF) << 32 |	\
		(unsigned long)((display_h) & 0x000007FF) << 44

#define GS_SET_DISPLAY2(display_x, display_y,magnify_h,magnify_v,display_w,display_h) \
			*(volatile unsigned long *)gs_p_display2 =		\
		(unsigned long)((display_x) & 0x00000FFF) <<  0 |	\
		(unsigned long)((display_y) & 0x000007FF) << 12 |	\
		(unsigned long)((magnify_h) & 0x0000000F) << 23 |	\
		(unsigned long)((magnify_v) & 0x00000003) << 27 |	\
		(unsigned long)((display_w) & 0x00000FFF) << 32 |	\
		(unsigned long)((display_h) & 0x000007FF) << 44

#define GS_SET_EXTBUF(A,B,C,D,E,F,G,H) \
			*(volatile unsigned long *)gs_p_extbuf =	\
		(unsigned long)((A) & 0x00003FFF) <<  0 | \
		(unsigned long)((B) & 0x0000003F) << 14 | \
		(unsigned long)((C) & 0x00000003) << 20 | \
		(unsigned long)((D) & 0x00000001) << 22 | \
		(unsigned long)((E) & 0x00000003) << 23 | \
		(unsigned long)((F) & 0x00000003) << 25 | \
		(unsigned long)((G) & 0x000007FF) << 32 | \
		(unsigned long)((H) & 0x000007FF) << 43

#define GS_SET_EXTDATA(x, y, sample_r_h, sample_r_v, write_w, write_h) \
			*(volatile unsigned long *)gs_p_extdata =		\
		(unsigned long)((x			) & 0x00000FFF) <<  0 | \
		(unsigned long)((y			) & 0x000007FF) << 12 | \
		(unsigned long)((sample_r_h	) & 0x0000000F) << 23 | \
		(unsigned long)((sample_r_v	) & 0x00000003) << 27 | \
		(unsigned long)((write_w	) & 0x00000FFF) << 32 | \
		(unsigned long)((write_h	) & 0x000007FF) << 44

#define GS_SET_EXTWRITE(write)\
			*(volatile unsigned long *)gs_p_extwrite = \
		(unsigned long)((write) & 0x00000001)

#define GS_SET_BGCOLOR(r,g,b) \
			*(volatile unsigned long *)gs_p_bgcolor =	\
		(unsigned long)((r) & 0x000000FF) <<  0 | \
		(unsigned long)((g) & 0x000000FF) <<  8 | \
		(unsigned long)((b) & 0x000000FF) << 16

#define GS_SET_CSR(signal_evnt,finish_evnt,hsync_intrupt,vsync_intrupt,write_terminate,flush,reset,nfield,current_field,fifo_status,gs_rev_number,gs_id) \
			*(volatile unsigned long *)gs_p_csr =				\
		(unsigned long)((signal_evnt	) & 0x00000001) <<  0 | \
		(unsigned long)((finish_evnt	) & 0x00000001) <<  1 | \
		(unsigned long)((hsync_intrupt	) & 0x00000001) <<  2 | \
		(unsigned long)((vsync_intrupt	) & 0x00000001) <<  3 | \
		(unsigned long)((write_terminate) & 0x00000001) <<  4 | \
		(unsigned long)((flush			) & 0x00000001) <<  8 | \
		(unsigned long)((reset			) & 0x00000001) <<  9 | \
		(unsigned long)((nfield			) & 0x00000001) << 12 | \
		(unsigned long)((current_field	) & 0x00000001) << 13 | \
		(unsigned long)((fifo_status	) & 0x00000003) << 14 | \
		(unsigned long)((gs_rev_number	) & 0x000000FF) << 16 | \
		(unsigned long)((gs_id			) & 0x000000FF) << 24

#define GS_SET_IMR(signal_mask, finish_mask, hsync_mask, vsync_mask, write_mask, exhs_mask, exvs_mask) \
			*(volatile unsigned long *)gs_p_imr =			\
		(unsigned long)((signal_mask) & 0x00000001) <<  8 | \
		(unsigned long)((finish_mask) & 0x00000001) <<  9 | \
		(unsigned long)((hsync_mask	) & 0x00000001) << 10 | \
		(unsigned long)((vsync_mask	) & 0x00000001) << 11 | \
		(unsigned long)((write_mask	) & 0x00000001) << 12 | \
		(unsigned long)((exhs_mask	) & 0x00000001) << 13 | \
		(unsigned long)((exvs_mask	) & 0x00000001) << 14

#define GS_SET_BUSDIR(direction) \
			*(volatile unsigned long *)gs_p_busdir = \
		(unsigned long)((direction) & 0x00000001)

#define GS_SET_SIGLBLID(signal_id, label_id) \
			*(volatile unsigned long *)gs_p_siglblid =	\
		(unsigned long)((signal_id	) & 0xFFFFFFFF) <<  0 | \
		(unsigned long)((label_id	) & 0xFFFFFFFF) << 32

/*
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
x These are use to SET the individual values		x
x in each of the readable Privileged registers.		x
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/

/*CSR */
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
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
x These are use to GET the individual values		x
x in each of the readable Privileged registers.		x
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/

/*CSR */
#define GS_GET_CSR_signal_evnt \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 0))

#define GS_GET_CSR_finish_evnt \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 1))

#define GS_GET_CSR_hsync_intrupt \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 2))

#define GS_GET_CSR_vsync_intrupt \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 3))

#define GS_GET_CSR_write_terminate \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 4))
/*flush (w)*/

/*reset (w)*/

#define GS_GET_CSR_write_nfield \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 12))

#define GS_GET_CSR_current_field \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000001 << 13))

#define GS_GET_CSR_fifo_status \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x00000003 << 14))

#define GS_GET_CSR_gs_rev_number \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x000000FF << 16))

#define GS_GET_CSR_gs_id \
			(*((volatile unsigned long *)(gs_p_csr)) & (0x000000FF << 24))

/*--------------------------------------------------------
--	'SET' GENERAL PURPOSE REG STRUCTS 'WITHOUT' REG		--
--														--
--														--
----------------------------------------------------------*/

//PRIM
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

//RGBAQ
#define gs_setRGBAQ(p, _r,_g,_b,_a,_q)	\
	(p)->r = _r,						\
	(p)->g = _g,						\
	(p)->b = _b,						\
	(p)->a = _a,						\
	(p)->q = _q

//ST
#define gs_setST(p, _s,_t)	\
	(p)->s = _s,			\
	(p)->t = _t

//UV
#define gs_setUV(p, _u,_v)	\
	(p)->u = _u,			\
	(p)->v = _v

//XYZF2
#define gs_setXYZF2(p, _x,_y,_z,_f)	\
	(p)->x = _x,						\
	(p)->y = _y,						\
	(p)->z = _z,						\
	(p)->f = _f

//XYZF3
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

/*--------------------------------------------------------
--	'SET' GENERAL PURPOSE REG STRUCTs & REGs			--
--														--
--														--
----------------------------------------------------------*/

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

/*----------------------------------------------------
--	MISC											--
--													--
--													--
----------------------------------------------------*/

/*SOURCE CHAIN TAG for DMA CHAIN MODE*/
typedef struct _GS_GIF_DMACHAIN_TAG{
	unsigned long	qwc	:16;
	unsigned long	pad1	:10;
	unsigned long	pce	:2;
	unsigned long	id	:3;
	unsigned long	irq	:1;
	unsigned long	addr	:31;
	unsigned long	spr	:1;
	unsigned long	pad2	:64;
}GS_GIF_DMACHAIN_TAG		__attribute__ ((aligned(16)));/*aligned 128bits*/

/*GIFTAG*/
typedef struct {
	unsigned long nloop	:15;
	unsigned long eop	:1;
	unsigned long pad1	:30;
	unsigned long pre	:1;
	unsigned long prim	:11;
	unsigned long flg	:2;
	unsigned long nreg	:4;
	unsigned long reg	:64;
}GS_GIF_TAG;

#define gs_setGIF_TAG(p, _nloop,_eop,_pre,_prim,_flg,_nreg,_reg)\
	(p)->nloop	= _nloop,				\
	(p)->eop	= _eop,					\
	(p)->pre	= _pre,					\
	(p)->prim	= _prim,				\
	(p)->flg	= _flg,					\
	(p)->nreg	= _nreg,				\
	(p)->reg	= _reg

/*----------------------------------------------------
--	MID LEVEL DEFINES								--
--													--
--													--
------------------------------------------------------*/

/* settings for GsSetCRTCSettings() */

/*A Default setting*/
#define CRTC_SETTINGS_DEFAULT1		CRTC_SETTINGS_EN1|CRTC_SETTINGS_BLENDVAL|CRTC_SETTINGS_OUTRC1|CRTC_SETTINGS_STYLERC1
#define CRTC_SETTINGS_DEFAULT2		CRTC_SETTINGS_EN2|CRTC_SETTINGS_BLENDVAL|CRTC_SETTINGS_OUTRC1|CRTC_SETTINGS_STYLERC1

/*setting*/
#define CRTC_SETTINGS_EN1			((unsigned long)(1)<<0)				// Enable RC1(ReadCircuit 1)
#define CRTC_SETTINGS_EN2			((unsigned long)(1)<<1)				// Enable RC2(ReadCircuit 1)
#define CRTC_SETTINGS_ENBOTH		CRTC_SETTINGS_EN1|CRTC_SETTINGS_EN2	// Enable RC1 & R2
#define CRTC_SETTINGS_BLENDRC1		((unsigned long)(0)<<5)				// Use Alpha value from rc1 for blending
#define CRTC_SETTINGS_BLENDVAL		((unsigned long)(1)<<5)				// Use Alpha value from alpha_value of GsSetCRTCSettings() for blending
#define CRTC_SETTINGS_OUTRC1		((unsigned long)(0)<<6)				// Output Final image to RC1
#define CRTC_SETTINGS_OUTRC2		((unsigned long)(1)<<6)				// Output Final image to RC2
#define CRTC_SETTINGS_STYLERC1		((unsigned long)(0)<<7)				// Blend With The Out Put of RC1
#define CRTC_SETTINGS_STYLEBG		((unsigned long)(1)<<7)				// Blend With The Out Put of BG(background)

typedef struct {
	short	x;
	short	y;
	short	w;
	short	h;
}GS_RECT;

typedef struct {
	unsigned short	x;
	unsigned short	y;
	unsigned short	w;
	unsigned short	h;
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
	unsigned short	vram_addr;
	unsigned char	psm;
	unsigned char	update_mask;
}GS_ZENV;

/*Screen Draw Environment*/
typedef struct {
	unsigned short	offset_x;	// Draw offset X
	unsigned short	offset_y;	// Draw offset Y
	GS_URECT	clip;		// Draw Clip rect
	unsigned short	vram_addr;	// Vram Address in frame buffer
	unsigned char	fbw;		// Width of vram (1=64)
	unsigned char	psm;		// Pixel Mode / PSM
	unsigned short	vram_x;		// X offset in vram;
	unsigned short	vram_y;		// Y offset in vram;
	unsigned int	draw_mask;	// Draw Mask (0=draw, 1=no draw)
	unsigned char	auto_clear;	// Set To 1 If You Want The Draw Environment's Backgroud to Clear When GsPutDrawEnv() is called
	GS_RGBAQ		bg_color;	// Color to use to clear backgroud
}GS_DRAWENV;

/*Screen Display Environment*/
typedef struct {
	GS_DISPLAY	disp;
	GS_DISPFB	dispfb;
}GS_DISPENV __attribute__ ((aligned(8)));/* Aligned, 64bits*/

/*a pixel  */
typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// point Color
	GS_R_XYZ		xyz0;		// vertex coordinate
}GS_POINT_P;					/*Size= 3 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// point color
	GS_R_XYZ		xyz0;		// Primative vertex coordinate
}GS_POINT;						/*Size= 2 QWords*/

/*A Flat Line*/
typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// line color
	GS_R_XYZ		xyz0;		// vertex coordinate
	GS_R_XYZ		xyz1;		// vertex coordinate
}GS_LINE_F1_P;					/*Size = 4 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// line color
	GS_R_XYZ		xyz0;		// vertex coordinate
	GS_R_XYZ		xyz1;		// vertex coordinate
}GS_LINE_F1;					/*Size = 3 QWords*/

/*A Gouraud Line */
typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// vertex 0 color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// vertex 1 color
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
}GS_LINE_G1_P;					/*Size= 5 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// vertex 0 color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// vertex 1 color
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
}GS_LINE_G1;					/*Size= 4 QWords*/

/*A Flat Triangle / Triangle Strip / Triangle Fan */
typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// tringle color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_F3_P;				/*Size= 5 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// triangle color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_F3;				/*Size= 4 QWords*/

typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// triangle color
	GS_R_UV			uv0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_UV			uv1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_UV			uv2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_FT3_P;				/*Size= 8 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// triangle color
	GS_R_UV			uv0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_UV			uv1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_UV			uv2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_FT3;				/*Size= 7 QWords*/

typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// triangle color
	GS_R_ST			st0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_ST			st1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_ST			st2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_FT3ST_P;			/*Size= 8 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// triangle color
	GS_R_ST			st0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_ST			st1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_ST			st2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_FT3ST;				/*Size= 7 QWords*/

/*A Gouraud Triangle / Tri-Strip / Tri-Fan */
typedef struct
{
	GS_R_PRIM		prim;		// primitive attributes
	GS_R_RGBAQ		rgbaq0;		// vertex 0 color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// vertex 1 color
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_RGBAQ		rgbaq2;		// vertex 2 color
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_G3_P;				/*Size= 7 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// vertex 0 color
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// vertex 1 color
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_RGBAQ		rgbaq2;		// vertex 2 color
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_G3;				/*Size= 6 QWords*/

typedef struct
{
	GS_R_PRIM		prim;		// Primitive Attributes
	GS_R_RGBAQ		rgbaq0;		// Vertex 0 Color
	GS_R_ST			st0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// Vertex 1 Color
	GS_R_ST			st1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_RGBAQ		rgbaq2;		// Vertex 2 Color
	GS_R_ST			st2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_GT3ST_P;			/*Size= 10 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// Vertex 0 Color
	GS_R_ST			st0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate
	GS_R_RGBAQ		rgbaq1;		// Vertex 1 Color
	GS_R_ST			st1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate
	GS_R_RGBAQ		rgbaq2;		// Vertex 2 Color
	GS_R_ST			st2;		// vertex 2 texture coordinate
	GS_R_XYZ		xyz2;		// vertex 2 coordinate
}GS_TRIANGLE_GT3ST;			/*Size= 9 QWords*/

/*A Untextured Sprite*/
typedef struct
{
	GS_R_PRIM		prim;		// Primitive Attributes
	GS_R_RGBAQ		rgbaq0;		// sprite Color
	GS_R_XYZ		xyz0;		// vertex 1 coordinate(upper left)
	GS_R_XYZ		xyz1;		// vertex 2 coordinate(lower right)
}GS_SPRITE_F4_P;			/*Size= 4 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// Vertex 1 Color
	GS_R_XYZ		xyz0;		// vertex 1 coordinate(upper left)
	GS_R_XYZ		xyz1;		// vertex 2 coordinate(lower right)
}GS_SPRITE_F4;				/*Size= 3 QWords*/

/*A Textured Sprite*/
typedef struct
{
	GS_R_PRIM		prim;		// Primitive Attributes
	GS_R_RGBAQ		rgbaq0;		// sprite Color
	GS_R_UV			uv0;		// vertex 0 Texture Coordinate(upper left)
	GS_R_XYZ		xyz0;		// vertex 0 coordinate(upper left)
	GS_R_UV			uv1;		// vertex 1 Texture Coordinate(lower right)
	GS_R_XYZ		xyz1;		// vertex 1 coordinate(lower right)
}GS_SPRITE_FT4_P;			/*Size= 6 QWords*/

typedef struct
{
	GS_R_RGBAQ		rgbaq0;		// sprite Color
	GS_R_UV			uv0;		// vertex 0 texture coordinate
	GS_R_XYZ		xyz0;		// vertex 0 coordinate(upper left)
	GS_R_UV			uv1;		// vertex 1 texture coordinate
	GS_R_XYZ		xyz1;		// vertex 1 coordinate(lower right)
}GS_SPRITE_FT4;				/*Size= 5 QWords*/

#define	GS_PACKET_DATA_QWORD_MAX	32000

typedef struct
{
	GS_GIF_DMACHAIN_TAG	tag;
	QWORD			data[GS_PACKET_DATA_QWORD_MAX];
}GS_GIF_PACKET	__attribute__ ((aligned(16)));/* Aligned, 128bits*/

typedef struct
{
	unsigned int	packet_count;
	unsigned int	packet_offset;
	unsigned int	qword_offset;
	GS_GIF_PACKET	*packets;
}GS_PACKET_TABLE;

typedef struct
{
	unsigned short	x;			// X Offset in Vram Address
	unsigned short	y;			// X Offset in Vram Address
	unsigned short	width;		// Height of image
	unsigned short	height;		// Width  of image
	unsigned short	vram_addr;	// Address in frame buffer
	unsigned char	vram_width;	// Width of vram (1=64)
	unsigned char	psm;	// Pixel Mode / PSM
}GS_IMAGE;

/*
typedef struct
{
	unsigned char	 img_psm;
	unsigned short	 img_width;
	unsigned short	 img_height;
	unsigned int	*img_addr;

	unsigned char	 clt_psm;
	unsigned short	 clt_csm;
	unsigned short	 clt_width;
	unsigned short	 clt_height;
	unsigned int	*clt_addr;
}GS_EE_IMAGE;	*/

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

/*----------------------------------------------------
--	COMMONLY UDED, LOW LEVEL FUNTIONS				--
--													--
--													--
------------------------------------------------------*/

/* These Use Gif-Dma To Transfer*/
int GsSetXYOffset1(unsigned short x, unsigned short y);
int GsSetXYOffset2(unsigned short x, unsigned short y);
int GsSetScissor1(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y);
int GsSetScissor2(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y);
int GsSetFrame1(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask);
int GsSetFrame2(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char psm, unsigned int draw_mask);
int GsTextureFlush(void);
int GsSetPixelTest1(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method);
int GsSetPixelTest2(unsigned char enable_alpha_test, unsigned char alpha_test_method, unsigned char alpha_reference, unsigned char alpha_fail_method, unsigned char enable_dest_alpha_test, unsigned char dest_alpha_test_mode, unsigned char enable_zbuff_test, unsigned char alpha_zbuff_method);
int GsSelectTexure1(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset);
int GsSelectTexure2(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset);
void GsSetFogColor(unsigned char r, unsigned char g, unsigned char b);
void GsEnableColorClamp(unsigned short enable);

/*----------------------------------------------------
--	NORMAL FUNTIONS									--
--													--
--													--
------------------------------------------------------*/

GsGParam_t *GsGetGParam(void);
void GsResetGraph(short int mode, short int interlace, short int omode, short int ffmode);
void GsResetPath(void);
void GsSetCRTCSettings(unsigned long settings, unsigned char alpha_value);

/* Initialise structs with defaults Based On Input*/
void GsSetDefaultDrawEnv(GS_DRAWENV *drawenv, unsigned short int psm, unsigned short int w, unsigned short int h);
void GsSetDefaultDrawEnvAddress(GS_DRAWENV *drawenv, unsigned short vram_addr);
void GsSetDefaultDisplayEnv(GS_DISPENV *dispenv, unsigned short int psm, unsigned short int w, unsigned short int h, unsigned short int dx, unsigned short int dy);
void GsSetDefaultDisplayEnvAddress(GS_DISPENV *dispenv, unsigned short vram_addr);
void GsSetDefaultZBufferEnv(GS_ZENV *zenv, unsigned char update_mask);
void GsSetDefaultZBufferEnvAddress(GS_ZENV *zenv, unsigned short vram_addr, unsigned char psm);

/* Execute struct's data (Environments)*/
void GsPutDrawEnv1(GS_DRAWENV		*drawenv);
void GsPutDrawEnv2(GS_DRAWENV		*drawenv);
void GsPutDisplayEnv1(GS_DISPENV	*dispenv);
void GsPutDisplayEnv2(GS_DISPENV	*dispenv);
void GsPutZBufferEnv1(GS_ZENV *zenv);
void GsPutZBufferEnv2(GS_ZENV *zenv);
void GsClearDrawEnv1(GS_DRAWENV	*drawenv);	// clear draw buffer with GS_DRAWENV->bg_color color (contex 1)
void GsClearDrawEnv2(GS_DRAWENV	*drawenv);	// clear draw buffer with GS_DRAWENV->bg_color color (contex 2)

/* Gif packet execution*/
QWORD *GsGifPacketsAlloc(GS_PACKET_TABLE *table, unsigned int  num_qwords);
void GsGifPacketsClear(GS_PACKET_TABLE *table);
int GsGifPacketsExecute(GS_PACKET_TABLE	*table, unsigned short wait);

/* Texture/Image Funtions*/
int GsLoadImage(const void *source_addr, GS_IMAGE *dest);

/**/
void GsOverridePrimAttributes(char override, char iip, char tme, char fge, char abe, char aa1, char fst, char ctxt, char fix);
void GsEnableDithering(unsigned char enable, int mode);
void GsEnableAlphaTransparency1(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short fail_method);
void GsEnableAlphaTransparency2(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short fail_method);
void GsEnableZbuffer1(unsigned short enable,unsigned short test_method);
void GsEnableZbuffer2(unsigned short enable,unsigned short test_method);
void GsEnableAlphaBlending1(unsigned short enable);
void GsEnableAlphaBlending2(unsigned short enable);

/**/
void GsDrawSync(int mode);
void GsHSync(int mode);
void GsVSync(int mode);

/* Vram Allocation*/
int    GsVramAllocFrameBuffer(short w, short h, short psm);
int    GsVramAllocTextureBuffer(short w, short h, short psm);
void GsVramFreeAllTextureBuffer(void);  //free texture buffer without freeing frame buffer
void GsVramFreeAll(void);

/* Just used for buffer swapping*/
int    GsDbGetDrawBuffer(void);
int    GsDbGetDisplayBuffer(void);
void GsDbSwapBuffer(void);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /*_LIBGS_H_*/
