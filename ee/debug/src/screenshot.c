#include "screenshot.h"
#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <io_common.h>

///////////////////////////////////////////////////////////////////////////////
//
// These macros are kept local so the screenshot funcion can work without
// interfering with other macros
//

#define PS2SS_GIF_AD    0x0e

#define PS2SS_GIFTAG(NLOOP,EOP,PRE,PRIM,FLG,NREG) \
    ((unsigned long)(NLOOP) << 0)   | \
    ((unsigned long)(EOP)   << 15)  | \
    ((unsigned long)(PRE)   << 46)  | \
    ((unsigned long)(PRIM)  << 47)  | \
    ((unsigned long)(FLG)   << 58)  | \
    ((unsigned long)(NREG)  << 60)

#define PS2SS_GSBITBLTBUF_SET(sbp, sbw, spsm, dbp, dbw, dpsm) \
  ((u64)(sbp)         | ((u64)(sbw) << 16) | \
  ((u64)(spsm) << 24) | ((u64)(dbp) << 32) | \
  ((u64)(dbw) << 48)  | ((u64)(dpsm) << 56))

#define PS2SS_GSTRXREG_SET(rrw, rrh) \
  ((u64)(rrw) | ((u64)(rrh) << 32))

#define PS2SS_GSTRXPOS_SET(ssax, ssay, dsax, dsay, dir) \
  ((u64)(ssax)        | ((u64)(ssay) << 16) | \
  ((u64)(dsax) << 32) | ((u64)(dsay) << 48) | \
  ((u64)(dir) << 59))

#define PS2SS_GSTRXDIR_SET(xdr) ((u64)(xdr))

#define PS2SS_GSBITBLTBUF         0x50
#define PS2SS_GSFINISH            0x61
#define PS2SS_GSTRXPOS            0x51
#define PS2SS_GSTRXREG            0x52
#define PS2SS_GSTRXDIR            0x53

#define PS2SS_GSPSMCT32           0
#define PS2SS_GSPSMCT24           1
#define PS2SS_GSPSMCT16           2

#define PS2SS_D1_CHCR             ((volatile unsigned int *)(0x10009000))
#define PS2SS_D1_MADR             ((volatile unsigned int *)(0x10009010))
#define PS2SS_D1_QWC              ((volatile unsigned int *)(0x10009020))
#define PS2SS_D1_TADR             ((volatile unsigned int *)(0x10009030))
#define PS2SS_D1_ASR0             ((volatile unsigned int *)(0x10009040))
#define PS2SS_D1_ASR1             ((volatile unsigned int *)(0x10009050))

#define PS2SS_CSR_FINISH          (1 << 1)
#define PS2SS_GS_CSR              ((volatile u64 *)(0x12001000))
#define PS2SS_GS_BUSDIR           ((volatile u64 *)(0x12001040))

#define PS2SS_VIF1_STAT           ((volatile u32 *)(0x10003c00))
#define PS2SS_VIF1_STAT_FDR       ( 1<< 23)
#define PS2SS_VIF1_MSKPATH3(mask) ((u32)(mask) | ((u32)0x06 << 24))
#define PS2SS_VIF1_NOP            0
#define PS2SS_VIF1_FLUSHA         (((u32)0x13 << 24))
#define PS2SS_VIF1_DIRECT(count)  ((u32)(count) | ((u32)(0x50) << 24))
#define PS2SS_VIF1_FIFO           ((volatile u128 *)(0x10005000))

////////////////////////////////////////////////////////////////////////////////
// Internal convfunctions

void ps2_screenshot_16to32_buffer( void* pTemp, u32 w, u32 h );
void ps2_screenshot_16to32_line( void* pTemp, u32 w );

///////////////////////////////////////////////////////////////////////////////
// ps2_screenshot_file

