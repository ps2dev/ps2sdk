/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2005, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Standard libc time functions
 */

#include <time.h>
#include <kernel.h>
#include <timer.h>

extern int      __time_intr_overflow_id;
extern clock_t  __time_intr_overflow_count;

#ifdef TIME_USE_T0
#define INTC_TIM       INTC_TIM0
#define T_COUNT        T0_COUNT
#define T_MODE         T0_MODE
#define T_COMP         T0_COMP
#else
#define INTC_TIM       INTC_TIM1
#define T_COUNT        T1_COUNT
#define T_MODE         T1_MODE
#define T_COMP         T1_COMP
#endif

#ifdef F___time_internals
int      __time_intr_overflow_id = -1;
clock_t  __time_intr_overflow_count = 0;

static int intrOverflow(int ca)
{
   __time_intr_overflow_count++;

   // A write to the overflow flag will clear the overflow flag
   // ---------------------------------------------------------
   *T_MODE |= (1 << 11);

   ExitHandler();
   return -1;
}

void _ps2sdk_time_init(void)
{
   *T_MODE = 0x0000; // Disable T_MODE

   if (__time_intr_overflow_id == -1)
   {
       __time_intr_overflow_id = AddIntcHandler(INTC_TIM, intrOverflow, 0);
       EnableIntc(INTC_TIM);
   }

   // Initialize the timer registers
   // CLKS: 0x02 - 1/256 of the BUSCLK (0x01 is 1/16th)
   //  CUE: 0x01 - Start/Restart the counting
   // OVFE: 0x01 - An interrupt is generated when an overflow occurs
   *T_COUNT = 0;
   *T_MODE = Tn_MODE(0x02, 0, 0, 0, 0, 0x01, 0, 0x01, 0, 0);

   __time_intr_overflow_count = 0;
}

void _ps2sdk_time_deinit(void)
{
   *T_MODE = 0x0000; // Stop the timer

   if (__time_intr_overflow_id >= 0)
   {
      DisableIntc(INTC_TIM);
      RemoveIntcHandler(INTC_TIM, __time_intr_overflow_id);
      __time_intr_overflow_id = -1;
   }

   __time_intr_overflow_count = 0;
}
#endif

#ifdef F_ps2_clock
clock_t ps2_clock(void)
{
   u64         t;

   // Tn_COUNT is 16 bit precision. Therefore, each __time_intr_overflow_count is 65536 ticks
   t = *T_COUNT + (__time_intr_overflow_count << 16);

   return t;
}
#endif

