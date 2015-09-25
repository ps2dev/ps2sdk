/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE UGLY DEBUG ON SCREEN
*/

#include <stdio.h>
#include <tamtypes.h>
#include <sifcmd.h>
#include <kernel.h>
#include <osd_config.h>
#include <stdarg.h>
#include <debug.h>

/* baseado nas libs do Duke... */

static short int X = 0, Y = 0;
static short int MX=80, MY=40;
static u32 bgcolor = 0;

struct t_setupscr
{
   u64	   dd0[6];
   u32	   dw0[2];
   u64	   dd1[1];
   u16	   dh[4];
   u64	   dd2[21];
};

struct t_setupchar
{
   u64	   dd0[4];
   u32	   dw0[1];
   u16	   x,y;
   u64	   dd1[1];
   u32	   dw1[2];
   u64	   dd2[5];
};

// from gsKit
static int debug_detect_signal()
{
   char romname[14];
   GetRomName(romname);
   return((romname[4] == 'E') ? 1 : 0);
}

static void Init_GS( int a, int b, int c)
{
   *(vu64 *)0x12001000 = 0x200;
   GsPutIMR(0xff00);
   SetGsCrt(a, b, c);
}

static void SetVideoMode(void)
{
  unsigned dma_addr;
  unsigned val1;
  unsigned val2;
  unsigned val3;
  unsigned val4;
  unsigned val4_lo;

/*	DISPLAY1	0x0983227c001bf9ff
		DX = 0x27C (636)
		DY = 0x032 (50)

		MAGH = 0x03 (4x)
		MAGV = 0x01 (2x)

		DW = 0x9FF (2560)
		DH = 0x1BF (447)

	SMODE2 		1 (Interlaced, field mode)	*/

  asm volatile ("        .set push               \n"
                "        .set noreorder          \n"
                "        lui     %4, 0x001b      \n"
                "        lui     %5, 0x0983      \n"
                "        lui     %0, 0x1200      \n"
                "        li      %2, 1           \n"
                "        ori     %4, %4, 0xf9ff  \n"
                "        ori     %5, %5, 0x227c  \n"
                "        li      %1, 0xff62      \n"
                "        dsll32  %4, %4, 0       \n"
                "        li      %3, 0x1400      \n"
                "        sd      %1, 0(%0)       \n"
                "        or      %4, %4, %5      \n"
                "        sd      %2, 0x20(%0)    \n"
                "        sd      %3, 0x90(%0)    \n"
                "        sd      %4, 0xa0(%0)    \n"
                "        .set pop                \n"
                : "=&r" (dma_addr), "=&r" (val1), "=&r" (val2),
                "=&r" (val3), "=&r" (val4), "=&r" (val4_lo) );
}

static inline void Dma02Wait(void)
{
  unsigned dma_addr;
  unsigned status;

  asm volatile ("        .set push               \n"
                "        .set noreorder          \n"
                "        lui   %0, 0x1001        \n"
                "        lw    %1, -0x6000(%0)   \n"
                "1:      andi  %1, %1, 0x100     \n"
		"        nop                     \n"
		"        nop                     \n"
		"        nop                     \n"
		"        nop                     \n"
                "        bnel  %1, $0, 1b        \n"
                "        lw    %1, -0x6000(%0)   \n"
                "        .set pop                \n"
                : "=&r" (dma_addr), "=&r" (status) );
}

static void DmaReset(void)
{
  unsigned dma_addr;
  unsigned temp;

  asm volatile ("        .set push               \n"
                "        .set noreorder          \n"
                "        lui   %0, 0x1001        \n"
                "        sw    $0, -0x5f80(%0)   \n"
                "        sw    $0, -0x5000(%0)   \n"
                "        sw    $0, -0x5fd0(%0)   \n"
                "        sw    $0, -0x5ff0(%0)   \n"
                "        sw    $0, -0x5fb0(%0)   \n"
                "        sw    $0, -0x5fc0(%0)   \n"
                "        lui   %1, 0             \n"
                "        ori   %1, %1, 0xff1f    \n"
                "        sw    %1, -0x1ff0(%0)   \n"
                "        lw    %1, -0x1ff0(%0)   \n"
                "        andi  %1, %1, 0xff1f    \n"
                "        sw    %1, -0x1ff0(%0)   \n"
                "        sw    $0, -0x2000(%0)   \n"
                "        sw    $0, -0x1fe0(%0)   \n"
                "        sw    $0, -0x1fd0(%0)   \n"
                "        sw    $0, -0x1fb0(%0)   \n"
                "        sw    $0, -0x1fc0(%0)   \n"
                "        lw    %1, -0x2000(%0)   \n"
                "        ori   %1, %1, 1         \n"
                "        sw    %1, -0x2000(%0)   \n"
                "        .set pop                \n"
                : "=&r" (dma_addr), "=&r" (temp) );
}

