/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2006-2007 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# Simplest example of MPEG bitstream deconing/display
# Note: the program expects raw mpeg video data as it does not perform
#       any demuxing. Only 4:2:0 colorspace is supported by the MPEG
#       decoder. Scalable extensions are not supported either.
#       Test bitstreams can be obtained for free at ftp://ftp.tek.com/tv/test/streams/Element/MPEG-Video-Conformance/main-profile/
#       This program was only tested on SCPH-3004R PAL console.
#       For real life usage of 'libmpeg' refer SMS project.
*/
#include "libmpeg.h"

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>
#include <gs_gp.h>

#include <kernel.h>
#include <dma.h>
#include <draw.h>
#include <fileio.h>
#include <malloc.h>
#include <graph.h>
#include <graph_vram.h>
#include <packet.h>
#include <stdio.h>

#define MPEG_BITSTREAM_FILE "host:test.bin"
/* get the whole file (or first 24MB) into memory for simplicity */
#define MAX_SIZE            ( 1024 * 1024 * 24 )

typedef struct InitCBParam {

 MPEGSequenceInfo* m_pInfo;
 void*             m_pData;
 PACKET            m_XFerPck;
 PACKET            m_DrawPck;
 int               m_TexAddr;

} InitCBParam;

static unsigned char* s_pMPEGData;
static unsigned char* s_pTransferPtr;
static unsigned int   s_MPEGDataSize;

static int   SetDMA ( void* );
static void* InitCB ( void*, MPEGSequenceInfo* );

