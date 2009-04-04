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

#ifndef _LIBGS_H_
#define _LIBGS_H_







#ifndef QWORD
typedef struct 
{
	unsigned long lo, hi;
}QWORD __attribute__((aligned(128)));
#endif





//type of primitives
#define GS_PRIM_POINT		0
#define GS_PRIM_LINE		1
#define GS_PRIM_LINE_STRIP	2
#define GS_PRIM_TRI			3
#define GS_PRIM_TRI_STRIP	4
#define GS_PRIM_TRI_FAN		5
#define GS_PRIM_SPRITE		6
#define GS_PRIM_PS2DEV		7


// regular pixmodes
#define GS_PIXMODE_32		 0
#define GS_PIXMODE_24		 1
#define GS_PIXMODE_16		 2
#define GS_PIXMODE_16S		10

// clut pixmodes
#define GS_CLUT_32			 0
#define GS_CLUT_16			 2
#define GS_CLUT_16S			10

// texture/image pixmodes
#define GS_TEX_32			 0
#define GS_TEX_24			 1
#define GS_TEX_16			 2
#define GS_TEX_16S			10
#define GS_TEX_8			19
#define GS_TEX_4			20
#define GS_TEX_8H			27
#define GS_TEX_4HL			36
#define GS_TEX_4HH			44

// Z-Buffer modes
#define GS_ZBUFF_32			48
#define GS_ZBUFF_24			49
#define GS_ZBUFF_16			50
#define GS_ZBUFF_16S		58

// Alpha Methods
#define GS_ALPHA_NEVER		0
#define GS_ALPHA_ALWAYS		1
#define GS_ALPHA_LESS		2
#define GS_ALPHA_LEQUAL		3
#define GS_ALPHA_EQUAL		4
#define GS_ALPHA_GEQUAL		5
#define GS_ALPHA_GREATER	6
#define GS_ALPHA_NOTEQUAL	7

// Alpha update Methods
#define GS_ALPHA_NO_UPDATE	0	//standard
#define GS_ALPHA_FB_ONLY	1
#define GS_ALPHA_ZB_ONLY	2
#define GS_ALPHA_RGB_ONLY	3


// Zbuffer test modes
#define GS_ZBUFF_NEVER		0
#define GS_ZBUFF_ALWAYS		1
#define GS_ZBUFF_GEQUAL		2
#define GS_ZBUFF_GREATER	3


//Texture Details
#define GS_TEX_CALC			0			//use near/far formula
#define GS_TEX_FIXED		1			//fixed value (use K value)

#define GS_TEX_MIPMAP0		0			//no mipmap
#define GS_TEX_MIPMAP1		1			//1 mipmap
#define GS_TEX_MIPMAP2		2			//2 mipmaps
#define GS_TEX_MIPMAP3		3			//...
#define GS_TEX_MIPMAP4		4			//...
#define GS_TEX_MIPMAP5		5			//...
#define GS_TEX_MIPMAP6		6			//...

#define GS_TEX_NEAREST					0	//UnFiltered
#define GS_TEX_LINEAR					1	//Filtered
#define GS_TEX_NEAREST_MIPMAP_NEAREST	2	//
#define GS_TEX_NEAREST_MIPMAP_LINEAR	3	//
#define GS_TEX_LINEAR_MIPMAP_NEAREST	4	//
#define GS_TEX_LINEAR_MIPMAP_LINEAR		5	//

#define GS_TEX_MIPMAP_DEFINE			0	//use values in MIPTBP1
#define GS_TEX_MIPMAP_AUTO				1	//auto calculate mipmap address
	
//Texture Funtion (used in TEX0->tcc)
#define GS_TEX_MODULATE					0	// brighten texture based on Pixel's Alpha 
#define GS_TEX_DECAL					1	// keep texture as is
#define GS_TEX_HIGHLIHGT1				2	// used when highlighting translucent polygons
#define GS_TEX_HIGHLIHGT2				3	// used when highlighting opaque	  polygons




/*GS Privileged Regs*/
#define gs_p_pmode			0x12000000	// Setup CRT Controller
#define gs_p_smode1			0x12000010	// ??
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
//#define gs_g_rgbaq2	0x11	// Vertex color
//#define gs_g_st2		0x12	// ...
//#define gs_g_uv2		0x13	// ...
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






/*----------------------------------------------------
--	GS Privileged Reg STRUCTs										--
--													--
--													--
----------------------------------------------------*/


/*PMODE*/
typedef struct {
	unsigned enable_rc1	:1;	// Enable ReadCircuit 1
	unsigned enable_rc2	:1;	// Enable ReadCircuit 2
	unsigned crt_out	:3;	// CRT output switching(always 1)
	unsigned mmod		:1;	// Value to use for alpha blend(0=value in 'RC1',1=value in 'blend_value')
	unsigned amod		:1;	// ReadCircuit to output alpha to (0=RC1, 1=RC2)
	unsigned blend_style:1;	// Blend Method(0=blend with RC2, 0=blend with BG)
	unsigned blend_value:8;	// Alpha Blend Value (0-255)
	unsigned nfld		:1;	// Output to NFIELD
	unsigned pad1		:15;// Pad with zeros
	unsigned exvwins	:10;// ??
	unsigned exvwine	:10;// ??
	unsigned exsyncmd	:1;	// ??
	unsigned pad2		:11;// Pad with zeros
}GS_PMODE;



typedef struct {
	
}GS_SMODE1;