int ps2_screenshot_file( const char* pFilename,unsigned int VramAdress,
                         unsigned int Width, unsigned int Height, unsigned int Psm )
{
  s32 file_handle;
  u32 y;
  static u32 in_buffer[1024*4];  // max 1024*32bit for a line, should be ok
  static u32 out_buffer[1024*4]; // max 1024*32bit for a line, should be ok

  unsigned char header[18] = 
  {
     0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, // Width 
     0, 0, // Heigth
    32, 8
  };

  s16* p_header = (s16*)&header;

  file_handle = fioOpen( pFilename, O_CREAT|O_WRONLY );

  // make sure we could open the file for output

  if( file_handle < 0 )
    return 0;

  /////////////////////////////////////////////////////////////////////////////
  // setup tga header

  p_header[6] = (s16)Width;
  p_header[7] = (s16)Height;

  fioWrite( file_handle, (void*)&header, 18 );

  /////////////////////////////////////////////////////////////////////////////
  // Check if we have a tempbuffer, if we do we use it 

  for( y = 0; y < Height; y++ )
  {
    u32* p_out = (u32*)&out_buffer;
    ps2_screenshot( in_buffer, VramAdress, 0, (Height-1)-y, Width, 1, Psm );

    if( Psm == PS2SS_GSPSMCT16 )
    {
      u32 x;
      u16* p_in  = (u16*)&in_buffer;
        
      for( x = 0; x < Width; x++ )
      {
	 u32 r = (p_in[x] & 31) << 3;
	 u32 g = ((p_in[x] >> 5) & 31) << 3;
	 u32 b = ((p_in[x] >> 10) & 31) << 3;
         p_out[x] = (0xff<<24)|(r<<16)|(g<<8)|b;
      }
    }
    else
    if( Psm == PS2SS_GSPSMCT24 )
    {
      u32 x;
      u8* p_in  = (u8*)&in_buffer;
        
      for( x = 0; x < Width; x++ )
      {
        u8 r =  *p_in++;
        u8 g =  *p_in++;
        u8 b =  *p_in++;
        p_out[x] = (0xff<<24)|(r<<16)|(g<<8)|b;
      }
    }
    else
    {
       u8 *p_in = (u8 *) &in_buffer;
       u32 x;

       for(x = 0; x < Width; x++)
       {
	  u8 r = *p_in++;
	  u8 g = *p_in++;
	  u8 b = *p_in++;
	  u8 a = *p_in++;
	  p_out[x] = (a << 24) | (r << 16) | (g << 8) | b;
       }
    }

    fioWrite( file_handle, p_out, Width*4 );
  }

  fioClose( file_handle );

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ps2_screenshot
//
// NOTE: This code was based on work by sparky
// Downloads vram back to host
//
// pDest      - Temporary storeage for the screen (allocated by user)
// VramAdress - pointer to where in vram to transfer from (wordaddress/64)
// Width      - Width of Screen 
// Height     - Width of Screen 
// Psm        - Pixelformat of screen
//

int ps2_screenshot( void* pDest, unsigned int VramAdress, unsigned int x, 
                    unsigned int y, unsigned int Width, unsigned int Height, 
                    unsigned int Psm )
{
  static u32 enable_path3[4] ALIGNED(16) = 
  { 
    PS2SS_VIF1_MSKPATH3(0), PS2SS_VIF1_NOP, PS2SS_VIF1_NOP, PS2SS_VIF1_NOP, 
  }; 

  u32  dma_chain[20*2] ALIGNED(16);
  u32* p_dma32 = (u32*)&dma_chain;
  u64* p_dma64 = (u64*)( p_dma32 + 4 );
  u32  uQSize;
  u32  prev_imr;
  u32  prev_chcr; 

  /////////////////////////////////////////////////////////////////////////////
  // Calc size depending on Psm

  if( Psm == PS2SS_GSPSMCT16 )
    uQSize = ((Width*Height*2)/16);
  else if( Psm == PS2SS_GSPSMCT24 )
    uQSize = ((Width*Height*3)/16);
  else
    uQSize = (Width*Height*4)/16;

  /////////////////////////////////////////////////////////////////////////////
  // Setup transfer texture back to memory 

  p_dma32[0] = PS2SS_VIF1_NOP; 
  p_dma32[1] = PS2SS_VIF1_MSKPATH3( 0x8000 ); 
  p_dma32[2] = PS2SS_VIF1_FLUSHA; 
  p_dma32[3] = PS2SS_VIF1_DIRECT( 6 ); 

  ////////////////////////////////////////////////////////////////////////////
  // Setup the blit

  p_dma64[0]  = PS2SS_GIFTAG(5, 1, 0, 0, 0, 1); // GIFTAG(NLOOP, EOP, PRE, PRIM, FLG, NREG) 
  p_dma64[1]  = PS2SS_GIF_AD; 

  p_dma64[2]  = PS2SS_GSBITBLTBUF_SET( VramAdress, Width/64, Psm, 0, 0, Psm ); 
  p_dma64[3]  = PS2SS_GSBITBLTBUF; 

  p_dma64[4]  = PS2SS_GSTRXPOS_SET( x, y, 0, 0, 0 ); // SSAX, SSAY, DSAX, DSAY, DIR 
  p_dma64[5]  = PS2SS_GSTRXPOS; 

  p_dma64[6]  = PS2SS_GSTRXREG_SET( Width, Height ); // RRW, RRh 
  p_dma64[7]  = PS2SS_GSTRXREG; 

  p_dma64[8]  = 0; 
  p_dma64[9]  = PS2SS_GSFINISH; 

  p_dma64[10] = PS2SS_GSTRXDIR_SET( 1 ); // XDIR 
  p_dma64[11] = PS2SS_GSTRXDIR; 

  prev_imr = GsPutIMR( GsGetIMR() | 0x0200 ); 
  prev_chcr = *PS2SS_D1_CHCR; 

  if( (*PS2SS_D1_CHCR & 0x0100) != 0 ) 
    return 0; 

  /////////////////////////////////////////////////////////////////////////////
  // set the FINISH event 

  *PS2SS_GS_CSR = PS2SS_CSR_FINISH; 

  /////////////////////////////////////////////////////////////////////////////
  // DMA from memory and start DMA transfer 

  FlushCache(0); 

  *PS2SS_D1_QWC  = 0x7; 
  *PS2SS_D1_MADR = (u32)p_dma32; 
  *PS2SS_D1_CHCR = 0x101; 

  asm __volatile__( " sync.l " ); 

  /////////////////////////////////////////////////////////////////////////////
  // check if DMA is complete (STR=0) 

  while( *PS2SS_D1_CHCR & 0x0100 ); 
  while( ( *PS2SS_GS_CSR & PS2SS_CSR_FINISH ) == 0 ); 

  /////////////////////////////////////////////////////////////////////////////
  // Wait for viffifo to become empty

  while( (*PS2SS_VIF1_STAT & (0x1f000000) ) );

  /////////////////////////////////////////////////////////////////////////////
  // Reverse busdir and transfer image to host

  *PS2SS_VIF1_STAT = PS2SS_VIF1_STAT_FDR; 
  *PS2SS_GS_BUSDIR = (u64)0x00000001; 

  FlushCache(0); 

  *PS2SS_D1_QWC  = uQSize; 
  *PS2SS_D1_MADR = (u32)pDest; 
  *PS2SS_D1_CHCR = 0x100; 

  asm __volatile__( " sync.l " ); 

  /////////////////////////////////////////////////////////////////////////////
  // check if DMA is complete (STR=0) 

  while ( *PS2SS_D1_CHCR & 0x0100 ); 
  *PS2SS_D1_CHCR = prev_chcr; 
  asm __volatile__( " sync.l " ); 
  *PS2SS_VIF1_STAT = 0; 
  *PS2SS_GS_BUSDIR = (u64)0; 

  /////////////////////////////////////////////////////////////////////////////
  // Put back prew imr and set finish event

  GsPutIMR( prev_imr ); 
  *PS2SS_GS_CSR = PS2SS_CSR_FINISH; 

  /////////////////////////////////////////////////////////////////////////////
  // Enable path3 again


  *PS2SS_VIF1_FIFO = *(u128*) enable_path3;

  return 1;
}

//////////////////////////////////////////////////////////////////////////////
// Converts to a buffer that fits tga

void ps2_screenshot_16to32_line( void* pTemp, u32 w )
{
}

//////////////////////////////////////////////////////////////////////////////
// Converts to a buffer that fits tga

void ps2_screenshot_16to32_buffer( void* pTemp, u32 w, u32 h )
{




}

