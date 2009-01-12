/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Standard libc time functions
*/

#include <time.h>
#include <kernel.h>
#include <timer.h>

static int      s_intrOverflowID = -1; 
static clock_t  s_intrOverflowCount = 0; 

#ifdef TIME_USE_T0
#define INTC_TIM       kINTC_TIMER0
#define T_COUNT        T0_COUNT
#define T_MODE         T0_MODE
#define T_COMP         T0_COMP
#else
#define INTC_TIM       kINTC_TIMER1
#define T_COUNT        T1_COUNT
#define T_MODE         T1_MODE
#define T_COMP         T1_COMP
#endif

static int intrOverflow(int ca) 
{ 
   s_intrOverflowCount++; 

   // A write to the overflow flag will clear the overflow flag 
   // --------------------------------------------------------- 
   *T_MODE |= (1 << 11); 

   return -1; 
} 

void _ps2sdk_time_init(void) 
{ 
   *T_MODE = 0x0000; // Disable T_MODE 

   if (s_intrOverflowID == -1) 
   {
       s_intrOverflowID = AddIntcHandler(kINTC_TIMER1, intrOverflow, 0); 
       EnableIntc(kINTC_TIMER1); 
   }

   // Initialize the timer registers 
   // CLKS: 0x02 - 1/256 of the BUSCLK (0x01 is 1/16th) 
   //  CUE: 0x01 - Start/Restart the counting 
   // OVFE: 0x01 - An interrupt is generated when an overflow occurs 
   *T_COUNT = 0; 
   *T_MODE = Tn_MODE(0x02, 0, 0, 0, 0, 0x01, 0, 0x01, 0, 0); 

   s_intrOverflowCount = 0; 
} 

void _ps2sdk_time_deinit(void) 
{ 
   *T_MODE = 0x0000; // Stop the timer 

   if (s_intrOverflowID >= 0) 
   { 
      DisableIntc(kINTC_TIMER1); 
      RemoveIntcHandler(kINTC_TIMER1, s_intrOverflowID); 
      s_intrOverflowID = -1; 
   } 

   s_intrOverflowCount = 0; 
} 

clock_t clock(void) 
{
   u64         t; 

   // Tn_COUNT is 16 bit precision. Therefore, each s_intrOverflowCount is 65536 ticks 
   t = *T_COUNT + (s_intrOverflowCount << 16); 

   return t; 
}

time_t time(time_t *t)
{
	if (t != 0) {
		*t = (time_t)-1;
	}

	return (time_t)-1;
}
