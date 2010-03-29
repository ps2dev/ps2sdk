#include <kernel.h>
#include <gs_privileged.h>

#include <graph.h>

int graph_initialize(int fbp, int width, int height, int psm, int x, int y)
{

	int mode = graph_get_region();

	// Set a default interlaced video mode with flicker filter.
	graph_set_mode(GRAPH_MODE_INTERLACED, mode, GRAPH_MODE_FIELD, GRAPH_ENABLE);

	// Set the screen dimensions to PAL, for increased frame buffer.
	if (mode == GRAPH_MODE_NTSC)
	{
		graph_set_screen(0,0,width,height);
 }
	else
	{
		graph_set_screen(0,0,width,height);
 }

	// Set black background
	graph_set_bgcolor(0,0,0);

	graph_set_framebuffer_filtered(fbp,width,psm,x,y);

	graph_enable_output();

  // End function.
  return 0;

}

int graph_add_vsync_handler(int (*vsync_callback)())
{

	int callback_id;

	DIntr();

	callback_id = AddIntcHandler(2, vsync_callback, -1);

	EnableIntc(2);

	EIntr();

	return callback_id;

}

void graph_remove_vsync_handler(int callback_id)
{

	DIntr();

	DisableIntc(2);

	RemoveIntcHandler(2, callback_id);

	EIntr();

}

int graph_get_field(void)
{

	// Return the currently displayed field.
	if (*GS_REG_CSR & (1 << 13))
	{

		return GRAPH_FIELD_ODD;

 }

	return GRAPH_FIELD_EVEN;

}

void graph_wait_hsync(void)
{

	// Initiate hsync interrupt
	*GS_REG_CSR |= *GS_REG_CSR & 4;

	// Wait for hsync interrupt to be generated.
	while (!(*GS_REG_CSR & 4));

}

int graph_check_vsync(void)
{

	return (*GS_REG_CSR & 8);

}

void graph_start_vsync(void)
{

	*GS_REG_CSR |= *GS_REG_CSR & 8;

}

void graph_wait_vsync(void)
{

	// Initiate vsync interrupt.
	*GS_REG_CSR |= *GS_REG_CSR & 8;

	// Wait for vsync interrupt to be generated.
	while (!(*GS_REG_CSR & 8));

}