int main ( void ) {
/* read file (or part of it ) into memory */
 PACKET      *lPck = malloc(sizeof(PACKET));
 QWORD       *q;
 FRAMEBUFFER frame;
 ZBUFFER z;
 InitCBParam lInfo;
 int         lFD = fioOpen ( MPEG_BITSTREAM_FILE, O_RDONLY );
 long        lSize;
 long        lPTS, lCurPTS;

 frame.width = 640;
 frame.height = 512;
 frame.mask = 0;
 frame.psm = GS_PSM_32;
 frame.address = graph_vram_allocate(frame.width,frame.height, frame.psm, GRAPH_ALIGN_PAGE);

 z.enable = 0;
 z.mask = 0;
 z.method = 0;
 z.zsm = 0;
 z.address = 0;

 packet_allocate(lPck, 100, 0, 0);

 if ( lFD < 0 ) {
  printf ( "test_mpeg: could not open '%s'\n", MPEG_BITSTREAM_FILE );
  goto end;
 }  /* end if */

 lSize = fioLseek ( lFD, 0, SEEK_END );
 fioLseek ( lFD, 0, SEEK_SET );

 if ( lSize <= 0 ) {
  printf ( "test_mpeg: could not obtain file size (%ld)\n", lSize );
  goto end;
 }  /* end if */

 s_pMPEGData = ( unsigned char* )malloc ( lSize = lSize > MAX_SIZE ? MAX_SIZE : lSize );

 if ( !s_pMPEGData ) {
  printf ( "test_mpeg: could not allocate enough memory (%ld)\n", lSize );
  goto end;
 }  /* end if */

 if (  fioRead (
        lFD, s_pTransferPtr = s_pMPEGData, s_MPEGDataSize = lSize
       ) != lSize
 ) {
  printf ( "test_mpeg: could not read file\n" );
  goto end;
 }  /* end if */

 fioClose ( lFD );

/* initialize DMAC (I have no idea what this code does as */
/* I'm not quite familiar with ps2sdk)                    */
 dma_channel_initialize ( DMA_CHANNEL_toIPU, NULL, 0 );
 dma_channel_initialize ( DMA_CHANNEL_GIF,   NULL, 0 );
 dma_channel_fast_waits( DMA_CHANNEL_GIF );

/* initialize graphics synthesizer */
 graph_initialize(0,640,512,GS_PSM_32,0,0);

/* setup texture buffer address just after the framebuffer */
 lInfo.m_TexAddr = graph_vram_allocate(0,0,GS_PSM_32,GRAPH_ALIGN_BLOCK);

 q = lPck->data;
 q = draw_setup_environment(q,0,&frame,&z);

/* clear screen */
 q = draw_clear(q,0,0,0,640.0f,512.0f,0,0,0);

 dma_channel_send_normal(DMA_CHANNEL_GIF, lPck->data, q - lPck->data, 0, 0);

/* now it's time to initialize MPEG decoder (though it can be   */
/* initialized any time). Just make sure that DMA transfers     */
/* to and from IPU (and DRAM -> SPR) are not active, otherwise  */
/* unpredicted things will happen. Initialization code is also  */
/* allocating some memory using 'memalign' function and no      */
/* check is performed whether the allocation was successful or  */
/* not, so, before calling this make sure that at least WxHx4x3 */
/* bytes are avaliable for dynamic allocation (possibly using   */
/* ps2_sbrk ( 0 ) call) where W and H are picture dimensions in */
/* units of pixels.                                             */
 MPEG_Initialize ( SetDMA, NULL, InitCB, &lInfo, &lCurPTS );
/* during decoding scratchpad RAM from address 0x0000 to 0x3C00 */
/* is used by the decoder.                                      */
/* let's go                                                     */
 while ( 1 ) {
/* try decode picture into "lInfo.m_pData" area. It's allowed     */
/* to supply different area each time, just make sure that        */
/* there're no conflicts with data cache, as decoder doesn't do   */
/* anything to synchronize/flush/invalidate data cache.           */
/* RGB -> YUV colorspace conversion is pefromed automatically     */
/* using interrupt hahdler/semaphore, so, multithreaded           */
/* application can benefit from it. Usage of IPU and DMA channels */
/* to/from IPU and DRAM -> SPR is strictly forbidden during       */
/* decoding :).                                                   */
  if (  !MPEG_Picture ( lInfo.m_pData, &lPTS )  ) {
/* MPEG_Picture returns nonzero if the picture was successfully */
/* decoded. Zero return means one of the following:             */
/* - end of stream was detected (SetDMA function returned zero) */
/* - MPEG sequence end code (0x000001B7) was detected           */
/* this test just finishes in both cases                        */
   if ( lInfo.m_pInfo -> m_fEOF  ) break;
/* ...instead of 'break' we can continue to the next sequence...*/
/* ...but I'm too lazy to handle second call of 'InitCB' :D     */
   else break;
  }  /* end if */
/* now transfer decoded picture data into texture area of GS RAM */
  dma_wait_fast();
  dma_channel_send_chain( DMA_CHANNEL_GIF, lInfo.m_XFerPck.data, lInfo.m_XFerPck.qwc, 0, 0);
/* wait for vsync 2 times (we have interlaced frame mode)  */
  graph_wait_vsync ();
  graph_wait_vsync ();
/* no need to wait for DMA transfer completion since vsyncs above */
/* have enough lattency...                                        */
/* ...and finally draw decoded picture...                         */
  dma_channel_send_normal( DMA_CHANNEL_GIF, lInfo.m_DrawPck.data, lInfo.m_DrawPck.qwc, 0, 0);
/* ...and go back for the next one */
 }  /* end while */
/* free memory and other resources */
 MPEG_Destroy ();

end:
 printf ( "test_mpeg: test finished\n" );
 return SleepThread (), 0;

}  /* end main */
/* This gets called by MPEG decoder to get data to decode.  */
/* It performs normal DMA data transfer to IPU and returns  */
/* nozero to indicate that data have been sent. Zero return */
/* indicates end-of-data condition, Amount of data per      */
/* transfer doesn't really matter, but it must be less than */
/* 1MB minus 16. Sample function uses 2048 bytes blocks.    */
/* Don't use source chain transfer as it will lead to       */
/* unpredictable results.                                   */
static int SetDMA ( void* apUserData ) {

 if ( s_pTransferPtr - s_pMPEGData >= s_MPEGDataSize ) return 0;

 dma_channel_wait(DMA_CHANNEL_toIPU,0);
 dma_channel_send_normal(
  DMA_CHANNEL_toIPU, s_pTransferPtr, 2048>>4, 0, 0
 );
 s_pTransferPtr += 2048;

 return 1;

}  /* end SetDMA */

