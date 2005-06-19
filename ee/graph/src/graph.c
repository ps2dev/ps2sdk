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

 #include <tamtypes.h>

 #include <dma.h>
 #include <graph.h>
 #include <kernel.h>
 #include <packet.h>
 #include <osd_config.h>

 GRAPH_MODE graph_mode[20] = {
  {  640,  448, 0x02, 1,  286720, GS_SET_DISPLAY(632, 50, 3, 0, 2559,  447) },
  {  640,  512, 0x03, 1,  327680, GS_SET_DISPLAY(652, 72, 3, 0, 2559,  511) },
  {  720,  480, 0x50, 0,  368640, GS_SET_DISPLAY(232, 35, 1, 0, 1439,  479) },
  { 1280,  720, 0x51, 0,  983040, GS_SET_DISPLAY(302, 24, 0, 0, 1279,  719) },
  { 1920, 1080, 0x52, 1, 2088960, GS_SET_DISPLAY(238, 40, 0, 0, 1919, 1079) },
  {  640,  480, 0x1A, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) },
  {  640,  480, 0x1B, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) },
  {  640,  480, 0x1C, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) },
  {  640,  480, 0x1D, 0,  327680, GS_SET_DISPLAY(276, 34, 1, 0, 1279,  479) },
  {  800,  600, 0x2A, 0,  512000, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) },
  {  800,  600, 0x2B, 0,  512000, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) },
  {  800,  600, 0x2C, 0,  512000, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) },
  {  800,  600, 0x2D, 0,  512000, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) },
  {  800,  600, 0x2E, 0,  512000, GS_SET_DISPLAY(420, 26, 1, 0, 1599,  599) },
  { 1024,  768, 0x3B, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) },
  { 1024,  768, 0x3C, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) },
  { 1024,  768, 0x3D, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) },
  { 1024,  768, 0x3E, 0,  786432, GS_SET_DISPLAY(580, 34, 1, 0, 2047,  767) },
  { 1280, 1024, 0x4A, 0, 1310720, GS_SET_DISPLAY(348, 40, 0, 0, 1279, 1023) },
  { 1280, 1024, 0x4B, 0, 1310720, GS_SET_DISPLAY(348, 40, 0, 0, 1279, 1023) },
 };

 PACKET graph_packet;

 int current_displaybuffer = -1;

 int current_drawbuffer = -1;

 int current_drawfield = -1;

 int current_mode = -1;

 int current_psm = -1;

 int current_zbuffer = -1;

 int current_zpsm = -1;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void) {

  // Reset the gif.
  ResetEE(0x08);

  // Reset and flush the gs.
  GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

  // Allocate the packet.
  if (packet_allocate(&graph_packet, 1024) < 0) { return -1; }

  // End function.
  return 0;

 }

 int graph_shutdown(void) {

  // Free the packet.
  if (packet_free(&graph_packet) < 0) { return -1; }

  // End function.
  return 0;

 }

 /////////////////////////
 // GRAPH GET FUNCTIONS //
 /////////////////////////

 float graph_get_aspect(void) {
  float aspect = 1.00f;

  // Determine the aspect as defined by the OSD configuration.
  if (configGetTvScreenType() == TV_SCREEN_169) { aspect = 1.7778f; } else { aspect = 1.3333f; }

  // Return the aspect ratio of the current mode.
  return (float)((float)aspect * (float)((float)graph_get_height() / (float)graph_get_width()));

 }

 int graph_get_bpp(void) {

  // Return the framebuffer bpp of the current mode.
  if (current_psm == GRAPH_PSM_32)  { return 32; } else
  if (current_psm == GRAPH_PSM_24)  { return 24; } else
  if (current_psm == GRAPH_PSM_16)  { return 16; } else
  if (current_psm == GRAPH_PSM_16S) { return 16; } else { return -1; }

  // End function.
  return 0;

 }

 int graph_get_displaybuffer(void) {

  // Return the display buffer vram address of the current mode.
  return current_displaybuffer;

 }

 int graph_get_displayfield(void) {

  // If the current mode is interlaced...
  if (graph_get_interlace() == 1) {

   // Return the display field of the current mode.
   if (GS_REG_CSR & (1 << 13)) { return GRAPH_FIELD_ODD; } else { return GRAPH_FIELD_EVEN; }

  // Else, the current mode is non-interlaced...
  } else {

   // Both fields are being displayed.
   return GRAPH_FIELD_BOTH;

  }

  // End function.
  return 0;

 }

 int graph_get_drawbuffer(void) {

  // Return the draw buffer vram address of the current mode.
  return current_drawbuffer;

 }

 int graph_get_drawfield(void) {

  // If the current mode is interlaced...
  if (graph_get_interlace() == 1) {

   // Return the draw field of the current mode.
   if (GS_REG_CSR & (1 << 13)) { return GRAPH_FIELD_EVEN; } else { return GRAPH_FIELD_ODD; }

  // Else, the mode is non-interlaced...
  } else {

   // Both fields are being drawn.
   return GRAPH_FIELD_BOTH;

  }

  // End function.
  return 0;

 }

 int graph_get_height(void) {

  // Return the pixel height of the current mode.
  return graph_mode[current_mode].height;

 }

 int graph_get_interlace(void) {

  // Return the interlace of the current mode.
  return graph_mode[current_mode].interlace;

 }

 int graph_get_psm(void) {

  // Return the the framebuffer psm of the current mode.
  return current_psm;

 }

 int graph_get_size(void) {

  // Return the framebuffer size of the current mode.
  return graph_mode[current_mode].size * (graph_get_bpp() >> 3);

 }

 int graph_get_width(void) {

  // Return the pixel width of the current mode.
  return graph_mode[current_mode].width;

 }

 int graph_get_zbpp(void) {

  // Return the zbuffer bpp of the current mode.
  if (current_zpsm == GRAPH_PSM_32)  { return 32; } else
  if (current_zpsm == GRAPH_PSM_24)  { return 24; } else
  if (current_zpsm == GRAPH_PSM_16)  { return 16; } else
  if (current_zpsm == GRAPH_PSM_16S) { return 16; } else { return -1; }

  // End function.
  return 0;

 }

 int graph_get_zbuffer(void) {

  // Return the zbuffer vram address of the current mode.
  return current_zbuffer;

 }

 int graph_get_zpsm(void) {

  // Return the zbuffer psm of the current mode.
  return current_zpsm;

 }

 int graph_get_zsize(void) {

  // Return the zbuffer size of the current mode.
  return graph_mode[current_mode].size * (graph_get_zbpp() >> 3);

 }

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_clearbuffer(int red, int green, int blue) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Clear the draw framebuffer and zbuffer.
  packet_append_64(&graph_packet, GIF_SET_TAG(6, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);
  packet_append_64(&graph_packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_PRIM);
  packet_append_64(&graph_packet, GIF_SET_RGBAQ(red, green, blue, 0x80, 0x3F800000));
  packet_append_64(&graph_packet, GIF_REG_RGBAQ);
  packet_append_64(&graph_packet, GIF_SET_XYZ(0x0000, 0x0000, 0x0000));
  packet_append_64(&graph_packet, GIF_REG_XYZ2);
  packet_append_64(&graph_packet, GIF_SET_XYZ(0xFFFF, 0xFFFF, 0x0000));
  packet_append_64(&graph_packet, GIF_REG_XYZ2);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 int graph_set_displaybuffer(int address) {

  // Set up the display buffer.
  GS_REG_DISPFB1 = GS_SET_DISPFB(address >> 13, graph_get_width() >> 6, current_psm, 0, 0);
  GS_REG_DISPFB2 = GS_SET_DISPFB(address >> 13, graph_get_width() >> 6, current_psm, 0, 0);

  // Save the display buffer.
  current_displaybuffer = address;

  // End function.
  return 0;

 }

 int graph_set_drawbuffer(int address) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Set up the draw buffer.
  packet_append_64(&graph_packet, GIF_SET_TAG(5, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, GIF_SET_FRAME(address >> 13, graph_get_width() >> 6, current_psm, 0));
  packet_append_64(&graph_packet, GIF_REG_FRAME_1);
  packet_append_64(&graph_packet, GIF_SET_SCISSOR(0, graph_get_width() - 1, 0, graph_get_height() - 1));
  packet_append_64(&graph_packet, GIF_REG_SCISSOR_1);
  packet_append_64(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  packet_append_64(&graph_packet, GIF_REG_TEST_1);
  packet_append_64(&graph_packet, GIF_SET_XYOFFSET((2048 - (graph_get_width() >> 1)) << 4, (2048 - (graph_get_height() >> 1)) << 4));
  packet_append_64(&graph_packet, GIF_REG_XYOFFSET_1);
  packet_append_64(&graph_packet, GIF_SET_PRMODECONT(1));
  packet_append_64(&graph_packet, GIF_REG_PRMODECONT);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Save the draw buffer.
  current_drawbuffer = address;

  // End function.
  return 0;

 }

 int graph_set_drawfield(int field) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // If the field is odd...
  if (field == GRAPH_FIELD_ODD) {

   // Draw only to odd fields.
   packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
   packet_append_64(&graph_packet, 0x000000000000000E);
   packet_append_64(&graph_packet, GIF_SET_SCANMSK(2));
   packet_append_64(&graph_packet, GIF_REG_SCANMSK);

  // Else if the field is even...
  } else if (field == GRAPH_FIELD_EVEN) {

   // Draw only to even fields.
   packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
   packet_append_64(&graph_packet, 0x000000000000000E);
   packet_append_64(&graph_packet, GIF_SET_SCANMSK(3));
   packet_append_64(&graph_packet, GIF_REG_SCANMSK);

  // Else, the mode is non-interlaced...
  } else {

   // Draw to all fields.
   packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
   packet_append_64(&graph_packet, 0x000000000000000E);
   packet_append_64(&graph_packet, GIF_SET_SCANMSK(0));
   packet_append_64(&graph_packet, GIF_REG_SCANMSK);

  }

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Save the draw field.
  current_drawfield = field;

  // End function.
  return 0;

 }

 int graph_set_mode(int mode, int psm, int zpsm) {

  // If an automatic mode is requested...
  if (mode == GRAPH_MODE_AUTO) {

   // If the region is SCEE...
   if (*(volatile char *)(0x1FC7FF52) == 'E') {

    // The default mode is PAL.
    mode = GRAPH_MODE_PAL;

   // Else, the default mode is NTSC.
   } else { mode = GRAPH_MODE_NTSC; }

  }

  // Request the mode change.
  SetGsCrt(graph_mode[mode].interlace, graph_mode[mode].mode, 0);

  // Set up the mode.
  GS_REG_PMODE = GS_SET_PMODE(1, 1, 0, 0, 0, 128);
  GS_REG_DISPLAY1 = graph_mode[mode].display;
  GS_REG_DISPLAY2 = graph_mode[mode].display;

  // Save the mode, psm and zpsm.
  current_mode = mode; current_psm = psm; current_zpsm = zpsm;

  // End function.
  return 0;

 }

 int graph_set_texture(int address, int width, int height, int psm) {
  int tw = 8, th = 8;

// FIXME: This isn't finished yet.

  // Determine the TW value.
  if (width == 256) { tw = 8; }

  // Determine the TH value.
  if (height == 256) { th = 8; }

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Set up the texture.
  packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 3));
  packet_append_64(&graph_packet, 0x0000000000000EEE);
  packet_append_64(&graph_packet, GIF_SET_CLAMP(0, 0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_CLAMP_1);
  packet_append_64(&graph_packet, GIF_SET_TEX0(address >> 8, width >> 6, psm, tw, th, 0, 1, 0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TEX0_1);
  packet_append_64(&graph_packet, GIF_SET_TEX1(1, 0, 1, 1, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TEX1_1);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 int graph_set_zbuffer(int address) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Set up the zbuffer.
  packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, GIF_SET_ZBUF(address >> 13, current_zpsm, 0));
  packet_append_64(&graph_packet, GIF_REG_ZBUF_1);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Save the zbuffer.
  current_zbuffer = address;

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 #define VIF1_REG_STAT *((vu32 *)(0x10003C00))

 int graph_vram_read(int address, int width, int height, int psm, void *data, int data_size) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Build the packet.
  packet_append_64(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, GIF_SET_BITBLTBUF(address >> 8, width >> 6, psm, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_BITBLTBUF);
  packet_append_64(&graph_packet, GIF_SET_TRXPOS(0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TRXPOS);
  packet_append_64(&graph_packet, GIF_SET_TRXREG(width, height));
  packet_append_64(&graph_packet, GIF_REG_TRXREG);
  packet_append_64(&graph_packet, GIF_SET_TRXDIR(1));
  packet_append_64(&graph_packet, GIF_REG_TRXDIR);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Wait for the packet transfer to complete.
  if (dma_channel_wait(DMA_CHANNEL_GIF, -1, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Reverse the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(1); VIF1_REG_STAT = (1 << 23);

  // Receive the data.
  if (dma_channel_receive(DMA_CHANNEL_VIF1, data, data_size, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Wait for the data transfer to complete.
  if (dma_channel_wait(DMA_CHANNEL_VIF1, -1, DMA_FLAG_NORMAL) < 0) { return -1; }

  // Restore the bus direction.
  GS_REG_BUSDIR = GS_SET_BUSDIR(0); VIF1_REG_STAT = (0 << 23);

  // Flush the cache, just in case.
  FlushCache(0);

  // End function.
  return 0;

 }

 int graph_vram_write(int address, int width, int height, int psm, void *data, int data_size) {

  // Reset the packet.
  if (packet_reset(&graph_packet) < 0) { return -1; }

  // Build the packet.
  packet_append_64(&graph_packet, DMA_SET_TAG(6, 0, DMA_TAG_CNT, 0, 0, 0));
  packet_append_64(&graph_packet, 0x0000000000000000);
  packet_append_64(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, GIF_SET_BITBLTBUF(0, 0, 0, address >> 8, width >> 6, psm));
  packet_append_64(&graph_packet, GIF_REG_BITBLTBUF);
  packet_append_64(&graph_packet, GIF_SET_TRXPOS(0, 0, 0, 0, 0));
  packet_append_64(&graph_packet, GIF_REG_TRXPOS);
  packet_append_64(&graph_packet, GIF_SET_TRXREG(width, height));
  packet_append_64(&graph_packet, GIF_REG_TRXREG);
  packet_append_64(&graph_packet, GIF_SET_TRXDIR(0));
  packet_append_64(&graph_packet, GIF_REG_TRXDIR);
  packet_append_64(&graph_packet, GIF_SET_TAG(data_size >> 4, 1, 0, 0, GIF_TAG_IMAGE, 1));
  packet_append_64(&graph_packet, 0x0000000000000000);
  packet_append_64(&graph_packet, DMA_SET_TAG(data_size >> 4, 0, DMA_TAG_REF, 0, (u32)data, 0));
  packet_append_64(&graph_packet, 0x0000000000000000);
  packet_append_64(&graph_packet, DMA_SET_TAG(2, 0, DMA_TAG_END, 0, 0, 0));
  packet_append_64(&graph_packet, 0x0000000000000000);
  packet_append_64(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  packet_append_64(&graph_packet, 0x000000000000000E);
  packet_append_64(&graph_packet, 0x0000000000000000);
  packet_append_64(&graph_packet, GIF_REG_TEXFLUSH);

  // Send the packet.
  if (packet_send(&graph_packet, DMA_CHANNEL_GIF, DMA_FLAG_NORMAL) < 0) { return -1; }

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH WAIT FUNCTIONS //
 //////////////////////////

 int graph_wait_hsync(void) {

  // Enable the hsync interrupt.
  GS_REG_CSR |= GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the hsync interrupt.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the hsync interrupt.
  GS_REG_CSR &= ~GS_SET_CSR(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }

 int graph_wait_vsync(void) {

  // Enable the vsync interrupt.
  GS_REG_CSR |= GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the vsync interrupt.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the vsync interrupt.
  GS_REG_CSR &= ~GS_SET_CSR(0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }
