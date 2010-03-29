#ifndef __GRAPH_H__
#define __GRAPH_H__

// Sets screen mode
#define GRAPH_MODE_AUTO				0	//  Automatic NTSC or PAL mode setting.
#define GRAPH_MODE_NTSC				1	//  256 x  224 to 640 x 448
#define GRAPH_MODE_PAL				2	//  256 x  256 to 640 x 512
#define GRAPH_MODE_HDTV_480P		3	//  720 x  480
#define GRAPH_MODE_HDTV_576P		4	//  656 x  576
#define GRAPH_MODE_HDTV_720P		5	// 1280 x  720
#define GRAPH_MODE_HDTV_1080I		6	// 1920 x 1080
#define GRAPH_MODE_VGA_640_60		7	//  640 x  480 @ 60hz
#define GRAPH_MODE_VGA_640_72		8	//  640 x  480 @ 72hz
#define GRAPH_MODE_VGA_640_75		9	//  640 x  480 @ 75hz
#define GRAPH_MODE_VGA_640_85		10	//  640 x  480 @ 85hz
#define GRAPH_MODE_VGA_800_56		11	//  800 x  600 @ 56hz
#define GRAPH_MODE_VGA_800_60		12	//  800 x  600 @ 60hz
#define GRAPH_MODE_VGA_800_72		13	//  800 x  600 @ 72hz
#define GRAPH_MODE_VGA_800_75		14	//  800 x  600 @ 75hz
#define GRAPH_MODE_VGA_800_85		15	//  800 x  600 @ 85hz
#define GRAPH_MODE_VGA_1024_60		16	// 1024 x  768 @ 60hz
#define GRAPH_MODE_VGA_1024_70		17	// 1024 x  768 @ 70hz
#define GRAPH_MODE_VGA_1024_75		18	// 1024 x  768 @ 75hz
#define GRAPH_MODE_VGA_1024_85		19	// 1024 x  768 @ 85hz
#define GRAPH_MODE_VGA_1280_60		20	// 1280 x 1024 @ 60hz
#define GRAPH_MODE_VGA_1280_75		21	// 1280 x 1024 @ 75hz

#define GRAPH_MODE_NONINTERLACED	0
#define GRAPH_MODE_INTERLACED		1

#define GRAPH_MODE_FIELD			0
#define GRAPH_MODE_FRAME			1
 
#define GRAPH_FIELD_EVEN			0
#define GRAPH_FIELD_ODD				1

// Generic use
#define GRAPH_DISABLE				0
#define GRAPH_ENABLE				1

// Smode1 parameters
#define GRAPH_GCONT_RGB				0
#define GRAPH_GCONT_YCRCB			1

#define GRAPH_CMOD_NTSC				2
#define GRAPH_CMOD_PAL				3

// Alpha Blending value to use
#define GRAPH_VALUE_RC1				0
#define GRAPH_VALUE_ALPHA			1

// Alpha output value
#define GRAPH_RC1_ALPHA				0
#define GRAPH_RC2_ALPHA				1

// Alpha blending method
#define GRAPH_BLEND_RC2				0
#define GRAPH_BLEND_BGCOLOR			1

#ifdef __cplusplus
extern "C" {
#endif

	// Initializes a default NTSC/PAL mode with default settings.
	int graph_initialize(int fbp, int width, int height, int psm, int x, int y);

	// Retrieves the PS2's region for automatic mode selection.
	int graph_get_region(void);

	// Returns an aspect ratio calculated from system setting, width, and height
	float graph_aspect_ratio(void);

	// Sets a default output method.
	void graph_enable_output(void);

	// Turns off the read circuits.
	void graph_disable_output(void);

	// Sets the graphics mode.
	int graph_set_mode(int interlace, int mode, int ffmd, int flicker_filter);

	// Sets the screen dimensions for the read circuits.
	int graph_set_screen(int x, int y, int width, int height);

	// Sets the framebuffer attributes for the read circuits with filter.
	void graph_set_framebuffer_filtered(int fbp, int width, int psm, int x, int y);

	// Sets the framebuffer attributes for the read circuits.
	void graph_set_framebuffer(int context, int fbp, int width, int psm, int x, int y);

	// Sets the background color for merge circuit.
	void graph_set_bgcolor(unsigned char r, unsigned char g, unsigned char b);

	// Sets the read circuits and merge cicuit.
	void graph_set_output(int rc1, int rc2, int alpha_select, int alpha_output, int blend_method, unsigned char alpha);

	// Add a vsync interrupt handler
	int graph_add_vsync_handler(int (*vsync_callback)());

	// Remove a vsync interrupt handler
	void graph_remove_vsync_handler(int callback_id);

	// Starts a horizontal sync event and waits
	void graph_wait_hsync(void);

	// Starts a vertical sync event and waits.
	void graph_wait_vsync(void);

	// Checks if a vertical sync event is currently generated.
	int graph_check_vsync(void);

	// Starts a vertical sync event and returns immediately.
	void graph_start_vsync(void);

	// Shut down the graphics library and hardware.
	int graph_shutdown(void);

	///****************USE AT OWN RISK****************///
	/// Sets PLL sync generator control register.     ///
	///                                               ///
	/// gcont sets RGB or RGB->YCrCb                  ///
	/// cmod  sets the color subcarrier               ///
	/// 2 = NTSC 3 = PAL                              ///
	///                                               ///
	/// Doesn't work...                               ///
	///                                               ///
	///void graph_set_smode1(char cmod, char gcont);  ///
	///****************USE AT OWN RISK****************///

#ifdef __cplusplus
}
#endif

#endif /*__GRAPH_H__*/