/*SMODE2*/
/*
typedef struct {
	unsigned RC   : 8;	// PLL Reference Divider
	unsigned LC   :13;	// PLL Loop Divider
	unsigned T1248: 4;	// PLL Output Divider
	unsigned SCLK : 2;	// VESA mode Clock
	unsigned CMOD : 2;	// Display mode
	unsigned INT  : 1;	// Interlace mode
	unsigned FFMD : 1;	//
	unsigned EX   : 1;	//
	unsigned VHH  : 1;	// Half H VBLANK
	unsigned VHP  : 1;	// Half H Pulse
	unsigned PRST : 1;	// PLL reset
	unsigned SINT : 1;	//
	unsigned SHCL : 1;	//
	unsigned PCK2 : 1;	//
	unsigned SPML : 4;	//
	unsigned p0   : 1;	//
	unsigned DPMS : 2;	// VESA DPMS mode
	unsigned PHS  : 1;	// HSync output
	unsigned PVS  : 1;	// VSync output
	unsigned PEHS : 1;	//
	unsigned PEVS : 1;	//
	unsigned RFSH : 6;	// Refresh rate
	unsigned p1   : 9;	//
}GS_SMODE2;
*/


typedef struct {
	unsigned interlace   :1;
	unsigned field_frame :1;
	unsigned vesta_dpms  :2;
	unsigned long pad2	 :60;
}GS_SMODE22;



/*DISPFB*/
typedef struct {
	unsigned address	:9;	// Base pointer in VRam
	unsigned width		:6;	// Buffer width in VRam
	unsigned pix_mode	:5;	// Pixel store mode
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

/*
typedef struct {
	unsigned EN1     : 1;	//
	unsigned EN2     : 1;	//
	unsigned CRTMD   : 3;	//
	unsigned MMOD    : 1;	//
	unsigned AMOD    : 1;	//
	unsigned SLBG    : 1;	//
	unsigned ALP     : 8;	//
	unsigned NFLD    : 1;	//
	unsigned p0      :15;	//
	unsigned EXVWINS :10;	//
	unsigned EXVWINE :10;	//
	unsigned EXSYNCMD: 1;	//
	unsigned p1      :11;	//
} GS_EXTBUF;
*/

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
	unsigned char	r;	// Background Color Red
	unsigned char	g;	// Background Color Green
	unsigned char	b;	// Background Color Blue
	unsigned char	pada;
	float			padq;
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
	unsigned direction		:1;	// Transmission direction(0=host->local, 1=host<-local)
	unsigned p0				:31;// Pad with zeros
    unsigned int p1;			// Pad with more zeros
}GS_BUSDIR;


/*SIGLBLID*/
/*typedef struct {
	unsigned int id;	// SIGNAL register id
	unsigned int p0;
}GS_SIGLBLID;
*/












/*GIFTAG*/
typedef struct {
	unsigned long nloop	:15;
	unsigned long eop	:1;
	unsigned long pad1	:16;
	unsigned long id	:14;
	unsigned long pre	:1;
	unsigned long prim	:11;
	unsigned long flg	:2;
	unsigned long nreg	:4;
	unsigned long reg	:64;
}GS_PRIMTAG;



#define gs_setPRIMTAG(p, _nloop,_eop,_id,_pre,_prim,_flg,_nreg,_reg)\
	(p)->nloop	= _nloop,				\
	(p)->eop	= _eop,					\
	(p)->id		= _id,					\
	(p)->pre	= _pre,					\
	(p)->prim	= _prim,				\
	(p)->flg	= _flg,					\
	(p)->nreg	= _nreg,				\
	(p)->reg	= _reg	



















/*--------------------------------------------------------
--	GENERAL PURPOSE REG STRUCTS	'WITHOUT' 64 BIT REG'	--
--														--
--	Remember that these structs have a size of 64 bits	--
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
	unsigned long fb_addr     :14;
	unsigned long fb_width    :6;
	unsigned long pix_mode    :6;
	unsigned long tex_width   :4;
	unsigned long tex_height  :4;
	unsigned long col_comp	  :1;
	unsigned long tfx		  :2;
	unsigned long clutb_addr  :14;
	unsigned long clut_pixmode:4;
	unsigned long clut_smode  :1;
	unsigned long clut_offset :5;
	unsigned long cld		  :3;

}GS_TEX0;


typedef struct {
	unsigned long wms:2;
	unsigned long wmt:2;
	unsigned long minu:10;
	unsigned long maxu:10;
	unsigned long minv:10;
	unsigned long maxv:10;
	unsigned long pad0:20;
}GS_CLAMP;


typedef struct {
		unsigned long pad1	:56;
		unsigned char f;
}GS_FOG;


typedef struct {
	unsigned long lcm	:1;
	unsigned long pad1	:1;
	unsigned long mxl	:3;
	unsigned long mmag	:1;
	unsigned long mmin	:3;
	unsigned long mtba	:1;
	unsigned long pad2	:9;
	unsigned long l		:2;
	unsigned long pad3	:11;
	unsigned long k		:12;
	unsigned long pad4	:20;
}GS_TEX1;


typedef struct {
	unsigned long pad1:20;
	unsigned long psm:6;
	unsigned long pad2:11;
	unsigned long cbp:14;
	unsigned long cpsm:4;
	unsigned long csm:1;
	unsigned long csa:5;
	unsigned long cld:3;
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
	unsigned long cbw:6;
	unsigned long cou:6;
	unsigned long cov:10;
	unsigned long pad0:42;
}GS_TEXCLUT;


typedef struct {
	unsigned long msk:2;
	unsigned long pad0:62;
}GS_SCANMSK;


typedef struct {
	unsigned long tbp1:14;
	unsigned long tbw1:6;
	unsigned long tbp2:14;
	unsigned long tbw2:6;
	unsigned long tbp3:14;
	unsigned long tbw3:6;
	unsigned long pad1:4;
} GS_MIPTBP1;


typedef struct {
	unsigned long tbp4:14;
	unsigned long tbw4:6;
	unsigned long tbp5:14;
	unsigned long tbw5:6;
	unsigned long tbp6:14;
	unsigned long tbw6:6;
	unsigned long pad0:4;
}GS_MIPTBP2;


typedef struct {
	unsigned long ta0	: 8;
	unsigned long pad1	: 7;
	unsigned long aem	: 1;
	unsigned long pad2	:16;
	unsigned long ta1	: 8;
	unsigned long pad3	:24;
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
	unsigned long a:2;
	unsigned long b:2;
	unsigned long c:2;
	unsigned long d:2;
	unsigned long pad1:24;
	unsigned long fix:8;
	unsigned long pad2:24;
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
	unsigned long ATE	:1;
	unsigned long ATST	:3;
	unsigned long AREF	:8;
	unsigned long AFAIL	:2;
	unsigned long DATE	:1;
	unsigned long DATM	:1;
	unsigned long ZTE	:1;
	unsigned long ZTST	:2;
	unsigned long pad1	:45;
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
	unsigned long pix_mode	:6;
	unsigned long pad3		:2;
	unsigned long draw_mask	:32;
} GS_FRAME;


typedef struct {
	unsigned long fb_addr	:9;
	unsigned long pad1			:15;
	unsigned long pix_mode		:4;
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












/*--------------------------------------------------------
--	GENERAL PURPOSE REG STRUCTS	'WITH' 64 BIT REG'		--
--														--
--	Remember that these structs have a size of 128 bits	--
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
	unsigned long reg;
}GS_R_PRIM;


typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	float		  q;
	unsigned long reg;
}GS_R_RGBAQ;


typedef struct {
	float			s;
	float			t;
	unsigned long	reg;
}GS_R_ST;


typedef struct {
	unsigned long u		:14;
	unsigned long pad1	:2;
	unsigned long v		:14;
	unsigned long pad2	:34;
	unsigned long reg;
}GS_R_UV;


typedef struct {
	unsigned short	x;
	unsigned short	y;
	unsigned int	z:24;
	unsigned char	f;
	unsigned long	reg;
}GS_R_XYZF;


typedef struct {
	unsigned short	x;
	unsigned short	y;
	unsigned int	z;
	unsigned long	reg;
}GS_R_XYZ;


typedef struct {
	unsigned long fb_addr     :14;
	unsigned long fb_width    :6;
	unsigned long pix_mode    :6;
	unsigned long tex_width   :4;
	unsigned long tex_height  :4;
	unsigned long col_comp	  :1;
	unsigned long tfx		  :2;
	unsigned long clutb_addr  :14;
	unsigned long clut_pixmode:4;
	unsigned long clut_smode  :1;
	unsigned long clut_offset :5;
	unsigned long cld		  :3;
	unsigned long reg;
}GS_R_TEX0;


typedef struct {
	unsigned long wms:2;
	unsigned long wmt:2;
	unsigned long minu:10;
	unsigned long maxu:10;
	unsigned long minv:10;
	unsigned long maxv:10;
	unsigned long pad44:20;
	unsigned long reg;
}GS_R_CLAMP;


typedef struct {
		unsigned long pad1	:56;
		unsigned char f;
		unsigned long reg;
}GS_R_FOG;


typedef struct {
	unsigned long lcm	:1;
	unsigned long pad1	:1;
	unsigned long mxl	:3;
	unsigned long mmag	:1;
	unsigned long mmin	:3;
	unsigned long mtba	:1;
	unsigned long pad2	:9;
	unsigned long l		:2;
	unsigned long pad3	:11;
	unsigned long k		:12;
	unsigned long pad4	:20;
	unsigned long reg;
}GS_R_TEX1;


typedef struct {
	unsigned long pad00:20;
	unsigned long psm:6;
	unsigned long pad26:11;
	unsigned long cbp:14;
	unsigned long cpsm:4;
	unsigned long csm:1;
	unsigned long csa:5;
	unsigned long cld:3;
	unsigned long reg;
}GS_R_TEX2;


typedef struct {
	unsigned long  offset_x	:16;
	unsigned short pad1;
	unsigned long  offset_y	:16;
	unsigned short pad2;
	unsigned long  reg;
} GS_R_XYOFFSET;


typedef struct {
	unsigned long control:1;
	unsigned long pad1	:63;
	unsigned long reg;
}GS_R_PRMODECONT;


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
	unsigned long reg;
}GS_R_PRMODE;


typedef struct {
	unsigned long cbw:6;
	unsigned long cou:6;
	unsigned long cov:10;
	unsigned long pad0:42;
	unsigned long reg;
}GS_R_TEXCLUT;


typedef struct {
	unsigned long msk:2;
	unsigned long pad0:62;
	unsigned long reg;
}GS_R_SCANMSK;


typedef struct {
	unsigned long tbp1:14;
	unsigned long tbw1:6;
	unsigned long tbp2:14;
	unsigned long tbw2:6;
	unsigned long tbp3:14;
	unsigned long tbw3:6;
	unsigned long pad0:4;
	unsigned long reg;
} GS_R_MIPTBP1;


typedef struct {
	unsigned long tbp4:14;
	unsigned long tbw4:6;
	unsigned long tbp5:14;
	unsigned long tbw5:6;
	unsigned long tbp6:14;
	unsigned long tbw6:6;
	unsigned long pad0:4;
	unsigned long reg;
}GS_R_MIPTBP2;


typedef struct {
	unsigned long ta0	: 8;
	unsigned long pad0	: 7;
	unsigned long aem	: 1;
	unsigned long pad1	:16;
	unsigned long ta1	: 8;
	unsigned long pad2	:24;
	unsigned long reg;
}GS_R_TEXA;


typedef struct {
	unsigned long r		:8;
	unsigned long g		:8;
	unsigned long b		:8;
	unsigned long pad1	:40;
	unsigned long reg;
}GS_R_FOGCOLOR;


typedef struct {
	unsigned long pad1;			// Pad With Zeros
	unsigned long reg;			
} GS_R_TEXFLUSH;


typedef struct {
	unsigned long clip_x0 :11;
	unsigned long pad1    :5;
	unsigned long clip_x1 :11;
	unsigned long pad2    :5;
	unsigned long clip_y0 :11;
	unsigned long pad3    :5;
	unsigned long clip_y1 :11;
	unsigned long pad4    :5;
	unsigned long reg;
}GS_R_SCISSOR;


typedef struct {
	unsigned long a:2;
	unsigned long b:2;
	unsigned long c:2;
	unsigned long d:2;
	unsigned long pad0:24;
	unsigned long fix:8;
	unsigned long pad1:24;
	unsigned long reg;
}GS_R_ALPHA;


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
	unsigned long reg;
} GS_R_DIMX;


typedef struct {
	unsigned long enable:1;
	unsigned long pad01:63;
	unsigned long reg;
} GS_R_DTHE;


typedef struct {
	unsigned long clamp:1;
	unsigned long pad01:63;
	unsigned long reg;
}GS_R_COLCLAMP;


typedef struct {
	unsigned long ATE	:1;
	unsigned long ATST	:3;
	unsigned long AREF	:8;
	unsigned long AFAIL	:2;
	unsigned long DATE	:1;
	unsigned long DATM	:1;
	unsigned long ZTE	:1;
	unsigned long ZTST	:2;
	unsigned long pad1	:45;
	unsigned long reg;
} GS_R_TEST;


typedef struct {
	unsigned long enable:1;
	unsigned long pad0:63;
	unsigned long reg;
}GS_R_PABE;


typedef struct {
	unsigned long alpha:1;
	unsigned long pad0:63;
	unsigned long reg;
}GS_R_FBA;


typedef struct {
	unsigned long fb_addr	:9;
	unsigned long pad1		:7;
	unsigned long fb_width	:6;
	unsigned long pad2		:2;
	unsigned long pix_mode	:6;
	unsigned long pad3		:2;
	unsigned long draw_mask	:32;
	unsigned long reg;
} GS_R_FRAME;


typedef struct {
	unsigned long fb_addr	:9;
	unsigned long pad1			:15;
	unsigned long pix_mode		:4;
	unsigned long pad2			:4;
	unsigned long update_mask	:1;
	unsigned long pad3			:31;
	unsigned long reg;
}GS_R_ZBUF;


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
	unsigned long reg;
}GS_R_BITBLTBUF;


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
	unsigned long reg;
}GS_R_TRXPOS;


typedef struct {
	unsigned long trans_w	:12;
	unsigned long pad1		:20;
	unsigned long trans_h	:12;
	unsigned long pad2		:20;
	unsigned long reg;
}GS_R_TRXREG;


typedef struct {
	unsigned long trans_dir	:2;
	unsigned long pad1		:62;
	unsigned long reg;
}GS_R_TRXDIR;



typedef struct {
	unsigned long data;
	unsigned long reg;
}GS_R_HWREG;


typedef struct {
	unsigned int signal_id;
	unsigned int update_mask;
	unsigned long reg;
}GS_R_SIGNAL;


typedef struct {
	unsigned long pad0;
	unsigned long reg;
}GS_R_FINISH;


typedef struct {
	unsigned int label_id;
	unsigned int update_mask;
	unsigned long reg;
}GS_R_LABEL;








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


#define gs_setTEX0_1(p, _fb_addr,_fb_width,_pix_mode,_tex_width,_tex_height,_col_comp,_tfx,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld)\
	(p)->fb_addr	= _fb_addr,			\
	(p)->fb_width	= _fb_width,		\
	(p)->pix_mode	= _pix_mode,		\
	(p)->tex_width	= twh4(_tex_width),	\
	(p)->tex_height	= twh4(_tex_height),\
	(p)->col_comp	= _col_comp,		\
	(p)->tfx		= _tfx,				\
	(p)->clutb_addr	= _clutb_addr,		\
	(p)->clut_pixmode=_clut_pixmode,	\
	(p)->clut_smode	= _clut_smode,		\
	(p)->clut_offset= _clut_offset,		\
	(p)->cld		= _cld				
				
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

/*
#define gs_setMIPTBP1_1(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
	(p)->tbp1	= _tbp1,		\
	(p)->tbw1	= _tbw1,		\
	(p)->tbp2	= _tbp2,		\		
	(p)->tbw2	= _tbw2,		\
	(p)->tbp3	= _tbp3,		\
	(p)->tbw3	= _tbw3	

#define gs_setMIPTBP1_2			gs_setMIPTBP1_1
*/	


