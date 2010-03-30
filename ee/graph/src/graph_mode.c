#include <stdlib.h>
#include <kernel.h>
#include <osd_config.h>

#include <gs_privileged.h>

#include <graph.h>
#include <graph_config.h>

typedef struct {
	int x,y; 
	int width, height; 
	int mode; 
} GRAPH_MODE;
 
/* y offset is non-interlaced */
GRAPH_MODE graph_mode[22] =
{

	{   0,   0,    0,    0,    0 }, // AUTO
	{ 652,  26, 2560,  256, 0x02 }, // NTSC-NI
	{ 680,  37, 2560,  256, 0x03 }, // PAL-NI
	{ 232,  35, 1440,  480, 0x50 }, // 480P
	{ 320,  64, 1312,  576, 0x53 }, // 576P only in bios>=220
	{ 420,  40, 1280,  720, 0x52 }, // 720P
	{ 300, 120, 1920,  540, 0x51 }, // 1080I
	{ 280,  18, 1280,  480, 0x1A }, // VGA   640x480@60
	{ 330,  18, 1280,  480, 0x1B }, // VGA   640x480@72
	{ 360,  18, 1280,  480, 0x1C }, // VGA   640x480@75
	{ 260,  18, 1280,  480, 0x1D }, // VGA   640x480@85
	{ 450,  25, 1600,  600, 0x2A }, // VGA   800x600@56
	{ 465,  25, 1600,  600, 0x2B }, // VGA   800x600@60
	{ 465,  25, 1600,  600, 0x2C }, // VGA   800x600@72
	{ 510,  25, 1600,  600, 0x2D }, // VGA   800x600@75
	{ 500,  25, 1600,  600, 0x2E }, // VGA   800x600@85
	{ 580,  30, 2048,  768, 0x3B }, // VGA  1024x768@60
	{ 266,  30, 1024,  768, 0x3C }, // VGA  1024x768@70
	{ 260,  30, 1024,  768, 0x3D }, // VGA  1024x768@75
	{ 290,  30, 1024,  768, 0x3E }, // VGA  1024x768@85
	{ 350,  40, 1280, 1024, 0x4A }, // VGA 1280x1024@60
	{ 350,  40, 1280, 1024, 0x4B }, // VGA 1280x1024@75

};

static float graph_width = 0.0f;
static float graph_height = 0.0f;
static float graph_aspect = 1.0f;

static int graph_filter = 0;
static int graph_crtmode = 0;
static int graph_interlace = 0;
static int graph_ffmd = 0;
static int graph_x = 0;
static int graph_y = 0;
static int graph_magh = 0;
static int graph_magv = 0;

static unsigned long graph_pmode = 0;