/* This gets called when sequence start header is detected in the     */
/* input bitstream. <apInfo> is filled by the decoder and callback    */
/* function initializes display process and other required stuff      */
/* based upon values provided in <apInfo> stucture. It should return  */
/* pointer to the data area where decoded picture (RGBA32, 16x16      */
/* blocks) will be stored. Pointer is supposed to be 16 byte aligned. */
/* It can be called several times (depending on number of video       */
/* sequences inside a bitstream). It is allowed to return the same    */
/* pointer each time, but the data area should be large enough to     */
/* accomodate the whole picture. <apParam> is just a user supplied    */
/* data (anything).  */
static void* InitCB ( void* apParam, MPEGSequenceInfo* apInfo ) {

 int          lDataSize = apInfo -> m_Width * apInfo -> m_Height * 4;
 char*        retVal    = ( char* )malloc ( lDataSize );
 InitCBParam* lpParam   = ( InitCBParam* )apParam;
 int          lMBW      = ( apInfo -> m_Width  ) >> 4;
 int          lMBH      = ( apInfo -> m_Height ) >> 4;
 int          lTBW      = ( apInfo -> m_Width + 63 ) >> 6;
 int          lTW       = draw_log2 ( apInfo -> m_Width  );
 int          lTH       = draw_log2 ( apInfo -> m_Height );
 int          lX, lY;
 char*        lpImg;
 QWORD*       q;

 lpParam -> m_TexAddr >>= 6;

 lpParam -> m_pData = lpImg = retVal;
 lpParam -> m_pInfo = apInfo;
 SyncDCache ( retVal, retVal + lDataSize );
/* This initializes picture transfer packet.      */
/* Decoded picture is a sequence of 16x16 pixels  */
/* 'subpictures' (macroblocks) and DMA controller */
/* will transfer them all at once using source    */
/* chain transfer mode.                           */
 packet_allocate(&lpParam -> m_XFerPck,(10 + 12 * lMBW * lMBH )>>1,0,0);

 q = lpParam-> m_XFerPck.data;

 DMATAG_CNT(q, 3, 0, 0, 0);
 q++;
 PACK_GIFTAG(q,GIF_SET_TAG( 2, 0, 0, 0, 0, 1 ),GIF_REG_AD);
 q++;
 PACK_GIFTAG(q,GS_SET_TRXREG( 16, 16 ), GS_REG_TRXREG);
 q++;
 PACK_GIFTAG(q,GS_SET_BITBLTBUF( 0, 0, 0, lpParam -> m_TexAddr, lTBW, GS_PSM_32 ), GS_REG_BITBLTBUF);
 q++;

 for ( lY = 0; lY < apInfo -> m_Height; lY += 16 ) {
  for ( lX = 0; lX < apInfo -> m_Width; lX += 16, lpImg  += 1024 ) {
   DMATAG_CNT(q, 4, 0, 0, 0 );
   q++;
   PACK_GIFTAG(q,GIF_SET_TAG( 2, 0, 0, 0, 0, 1 ), GIF_REG_AD );
   q++;
   PACK_GIFTAG(q,GS_SET_TRXPOS( 0, 0, lX, lY, 0 ), GS_REG_TRXPOS );
   q++;
   PACK_GIFTAG(q,GS_SET_TRXDIR( 0 ), GS_REG_TRXDIR );
   q++;
   PACK_GIFTAG(q,GIF_SET_TAG( 64, 1, 0, 0, 2, 0),0);
   q++;
   DMATAG_REF(q, 64, ( unsigned )lpImg, 0, 0, 0);
   q++;
  }  /* end for */
 }  /* end for */

 //DMATAG_END(q,0,0,0,0);
 //q++;

 lpParam-> m_XFerPck.qwc = q - lpParam-> m_XFerPck.data;

/* This initializes picture drawing packet. Just textrured sprite */
/* that occupies the whole screen (no aspect ratio is taken into  */
/* account for simplicity.                                        */
 packet_allocate(&lpParam -> m_DrawPck,7,0,0);
 q = lpParam -> m_DrawPck.data;
 PACK_GIFTAG(q, GIF_SET_TAG( 6, 1, 0, 0, 0, 1 ), GIF_REG_AD );
 q++;
 PACK_GIFTAG(q, GS_SET_TEX0( lpParam -> m_TexAddr, lTBW, GS_PSM_32, lTW, lTH, 1, 1, 0, 0, 0, 0, 0 ), GS_REG_TEX0_1 );
 q++;
 PACK_GIFTAG(q, GS_SET_PRIM( 6, 0, 1, 0, 0, 0, 1, 0, 0 ), GS_REG_PRIM );
 q++;
 PACK_GIFTAG(q, GS_SET_UV( 0, 0 ), GS_REG_UV  );
 q++;
 PACK_GIFTAG(q, GS_SET_XYZ( 0, 0, 0 ), GS_REG_XYZ2 );
 q++;
 PACK_GIFTAG(q, GS_SET_UV( apInfo -> m_Width << 4, apInfo -> m_Height << 4 ), GS_REG_UV );
 q++;
 PACK_GIFTAG(q, GS_SET_XYZ(  640 << 4, 512 << 4, 0 ), GS_REG_XYZ2 );
 q++;

 lpParam -> m_DrawPck.qwc = q - lpParam -> m_DrawPck.data;

 return retVal;

}  /* end InitCB */