#define gs_setMIPTBP2_1(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6)	\
	(p)->tbp4	= _tbp4,		\
	(p)->tbw4	= _tbw4,		\
	(p)->tbp5	= _tbp5,		\
	(p)->tbw5	= _tbw5,		\
	(p)->tbp6	= _tbp6,		\
	(p)->tbw6	= _tbw6	

#define gs_setMIPTBP2_2			gs_setMIPTBP2_1



#define gs_setTEXA(p, _ta0,_aem,_ta1)	\
	(p)->ta0	= _ta0,						\
	(p)->aem	= _aem,						\
	(p)->ta1	= _ta1	


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



#define gs_setALPHA_1(p, _a,_b,_c,_d,_fix)	\
	(p)->a		= _a,						\
	(p)->b		= _b,						\
	(p)->c		= _c,						\
	(p)->d		= _d,						\
	(p)->fix	= _fix	

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


#define gs_setTEST_1(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST)	\
	(p)->ATE		= _ATE,					\
	(p)->ATST		= _ATST,				\
	(p)->AREF		= _AREF,				\
	(p)->AFAIL		= _AFAIL,				\
	(p)->DATE		= _DATE,				\
	(p)->DATM		= _DATM,				\
	(p)->ZTE		= _ZTE,					\
	(p)->ZTST		= _ZTST						

#define gs_setTEST_2			gs_setTEST_1


	
#define gs_setPABE(p, _enable)	\
	(p)->enable	= _enable


#define gs_setFBA(p, _alpha)	\
	(p)->alpha	= _alpha


#define gs_setFRAME_1(p, _fb_addr,_fb_width,_pix_mode,_draw_mask)	\
	(p)->fb_addr		= _fb_addr,				\
	(p)->fb_width		= _fb_width,			\
	(p)->pix_mode		= _pix_mode,			\
	(p)->draw_mask		= _draw_mask

#define gs_setFRAME_2				gs_setFRAME_1	



#define gs_setZBUF_1(p, _fb_addr,_pix_mode,_update_mask)	\
	(p)->fb_addr		= _fb_addr,						\
	(p)->pix_mode		= _pix_mode,					\
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

#define gs_setFINISH(p)


#define gs_setLABEL(p, _label_id,_update_mask)	\
	(p)->label_id	= _label_id,				\
	(p)->update_mask= _update_mask
	
	


















/*--------------------------------------------------------
--	'SET' GENERAL PURPOSE REG STRUCTs & REGs			--
--														--
--														--
----------------------------------------------------------*/



#define gs_setR_PRIM(p, _prim_type,_iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)\
						gs_setPRIM(p, _prim_type,_iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix),\
						p->reg = gs_g_prim
	

//RGBAQ
#define gs_setR_RGBAQ(p, _r,_g,_b,_a,_q)	\
						gs_setRGBAQ(p, _r,_g,_b,_a,_q),\
						(p)->reg = gs_g_rgbaq



//ST
#define gs_setR_ST(p, _s,_t)	\
						gs_setST(p, _s,_t),\
						(p)->reg = gs_g_st

//UV
#define gs_setR_UV(p, _u,_v)	\
						gs_setUV(p, _u,_v),\
						(p)->reg = gs_g_uv

//XYZF2
#define gs_setR_XYZF2(p, _x,_y,_z,_f)	\
						gs_setXYZF2(p, _x,_y,_z,_f),\
						(p)->reg = gs_g_xyzf2


//XYZF3
#define gs_setR_XYZF3(p, _x,_y,_z,_f)	\
						gs_setR_XYZF3(p, _x,_y,_z,_f),\
						(p)->reg = gs_g_xyzf3
		

#define gs_setR_XYZ2(p, _x,_y,_z)	\
						gs_setXYZ2(p, _x,_y,_z),\
						(p)->reg = 	gs_g_xyz2		
				
#define gs_setR_XYZ3(p, _x,_y,_z)	\
						gs_setXYZ3(p, _x,_y,_z),\
						(p)->reg = 	gs_g_xyz3


#define gs_setR_TEX0_1(p, _fb_addr,_fb_width,_pix_mode,_tex_width,_tex_height,_col_comp,_tfx,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld)\
							gs_setTEX0_1(p, _fb_addr,_fb_width,_pix_mode,_tex_width,_tex_height,_col_comp,_tfx,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld),\
							(p)->reg = 	gs_g_tex0_1
	
			
#define gs_setR_TEX0_2(p, _fb_addr,_fb_width,_pix_mode,_tex_width,_tex_height,_col_comp,_tfx,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld)\
							gs_setTEX0_2(p, _fb_addr,_fb_width,_pix_mode,_tex_width,_tex_height,_col_comp,_tfx,_clutb_addr,_clut_pixmode,_clut_smode,_clut_offset,_cld),\
							(p)->reg = 	gs_g_tex0_2



#define gs_setR_CLAMP_1(p, wms,wmt,minu,maxu,minv,maxv)	\
							gs_setCLAMP_1(p, wms,wmt,minu,maxu,minv,maxv),\
							(p)->reg = 	gs_g_clamp_1

	
#define gs_setR_CLAMP_2(p, wms,wmt,minu,maxu,minv,maxv)	\
							gs_setCLAMP_2(p, wms,wmt,minu,maxu,minv,maxv),\
							(p)->reg = 	gs_g_clamp_2
	


#define gs_setR_FOG(p, _f)	\
							gs_setFOG(p, _f),\
							(p)->reg = 	gs_g_fog


#define gs_setR_TEX1_1(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k)	\
							gs_setTEX1_1(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k),\
							(p)->reg = 	gs_g_tex1_1

#define gs_setR_TEX1_2(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k)	\
							gs_setTEX1_2(p, _lcm,_mxl,_mmag,_mmin,_mtba,_l,_k),\
							(p)->reg = 	gs_g_tex1_2



#define gs_setR_TEX2_1(p, _psm,_cbp,_cpsm,_csm,_csa,_cld)	\
							gs_setTEX2_1(p, _psm,_cbp,_cpsm,_csm,_csa,_cld),\
							(p)->reg = 	gs_g_tex2_1

#define gs_setR_TEX2_2(p, _psm,_cbp,_cpsm,_csm,_csa,_cld)	\
							gs_setTEX2_2(p, _psm,_cbp,_cpsm,_csm,_csa,_cld),\
							(p)->reg = 	gs_g_tex2_2

	

#define gs_setR_XYOFFSET_1(p, _offset_x,_offset_y)	\
							gs_set_XYOFFSET_1(p, _offset_x,_offset_y),\
							(p)->reg = 	gs_g_xyoffset_1

#define gs_setR_XYOFFSET_2(p, _offset_x,_offset_y)	\
							gs_set_XYOFFSET_2(p, _offset_x,_offset_y),\
							(p)->reg = 	gs_g_xyoffset_2


#define gs_setR_PRMODECONT(p, _control)	\
							gs_set_PRMODECONT(p, _control),\
							(p)->reg = 	gs_g_prmodecont



#define gs_setR_PRMODE(p, _iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix)	\
							gs_setPRMODE(p, _iip,_tme,_fge,_abe,_aa1,_fst,_ctxt,_fix),\
							(p)->reg = 	gs_g_prmode
	

#define gs_setR_TEXCLUT(p, _cbw,_cou,_cov)	\
							gs_setTEXCLUT(p, _cbw,_cou,_cov),\
							(p)->reg = 	gs_g_texclut

#define gs_setR_SCANMSK(p, _msk)	\
							gs_setSCANMSK(p, _msk),\
							(p)->reg = 	gs_g_scanmsk


#define gs_setR_MIPTBP1_1(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
							gs_setMIPTBP1_1(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3),\
							(p)->reg = 	gs_g_miptbp1_1

#define gs_setR_MIPTBP1_2(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3)	\
							gs_setMIPTBP1_2(p, _tbp1,_tbw1,_tbp2,_tbw2,_tbp3,_tbw3),\
							(p)->reg = 	gs_g_miptbp1_2