// old bios has GCONT enabled
unsigned long smode1_values[22] =
{

	0,
	GS_SET_SMODE1(4,32,1,0,2,0,1,1,0,0,4,0,0,0,0,0,1,1,1,1,0), //0x02
	GS_SET_SMODE1(4,32,1,0,3,0,1,1,0,0,4,0,0,0,0,0,1,1,1,1,0), //0x03
	GS_SET_SMODE1(4,32,1,0,0,0,1,1,0,0,2,0,0,0,0,0,1,1,1,1,1), //0x50
	GS_SET_SMODE1(4,32,1,0,0,0,1,1,0,1,2,0,0,0,0,0,1,1,1,1,1), //0x53
	GS_SET_SMODE1(2,22,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x52
	GS_SET_SMODE1(2,22,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,0), //0x51
	GS_SET_SMODE1(2,15,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x1A
	GS_SET_SMODE1(3,28,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x1B
	GS_SET_SMODE1(3,28,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x1C
	GS_SET_SMODE1(3,16,0,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x1D
	GS_SET_SMODE1(3,16,0,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x2A
	GS_SET_SMODE1(6,71,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x2B
	GS_SET_SMODE1(5,74,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x2C
	GS_SET_SMODE1(3,44,1,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x2D
	GS_SET_SMODE1(3,25,0,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x2E
	GS_SET_SMODE1(3,29,0,0,0,0,0,1,0,0,2,0,0,0,0,0,1,1,1,0,1), //0x3B
	GS_SET_SMODE1(6,67,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x3C
	GS_SET_SMODE1(3,35,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x3D
	GS_SET_SMODE1(1, 7,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x3E
	GS_SET_SMODE1(1, 8,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x4A
	GS_SET_SMODE1(1,10,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,1,0,1), //0x4B

};

static inline int __udelay(unsigned int usecs)
{
 
	register unsigned int loops_total = 0;
	register unsigned int loops_end   = usecs * 148;

	if (usecs > loops_end)
	{

		return -1;

	}

	asm volatile (".set noreorder\n\t"
				  "0:\n\t"
				  "beq %0,%2,0f\n\t"
				  "addiu %0,1\n\t"
				  "bne %0,%2,0b\n\t"
				  "addiu %0,1\n\t"
				  "0:\n\t"
				  ".set reorder\n\t"
				  :"=r" (loops_total)
				  :"0" (loops_total), "r" (loops_end));

	return 0;

}

int graph_get_region(void)
{

	char romname[14];

	GetRomName((char *)romname);

	if (romname[4] == 'E')
	{
		return GRAPH_MODE_PAL;
	}

	return GRAPH_MODE_NTSC;

}

int graph_set_mode(int interlace, int mode, int ffmd, int flicker_filter)
{

	char romname[14];

	// Reset GS.
	*GS_REG_CSR = (unsigned long)1<<9;

	// Clear GS CSR.
	*GS_REG_CSR = GS_SET_CSR(0,0,0,0,0,0,0,0,0,0,0,0);

	// Unmask GS VSYNC Interrupt.
	GsPutIMR(0x00007700);

	// Ensure registers are written prior to setting another mode.
	asm volatile ("sync.p\n\t"
				  "nop\n\t");

	// If 576P is requested, check if bios supports it.
	if (mode == GRAPH_MODE_HDTV_576P)
	{

		GetRomName((char *)romname);

		if (strtol(romname,NULL,10) < 220) { mode = GRAPH_MODE_PAL; }

	}

	// 1080I is forced to be interlaced so correct value just in case.
	if (mode == GRAPH_MODE_HDTV_1080I)
	{

		interlace = GRAPH_MODE_INTERLACED;

	}

	if (interlace == GRAPH_MODE_NONINTERLACED)
	{

		flicker_filter = GRAPH_DISABLE;

	}

	// Set graph's mode, interlacing and field mode.
	graph_crtmode = mode;
	graph_interlace = interlace;
	graph_ffmd = ffmd;
	graph_filter = flicker_filter;

	// Set the requested mode.
	SetGsCrt(graph_interlace, graph_crtmode, graph_ffmd);

	return 0;

}

int graph_set_screen(int x, int y, int width, int height)
{

	int dx,dy,dw,dh;

	graph_x = x;
	graph_y = y;

	graph_width = (float)width;
	graph_height = (float)height;

	// Check if the mode has been set
	if (graph_crtmode == GRAPH_MODE_AUTO)
	{

		return -1;

	}

	// Add X adjustment to default X offset
	dx = graph_mode[graph_crtmode].x + graph_x;

	// Get default Y offset
	dy = graph_mode[graph_crtmode].y;

	// Get screen's width and height
	dw = graph_mode[graph_crtmode].width;
	dh = graph_mode[graph_crtmode].height;

	// Double Y offset for interlacing in FIELD mode
	// Double screen's height parameter 
	if ((graph_interlace) && (graph_ffmd == GRAPH_MODE_FIELD))
	{

		dy = (dy - 1) * 2;
		dh = graph_mode[graph_crtmode].height * 2;

	}

	// Now add Y adjustment
	dy += graph_y;

	// Determine magnification
	graph_magh = dw / width;
	graph_magv = dh / height;

	// Make sure it doesn't turn negative
	if (graph_magh < 1)
	{

		graph_magh = 1;

	}

	if (graph_magv < 1)
	{

		graph_magv = 1;

	}

	// Set the display attributes but use the user defined height.
	if (graph_filter)
	{

		// For flicker filter, we need to get add an extra line.
		*GS_REG_DISPLAY1 = GS_SET_DISPLAY(dx,dy,graph_magh-1,graph_magv-1,dw-1,height-1);
		*GS_REG_DISPLAY2 = GS_SET_DISPLAY(dx,dy,graph_magh-1,graph_magv-1,dw-1,height-2);

	}
	else
	{

		*GS_REG_DISPLAY1 = GS_SET_DISPLAY(dx,dy,graph_magh-1,graph_magv-1,dw-1,height-1);
		*GS_REG_DISPLAY2 = GS_SET_DISPLAY(dx,dy,graph_magh-1,graph_magv-1,dw-1,height-1);

	}

	return 0;

}

void graph_set_framebuffer_filtered(int fbp, int width, int psm, int x, int y)
{

	*GS_REG_DISPFB1 = GS_SET_DISPFB(fbp>>11,width>>6,psm,x,y);

	// For flicker filter, we need to offset the lines by 1 for the other read circuit.
	*GS_REG_DISPFB2 = GS_SET_DISPFB(fbp>>11,width>>6,psm,x,y+1);

}

void graph_set_framebuffer(int context, int fbp, int width, int psm, int x, int y)
{

	if (context == 0)
	{

		*GS_REG_DISPFB1 = GS_SET_DISPFB(fbp>>11,width>>6,psm,x,y);

	}
	else
	{

		// For flicker filter, we need to offset the lines by 1 for the other read circuit.
		*GS_REG_DISPFB2 = GS_SET_DISPFB(fbp>>11,width>>6,psm,x,y);

	}

}

void graph_set_bgcolor(unsigned char r, unsigned char g, unsigned char b)
{

	*GS_REG_BGCOLOR = GS_SET_BGCOLOR(r,g,b);

}

void graph_set_output(int rc1, int rc2, int alpha_select, int alpha_output, int blend_method, unsigned char alpha)
{

	graph_pmode  = GS_SET_PMODE(rc1, rc2, alpha_select, alpha_output, blend_method, alpha);
	*GS_REG_PMODE = graph_pmode;

}

// Sets up the frame in rc1 to be blended with rc2 using a constant alpha value.
// By offsetting the frame a line in the rc2 display/dispfb registers, the odd
// numbered lines are more biased against the even lines, producing less jitter.
void graph_enable_output(void)
{

	if (graph_filter)
	{

		graph_set_output(GRAPH_ENABLE,GRAPH_ENABLE,GRAPH_VALUE_ALPHA,GRAPH_RC1_ALPHA,GRAPH_BLEND_RC2,0x70);

	}
	else
	{

		graph_set_output(0,1,0,1,0,0x80);

	}

}

void graph_disable_output(void)
{

	graph_set_output(0,0,0,0,0,0);

}

void graph_set_smode1(char cmod, char gcont)
{

	u64 pmode_val;
	u64 smode1_val;

	// Get the current values;
	pmode_val = graph_pmode;
	smode1_val = smode1_values[graph_crtmode];

	if (smode1_val == 0)
	{

		return;

	}

	// If not VESA, set analog or digital output.
	if ((graph_crtmode < 0x04) || (graph_crtmode > 0x50))
	{

		smode1_val |= (unsigned long)(gcont & 1) << 25;

	}

	// If mode is a TV mode, set carrier modulation.
	if ((graph_crtmode < 0x04) && (cmod > 0x01) && (cmod < 0x04))
	{

		smode1_val |= (unsigned long)(cmod & 3) << 13;

	}

	// Turn off read circuits.
	*GS_REG_PMODE  = graph_pmode & ~3;

	// Disable PRST for TV modes and enable for all other modes.
	*GS_REG_SMODE1 = smode1_val | (unsigned long)1 << 16;

	asm volatile ("sync.l; sync.p;");

	// If VESA, 1080I, or 720P, disable bit PRST now and delay 2.5ms.
	if ((graph_crtmode >= 0x1A) && (graph_crtmode != 0x50) && (graph_crtmode != 0x53))
	{

		*GS_REG_SMODE1 = smode1_val & ~((unsigned long)1 << 16);
		__udelay(2500);

	}

	// Now disable both bits PRST & SINT.
	*GS_REG_SMODE1 = smode1_val & ~((unsigned long)1 << 16) & ~((unsigned long)1 << 17);

	// Now enable read circuits.
	*GS_REG_PMODE  = pmode_val;

	asm volatile ("sync.l; sync.p;");

}

float graph_aspect_ratio(void)
{

	// Get the tv screen type as defined in the osd configuration.
	if (configGetTvScreenType() == TV_SCREEN_169)
	{

		graph_aspect = 1.78f;

	}
	else
	{

		graph_aspect = 1.33f;

	}

	// Return the current aspect ratio
	graph_aspect = graph_aspect * (graph_height / graph_width);

	return graph_aspect;

}

int graph_get_config(char *config)
{

	return graph_make_config(graph_crtmode, graph_interlace, graph_ffmd, graph_x, graph_y, graph_filter, config);

}

int graph_shutdown(void)
{

	graph_disable_output();

	// Reset GS.
	*GS_REG_CSR = (unsigned long)1<<9;

	// Clear GS CSR.
	*GS_REG_CSR = GS_SET_CSR(0,0,0,0,0,0,0,0,0,0,0,0);

	// Reset the static variables.
	graph_aspect = 1.0f;
	graph_width = 0.0f;
	graph_height = 0.0f;
	graph_filter = 0;
	graph_crtmode = 0;
	graph_interlace = 0;
	graph_ffmd = 0;
	graph_x = 0;
	graph_y = 0;
	graph_magh = 0;
	graph_magv = 0;

	return 0;

}