/*
 * addr is the address of the data to be transfered.  addr must be 16
 * byte aligned.
 *
 * size is the size (in 16 byte quads) of the data to be transfered.
 */

static inline void progdma( void *addr, int size)
{
  unsigned dma_addr;
  unsigned temp;

  asm volatile ("        .set push               \n"
                "        .set noreorder          \n"
                "        lui   %0, 0x1001        \n"
                "        sw    %3, -0x5fe0(%0)   \n"
                "        sw    %2, -0x5ff0(%0)   \n"
                "        li    %1, 0x101         \n"
                "        sw    %1, -0x6000(%0)   \n"
                "        .set pop                \n"
                : "=&r" (dma_addr), "=&r" (temp)
                : "r" (addr), "r" (size) );
}

void scr_setbgcolor(u32 color)
{
	bgcolor = color;
}

void init_scr(void)
{
  static struct t_setupscr setupscr __attribute__ (( aligned (16) )) = {
	{ 0x100000000000800E, 0xE, 0xA0000, 0x4C, 0x8C, 0x4E },
	{ 27648, 30976 },
	{ 0x18 },
	{ 0, 639, 0, 223 },
	{ 0x40, 1, 0x1a, 1, 0x46, 0, 0x45, 0x70000,
	0x47, 0x30000, 0x47, 6, 0, 0x3F80000000000000, 1, 0x79006C00, 5,
	0x87009400, 5, 0x70000, 0x47 }
   };

   X = Y = 0;
   DmaReset();

   Init_GS( 1, debug_detect_signal() == 1 ? 3 : 2, 1);

   SetVideoMode();
   Dma02Wait();
   progdma( &setupscr, 15);
}

extern u8 msx[];

void
_putchar( int x, int y, u32 color, u8 ch)
{
   static struct t_setupchar setupchar __attribute__((aligned(16))) = {
	{ 0x1000000000000004, 0xE, 0xA000000000000, 0x50 },
	{ 0 },
	100, 100,
	{ 0x51 },
	{ 8, 8 },
	{ 0x52, 0, 0x53, 0x800000000000010, 0}
   };
   /* charmap must be aligned to a 64-bye boundary.  */
   static u32	charmap[64] __attribute__((aligned(16)));
   int 	i,j, l;
   u8	*font;
   u32  pixel;

   ((struct t_setupchar*)UNCACHED_SEG(&setupchar))->x = x;
   ((struct t_setupchar*)UNCACHED_SEG(&setupchar))->y = y;

   progdma(&setupchar, 6);

   font = &msx[ (int)ch * 8];
   for (i=l=0; i < 8; i++, l+= 8, font++)
      for (j=0; j < 8; j++)
          {
          if ((*font & (128 >> j)))
              pixel = color;
          else
              pixel = bgcolor;
          *(u32*)UNCACHED_SEG(&charmap[ l + j]) = pixel;
          }

   Dma02Wait();

   progdma(charmap, (8*8*4) / 16);
   Dma02Wait();
}

static void  clear_line( int Y)
{
   int i;
   for (i=0; i < MX; i++)
    _putchar( i*8 , Y * 8, bgcolor, ' ');
}

void scr_printf(const char *format, ...)
{
   va_list	opt;
   u8		buff[2048], c;
   int		i, bufsz, j;


   va_start(opt, format);
   bufsz = vsnprintf( buff, sizeof(buff), format, opt);

   for (i = 0; i < bufsz; i++)
       {
       c = buff[i];
       switch (c)
          {
          case		'\n':
                             X = 0;
                             Y ++;
                             if (Y == MY)
                                 Y = 0;
                             clear_line(Y);
                             break;
          case      '\t':
                             for (j = 0; j < 5; j++) {
                             	_putchar( X*7 , Y * 8, 0xffffff, ' ');
                             	X++;
                             }
                             break;
          default:
             		     _putchar( X*7 , Y * 8, 0xffffff, c);
                             X++;
                             if (X == MX)
                                {
                                X = 0;
                                Y++;
                                if (Y == MY)
                                   Y = 0;
                                clear_line(Y);
                                }
          }
       }
    _putchar( X*7 , Y * 8, 0xffffff, 219);
    va_end(opt);
}

void scr_setXY(int x, int y)
{
	if( x<MX && x>=0 ) X=x;
	if( y<MY && y>=0 ) Y=y;
}

int scr_getX()
{
	return X;
}

int scr_getY()
{
	return Y;
}

void scr_clear()
{
	int y;
	for(y=0;y<MY;y++)
		clear_line(y);
	scr_setXY(0,0);
}