#define gs_setR_MIPTBP2_1(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6)	\
							gs_setR_MIPTBP2_1(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6),\
							(p)->reg = 	gs_g_miptbp2_1

#define gs_setR_MIPTBP2_2(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6)	\
							gs_setMIPTBP2_2(p, _tbp4,_tbw4,_tbp5,_tbw5,_tbp6,_tbw6),\
							(p)->reg = 	gs_g_miptbp2_2


#define gs_setR_TEXA(p, _ta0,_aem,_ta1)	\
							gs_setTEXA(p, _ta0,_aem,_ta1),\
							(p)->reg = 	gs_g_texa

#define gs_setR_FOGCOLOR(p, _r,_g,_b)		\
							gs_setFOGCOLOR(p, _r,_g,_b),\
							(p)->reg = 	gs_g_fogcol

#define gs_setR_TEXFLUSH(p)\
							(p)->reg = 	gs_g_texflush
	

#define gs_setR_SCISSOR_1(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1)	\
							gs_setSCISSOR_1(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1),\
							(p)->reg = 	gs_g_scissor_1

#define gs_setR_SCISSOR_2(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1)	\
							gs_setSCISSOR_2(p, _clip_x0,_clip_x1,_clip_y0,_clip_y1),\
							(p)->reg = 	gs_g_scissor_2



#define gs_setR_ALPHA_1(p, _a,_b,_c,_d,_fix)	\
							gs_setALPHA_1(p, _a,_b,_c,_d,_fix),\
							(p)->reg = 	gs_g_alpha_1

#define gs_setR_ALPHA_2(p, _a,_b,_c,_d,_fix)	\
							gs_setALPHA_2(p, _a,_b,_c,_d,_fix),\
							(p)->reg = 	gs_g_alpha_2



#define gs_setR_DIMX(p, _dimx00,_dimx01,_dimx02,_dimx03,_dimx10,_dimx11,_dimx12,_dimx13,_dimx20,_dimx21,_dimx22,_dimx23,_dimx30,_dimx31,_dimx32,_dimx33)	\
							gs_setDIMX(p, _dimx00,_dimx01,_dimx02,_dimx03,_dimx10,_dimx11,_dimx12,_dimx13,_dimx20,_dimx21,_dimx22,_dimx23,_dimx30,_dimx31,_dimx32,_dimx33),\
							(p)->reg = 	gs_g_dimx



#define gs_setR_DTHE(p, _enable)	\
							gs_setDTHE(p, _enable),\
							(p)->reg = 	gs_g_dthe
		

#define gs_setR_COLCLAMP(p, _clamp)	\
							gs_setCOLCLAMP(p, _clamp),\
							(p)->reg = gs_g_colclamp
	

#define gs_setR_TEST_1(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST)	\
							gs_setTEST_1(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST),\
							(p)->reg = gs_g_test_1

#define gs_setR_TEST_2(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST)	\
							gs_setTEST_2(p, _ATE,_ATST,_AREF,_AFAIL,_DATE,_DATM,_ZTE,_ZTST),\
							(p)->reg = gs_g_test_2
	
	
#define gs_setR_PABE(p, _enable)	\
							gs_setPABE(p, _enable),\
							(p)->reg = gs_g_pabe
	

#define gs_setR_FBA(p, _alpha)	\
							gs_setFBA(p, _alpha),\
							(p)->reg = gs_g_fba


#define gs_setR_FRAME_1(p, _fb_addr,_fb_width,_pix_mode,_draw_mask)	\
							gs_setFRAME_1(p, _fb_addr,_fb_width,_pix_mode,_draw_mask),\
							(p)->reg = gs_g_frame_1

#define gs_setR_FRAME_2(p, _fb_addr,_fb_width,_pix_mode,_draw_mask)	\
							gs_setFRAME_2(p, _fb_addr,_fb_width,_pix_mode,_draw_mask),\
							(p)->reg = gs_g_frame_2


#define gs_setR_ZBUF_1(p, _fb_addr,_pix_mode,_update_mask)	\
							gs_setZBUF_1(p, _fb_addr,_pix_mode,_update_mask),\
							(p)->reg = gs_g_zbuf_1

#define gs_setR_ZBUF_2(p, _fb_addr,_pix_mode,_update_mask)	\
							gs_setZBUF_2(p, _fb_addr,_pix_mode,_update_mask),\
							(p)->reg = gs_g_zbuf_2




#define gs_setR_BITBLTBUF(p, _src_addr,_src_width,_src_pixmode,_dest_addr,_dest_width,_dest_pixmode)	\
							gs_setBITBLTBUF(p, _src_addr,_src_width,_src_pixmode,_dest_addr,_dest_width,_dest_pixmode),\
							(p)->reg = gs_g_bitbltbuf


#define gs_setR_TRXPOS(p, _src_x,_src_y,_dest_x,_dest_y,_direction)	\
							gs_setTRXPOS(p, _src_x,_src_y,_dest_x,_dest_y,_direction),\
							(p)->reg = gs_g_trxpos

#define gs_setR_TRXREG(p, _trans_w,_trans_h)	\
							gs_setTRXREG(p, _trans_w,_trans_h),\
							(p)->reg = gs_g_trxreg


#define gs_setR_TRXDIR(p, _trans_dir)	\
							gs_setTRXDIR(p, _trans_dir),\
							(p)->reg = gs_g_trxdir



#define gs_setR_HWREG(p, _data)	\
							gs_setHWREG(p, _data),\
							(p)->reg = gs_g_hwreg


#define gs_setR_SIGNAL(p, _signal_id,_update_mask)	\
							gs_setSIGNAL(p, _signal_id,_update_mask),\
							(p)->reg = gs_g_signal

#define gs_setR_FINISH(p)\
							(p)->reg = gs_g_finish

#define gs_setR_LABEL(p, _label_id,_update_mask)	\
							gs_setLABEL(p, _label_id,_update_mask),\
							(p)->reg = gs_g_label






















/*----------------------------------------------------
--	MID LEVEL DEFINES								--
--													--
--													--
------------------------------------------------------*/

