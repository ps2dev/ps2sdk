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

 #include <graph.h>
 #include <graph_registers.h>

 #include <../../dma/include/dma.h>
 #include <../../dma/include/dma_registers.h>

 #include <tamtypes.h>
 #include <kernel.h>

 GRAPH_MODE graph_mode[4] = {
  { 640, 448, 0x02, 1, 660, 48, 3, 0, 2559, 447 }, // NTSC (640x448i)
  { 640, 512, 0x03, 1, 652, 30, 3, 0, 2559, 511 }, // PAL  (640x512i)
  { 720, 480, 0x50, 0, 232, 35, 2, 0, 2159, 479 }, // HDTV (720x480p)
  { 640, 480, 0x1A, 0, 280, 18, 3, 0, 2559, 479 }  // VGA  (640x480p)
 };

 DMA_PACKET graph_packet;

 int current_mode = 0, current_bpp = 0;

 /////////////////////
 // GRAPH FUNCTIONS //
 /////////////////////

 int graph_initialize(void) {

  // Reset the gif.
  ResetEE(0x08);

  // Reset and flush the gs.
  GS_REG_CSR = GS_SET_CSR(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0);

  // Initialize the gif dma channel.
  dma_channel_initialize(DMA_CHANNEL_GIF, NULL);

  // Allocate an internal use packet.
  dma_packet_allocate(&graph_packet, 1024);

  // End function.
  return 0;

 }

 int graph_shutdown(void) {

  // Shut down the dma gif channel.
  dma_channel_shutdown(DMA_CHANNEL_GIF);

  // Free the internal use packet.
  dma_packet_free(&graph_packet);

  // End function.
  return 0;

 }

 /////////////////////////
 // GRAPH SET FUNCTIONS //
 /////////////////////////

 int graph_set_displaymode(int mode, int bpp) {

  // Request the mode change.
  SetGsCrt(graph_mode[mode].interlace, graph_mode[mode].mode, 0);

  // Set up the mode.
  GS_REG_PMODE = GS_SET_PMODE(1, 1, 0, 0, 0, 256);
  GS_REG_DISPLAY1 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);
  GS_REG_DISPLAY2 = GS_SET_DISPLAY(graph_mode[mode].dx, graph_mode[mode].dy, graph_mode[mode].magh, graph_mode[mode].magv, graph_mode[mode].dw, graph_mode[mode].dh);

  // Save the mode and bpp.
  current_mode = mode; current_bpp = bpp;

  // End function.
  return 0;

 }

 int graph_set_displaybuffer(int address) {

  // Set up the display buffer.
  GS_REG_DISPFB1 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_bpp, 0, 0);
  GS_REG_DISPFB2 = GS_SET_DISPFB(address >> 13, graph_mode[current_mode].width >> 6, current_bpp, 0, 0);

  // End function.
  return 0;

 }

 int graph_set_drawbuffer(int address, int width, int height, int bpp) {

  // Autofill width, height and bpp if none are given.
  if (width  < 0) { width  = graph_mode[current_mode].width;  }
  if (height < 0) { height = graph_mode[current_mode].height; }
  if (bpp    < 0) { bpp = current_bpp; }

  // Set up the draw buffer.
  dma_packet_clear(&graph_packet);
  dma_packet_append(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  dma_packet_append(&graph_packet, 0x0E);
  dma_packet_append(&graph_packet, GIF_SET_FRAME(address >> 13, width >> 6, bpp, 0));
  dma_packet_append(&graph_packet, GIF_REG_FRAME_1);
  dma_packet_append(&graph_packet, GIF_SET_SCISSOR(0, width - 1, 0, height - 1));
  dma_packet_append(&graph_packet, GIF_REG_SCISSOR_1);
  dma_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  dma_packet_append(&graph_packet, GIF_REG_TEST_1);
  dma_packet_append(&graph_packet, GIF_SET_XYOFFSET(0, 0));
  dma_packet_append(&graph_packet, GIF_REG_XYOFFSET_1);
  dma_packet_send(&graph_packet, DMA_CHANNEL_GIF);

  // End function.
  return 0;

 }

 int graph_set_zbuffer(int address, int bpp) {

  // Set up the zbuffer.
  dma_packet_clear(&graph_packet);
  dma_packet_append(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  dma_packet_append(&graph_packet, 0x0E);
  dma_packet_append(&graph_packet, GIF_SET_ZBUF(address >> 13, bpp, 0));
  dma_packet_append(&graph_packet, GIF_REG_ZBUF_1);
  dma_packet_send(&graph_packet, DMA_CHANNEL_GIF);

  // End function.
  return 0;

 }

 int graph_set_clearbuffer(int red, int green, int blue, int alpha) {

  // Clear the screen.
  dma_packet_clear(&graph_packet);
  dma_packet_append(&graph_packet, GIF_SET_TAG(6, 1, 0, 0, GIF_TAG_PACKED, 1));
  dma_packet_append(&graph_packet, 0x0E);
  dma_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 0, 1));
  dma_packet_append(&graph_packet, GIF_REG_TEST_1);
  dma_packet_append(&graph_packet, GIF_SET_PRIM(6, 0, 0, 0, 0, 0, 0, 0, 0));
  dma_packet_append(&graph_packet, GIF_REG_PRIM);
  dma_packet_append(&graph_packet, GIF_SET_RGBAQ(red, green, blue, alpha, 0x3F800000));
  dma_packet_append(&graph_packet, GIF_REG_RGBAQ);
  dma_packet_append(&graph_packet, GIF_SET_XYZ(0, 0, 0));
  dma_packet_append(&graph_packet, GIF_REG_XYZ2);
  dma_packet_append(&graph_packet, GIF_SET_XYZ(65535, 65535, 0));
  dma_packet_append(&graph_packet, GIF_REG_XYZ2);
  dma_packet_append(&graph_packet, GIF_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2));
  dma_packet_append(&graph_packet, GIF_REG_TEST_1);
  dma_packet_send(&graph_packet, DMA_CHANNEL_GIF);

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH VRAM FUNCTIONS //
 //////////////////////////

 int graph_vram_read(int address, int width, int height, int bpp, void *data, int data_size) {

  // NOT READY YET

  // End function.
  return 0;

 }

 int graph_vram_write(int address, int width, int height, int bpp, void *data, int data_size) {

  // Write the data into vram.
  dma_packet_clear(&graph_packet);
  dma_packet_append(&graph_packet, DMA_SET_TAG(6, 0, DMA_TAG_CNT, 0, 0, 0));
  dma_packet_append(&graph_packet, 0x00);
  dma_packet_append(&graph_packet, GIF_SET_TAG(4, 1, 0, 0, GIF_TAG_PACKED, 1));
  dma_packet_append(&graph_packet, 0x0E);
  dma_packet_append(&graph_packet, GIF_SET_BITBLTBUF(0, 0, 0, address >> 8, width >> 6, bpp));
  dma_packet_append(&graph_packet, GIF_REG_BITBLTBUF);
  dma_packet_append(&graph_packet, GIF_SET_TRXPOS(0, 0, 0, 0, 0));
  dma_packet_append(&graph_packet, GIF_REG_TRXPOS);
  dma_packet_append(&graph_packet, GIF_SET_TRXREG(width, height));
  dma_packet_append(&graph_packet, GIF_REG_TRXREG);
  dma_packet_append(&graph_packet, GIF_SET_TRXDIR(0));
  dma_packet_append(&graph_packet, GIF_REG_TRXDIR);
  dma_packet_append(&graph_packet, GIF_SET_TAG(data_size >> 4, 1, 0, 0, GIF_TAG_IMAGE, 1));
  dma_packet_append(&graph_packet, 0x00);
  dma_packet_append(&graph_packet, DMA_SET_TAG(data_size >> 4, 0, DMA_TAG_REF, 0, (u32)data, 0));
  dma_packet_append(&graph_packet, 0x00);
  dma_packet_append(&graph_packet, DMA_SET_TAG(2, 0, DMA_TAG_END, 0, 0, 0));
  dma_packet_append(&graph_packet, 0x00);
  dma_packet_append(&graph_packet, GIF_SET_TAG(1, 1, 0, 0, GIF_TAG_PACKED, 1));
  dma_packet_append(&graph_packet, 0x0E);
  dma_packet_append(&graph_packet, 0x00);
  dma_packet_append(&graph_packet, GIF_REG_TEXFLUSH);
  dma_packet_send_chain(&graph_packet, DMA_CHANNEL_GIF);

  // End function.
  return 0;

 }

 //////////////////////////
 // GRAPH WAIT FUNCTIONS //
 //////////////////////////

 int graph_wait_finish(void) {

  // Enable the finish event.
  GS_REG_CSR |= GS_SET_CSR(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // Wait for the finish event.
  while (!(GS_REG_CSR & (GS_SET_CSR(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)))) { }

  // Disable the finish event.
  GS_REG_CSR &= ~GS_SET_CSR(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

  // End function.
  return 0;

 }

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