/* ReadCircuit Options For GsSetCRTCMode() */
/*A Default setting*/

#define CRTC_OPTION_DEFAULT				CRTC_OPTION_EN1|CRTC_OPTION_BLENDVAL|CRTC_OPTION_OUTRC2|CRTC_OPTION_STYLERC1

/**/
#define CRTC_OPTION_EN1			((unsigned long)(1)<<0)			// Enable RC1(ReadCircuit 1)
#define CRTC_OPTION_EN2			((unsigned long)(1)<<1)			// Enable RC2(ReadCircuit 1)
#define CRTC_OPTION_ENBOTH		CRTC_OPTION_EN1|CRTC_OPTION_EN2	// Enable RC1 & R2
#define CRTC_OPTION_BLENDRC1	((unsigned long)(0)<<5)			// Use Alpha value from rc1 for blending
#define CRTC_OPTION_BLENDVAL	((unsigned long)(1)<<5)			// Use Alpha value from alpha_value of GsSetCRTCMode() for blending
#define CRTC_OPTION_OUTRC1		((unsigned long)(0)<<6)			// Output Final image to RC1
#define CRTC_OPTION_OUTRC2		((unsigned long)(1)<<6)			// Output Final image to RC2
#define CRTC_OPTION_STYLERC1	((unsigned long)(0)<<7)			// Blend With The Out Put of RC1
#define CRTC_OPTION_STYLEBG		((unsigned long)(1)<<7)			// Blend With The Out Put of BG(background)


/**/


 


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



/*Screen Draw Environment*/
typedef struct {
	unsigned short	offset_x;	// Draw offset X in vram
	unsigned short	offset_y;	// Draw offset Y in vram
	GS_URECT		clip;		// Draw Clip rect
	unsigned short	vram_page;	// Vram Page / Address in frame buffer
	unsigned char	vram_width;	// Width of vram (1=64)
	unsigned char	pix_mode;	// Pixel Mode / PSM
	unsigned short	vram_x;		// X offset in vram;
	unsigned short	vram_y;		// Y offset in vram;
	unsigned int	draw_mask;	// Draw Mask (0=draw, 1=no draw)
	unsigned char	auto_clear;	// Set To 1 If You Want The Draw Environment's Backgroud to Clear When GsPutDrawEnv() is called	
	unsigned char	r,g,b,a;	// Color To Use When Backgroud is Cleared
}GS_DRAWENV;


/*Screen Display Environment*/
typedef struct {
	GS_URECT		screen;		// Screen Offset & Width
	unsigned char	magnify_h;	// Screen Horizontal magnification
	unsigned char	magnify_v;	// Screen Vertical   magnification
	unsigned short	vram_page;	// Vram Page / Address in frame buffer
	unsigned char	vram_width;	// Width of vram (1=64)
	unsigned char	pix_mode;	// Pixel Mode / PSM
	unsigned short	vram_x;		// X offset in vram;
	unsigned short	vram_y;		// Y offset in vram;
}GS_DISPENV;


typedef struct
{
	unsigned short	vram_page;
	unsigned char	pix_mode;
	unsigned char	update_mask;
}GS_ZENV;



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



typedef struct
{
	unsigned short	x;			// X Offset in Vram Address 
	unsigned short	y;			// X Offset in Vram Address
	unsigned short	width;		// Height of image
	unsigned short	height;		// Width  of image
	unsigned short	vram_page;	// Vram Page / Address in frame buffer
	unsigned char	vram_width;	// Width of vram (0=64)
	unsigned char	pix_mode;	// Pixel Mode / PSM
}GS_IMAGE;


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
}GS_EE_IMAGE;


typedef struct  {
	unsigned short			qwc;	
	unsigned short			pad1:10;
	unsigned short			pce	:2;
	unsigned short			id	:3;
	unsigned short			irq	:1;
	unsigned int			addr:31;
	unsigned short			spr	:1;
	unsigned long			pad2;
}sDMA_CHAIN __attribute__((aligned(16)));;


typedef struct
{
	QWORD			pocket;
	 
}GS_GIFPOCKET0;


typedef struct
{
	sDMA_CHAIN		tag;
	QWORD			pocket[128];
	
}GS_GIFPOCKET1;


typedef struct
{
	unsigned long	mode;				//0=normal,1=chain
	unsigned long	path;				//0
	unsigned long	qwords;				//use only in normal mode
	unsigned long	*system0;			//do not use
	QWORD			*pocket_addr;		//a memry buffer that is 64-bit aligned
}GS_GIF_TABLE;





typedef struct
{
	sDMA_CHAIN	chain;
	sDMA_CHAIN  *next;
	 
}GS_GIFPOCKET2;



/*----------------------------------------------------
--	MID LEVEL FUNTIONS								--
--													--
--													--
------------------------------------------------------*/

//extern short GsInit(void);
//extern void	 GsSetVideoMode(unsigned short mode);
//extern short GsSetCRTCMode(unsigned long option, unsigned char alpha_value);

/*Set Structure Members*/
//extern short GsSetDefDrawEnv(GS_DRAWENV *drawenv, unsigned short x, unsigned short y, unsigned short w, unsigned short h);
//extern short GsSetDefDispEnv(GS_DISPENV *dispenv, unsigned short x, unsigned short y, unsigned short w, unsigned short h);
//extern short GsSetDefDrawEnvAddress(GS_DRAWENV *drawenv, unsigned short vram_page, unsigned char	vram_width, unsigned char pix_mode, unsigned short vram_x, unsigned short vram_y);
//extern short GsSetDefDispEnvAddress(GS_DISPENV *dispenv, unsigned short vram_page, unsigned char	vram_width, unsigned char pix_mode, unsigned short vram_x, unsigned short vram_y);
//extern short GsSetDefZBufferEnv(GS_ZENV *zenv, unsigned short vram_addr, unsigned char pix_mode, unsigned char update_mask);
/*Apply To Registers*/
//extern short GsPutDrawEnv1(GS_DRAWENV *drawenv);
//extern short GsPutDrawEnv2(GS_DRAWENV *drawenv);
//extern short GsPutDispEnv1(GS_DISPENV *dispenv);
//extern short GsPutDispEnv2(GS_DISPENV *dispenv);
//extern short GsPutZBufferEnv1(GS_ZENV *zenv);
//extern short GsPutZBufferEnv2(GS_ZENV *zenv);
/*Texture/Image Funtions*/
//extern short GsLoadImage(unsigned int *src_addr, GS_IMAGE *dest);
/**/
//extern short GsLoadTexPage1(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset);
//extern short GsLoadTexPage2(unsigned short tex_addr, unsigned char addr_width, unsigned char tex_pixmode, unsigned short tex_width, unsigned short tex_height, unsigned short clut_addr, unsigned char clut_pixmode, unsigned char clut_storagemode,unsigned char clut_offset);
//extern void  GsDrawGifPocket(GS_GIF_TABLE *pocket);
//extern unsigned int *GsGifPocketAlloc(unsigned short qwords, GS_GIF_TABLE *gif_table);

/*Screen Font Funtions*/
//extern void  GsFntLoad(unsigned short vram_addr, unsigned char addr_width,unsigned char x, unsigned char y);
//extern short GsFntSetColor(unsigned char r, unsigned char g, unsigned char b);
//extern short GsFntSetSize(unsigned short width, unsigned short height);
//extern short GsFntPrint(unsigned short x, unsigned short y, char *str,...);
/**/
//extern void	 GsOverridePrimAttr(char override, char iip, char tme, char fge, char abe, char aa1, char fst, char ctxt, char fix);
//extern void  GsEnableAlphaTransparentcy1(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short update_mode);
//extern void  GsEnableAlphaTransparentcy2(unsigned short enable,unsigned short method,unsigned char alpha_ref,unsigned short update_mode);
//extern void	 GsEnableAlphaBlending1(unsigned short enable);
//extern void  GsEnableZbuffer1(unsigned short enable,unsigned short test_mode);
//extern void  GsEnableZbuffer2(unsigned short enable,unsigned short test_mode);
//extern void  GsEnableColorClamp(unsigned short enable);
/**/
//extern void  GsSetTextureDetail1(unsigned char quality_mode, unsigned char mipmap_count, unsigned char near_quality, unsigned char far_quality, unsigned char mipmap_address_mode, unsigned short L, short K);
//extern void  GsSetTextureDetail2(unsigned char quality_mode, unsigned char mipmap_count, unsigned char near_quality, unsigned char far_quality, unsigned char mipmap_address_mode, unsigned short L, short K);
/**/
//extern void	 GsSetFogColor(unsigned char r, unsigned char g, unsigned char b);
/**/
//extern short GsDrawSync(short mode);
//extern short GsHSync(short mode);
//extern short GsVSync(short mode);
/**/
//extern int	 GsSwapFrameBuffer(void);
//extern int	 GsGetActiveBuffer(void);




//extern GS_DRAWENV		GS_DrawEnv1;
//extern GS_DRAWENV		GS_DrawEnv2;




/*----------------------------------------------------
--	LOW LEVEL FUNTIONS								--
--													--
--													--
------------------------------------------------------*/


/* These Use Gif-Dma To Transfer */
//extern short GsSetXYOffset1(unsigned short x, unsigned short y);
//extern short GsSetXYOffset2(unsigned short x, unsigned short y);
//extern short GsSetScissor1(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y);
//extern short GsSetScissor2(unsigned short upper_x, unsigned short upper_y, unsigned short lower_x, unsigned short lower_y);
//extern short GsSetFrame1(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char pix_mode, unsigned int draw_mask);
//extern short GsSetFrame2(unsigned short framebuffer_addr, unsigned char framebuffer_width, unsigned char pix_mode, unsigned int draw_mask);
//extern short GsTexFlush(void);











/*Write Directly to GS Privileged regs*/

#define GS_SET_PMODE(enable_rc1, enable_rc2, mmod, amod, blend_style, blend_value) \
			*(volatile unsigned long *)gs_p_pmode =			\
		(unsigned long)((enable_rc1	) & 0x00000001) <<  0 | \
		(unsigned long)((enable_rc2	) & 0x00000001) <<  1 | \
		(unsigned long)((1			) & 0x00000007) <<  2 | \
		(unsigned long)((mmod		) & 0x00000001) <<  5 | \
		(unsigned long)((amod		) & 0x00000001) <<  6 | \
		(unsigned long)((blend_style) & 0x00000001) <<  7 | \
		(unsigned long)((blend_value) & 0x000000FF) <<  8


/* un-documented
#define GS_SET_SMODE1(interlace, field_frame, vesta_dpms) \
			*(volatile unsigned long *)gs_p_smode1=			\
		
*/

#define GS_SET_SMODE2(interlace, field_frame, vesta_dpms) \
			*(volatile unsigned long *)gs_p_smode2 =		\
		(unsigned long)((interlace	) & 0x00000001) <<  0 | \
		(unsigned long)((field_frame) & 0x00000001) <<  1 | \
		(unsigned long)((vesta_dpms	) & 0x00000003) <<  2



#define GS_SET_DISPFB1(address, width, pix_mode, x, y) \
			*(volatile unsigned long *)gs_p_dispfb1=\
		(unsigned long)((address	) & 0x000001FF) <<  0 | \
		(unsigned long)((width		) & 0x0000003F) <<  9 | \
		(unsigned long)((pix_mode	) & 0x0000001F) << 15 | \
		(unsigned long)((x			) & 0x000007FF) << 32 | \
		(unsigned long)((y			) & 0x000007FF) << 43



#define GS_SET_DISPFB2(address, width, pix_mode, x, y) \
			*(volatile unsigned long *)gs_p_dispfb2=\
		(unsigned long)((address	) & 0x000001FF) <<  0 | \
		(unsigned long)((width		) & 0x0000003F) <<  9 | \
		(unsigned long)((pix_mode	) & 0x0000001F) << 15 | \
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




















#endif /*_LIBGS_H_*/
