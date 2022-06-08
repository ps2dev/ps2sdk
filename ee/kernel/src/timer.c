/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Some routines to do some timer work
 */

#include <timer.h>
#include <kernel.h>

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
s32  __time_intr_overflow_id = -1;
u64  __time_intr_overflow_count = 0;
#else
extern s32  __time_intr_overflow_id;
extern u64  __time_intr_overflow_count;
#endif

#ifdef F_StartTimerSystemTime
static s32 intrOverflow(int ca)
{
    (void)ca;

    __time_intr_overflow_count++;

    // A write to the overflow flag will clear the overflow flag
    // ---------------------------------------------------------
    *T_MODE |= (1 << 11);

    ExitHandler();
    return -1;
}

/** Init the timer
 *
 *
 * Start the interruption handler for getting the system time
 * This function should be called at beginning of your program
 */
void StartTimerSystemTime(void)
{
    u32 oldintr;

    oldintr = DIntr();

    *T_MODE = 0x0000; // Disable T_MODE
    if (__time_intr_overflow_id == -1)
    {
        __time_intr_overflow_id = AddIntcHandler(INTC_TIM, intrOverflow, 0);
        EnableIntc(INTC_TIM);
    }

    __time_intr_overflow_count = 0;

    // Initialize the timer registers
    // CLKS: 0x02 - 1/256 of the BUSCLK (0x01 is 1/16th)
    //  CUE: 0x01 - Start/Restart the counting
    // OVFE: 0x01 - An interrupt is generated when an overflow occurs
    *T_COUNT = 0;
    *T_MODE = Tn_MODE(0x02, 0, 0, 0, 0, 0x01, 0, 0x01, 0, 0);

    if(oldintr) { EIntr(); }
}
#endif

#ifdef F_StopTimerSystemTime
/** Finish the timer
 *
 *
 * Stops the interruption handler for getting the  system time
 * This function should be called when ending your program
 */
void StopTimerSystemTime(void)
{
    u32 oldintr;
    oldintr = DIntr();

    *T_MODE = 0x0000; // Stop the timer

    if (__time_intr_overflow_id >= 0)
    {
        DisableIntc(INTC_TIM);
        RemoveIntcHandler(INTC_TIM, __time_intr_overflow_id);
        __time_intr_overflow_id = -1;
    }

    __time_intr_overflow_count = 0;
    if(oldintr) { EIntr(); }
}
#endif

#ifdef F_iGetTimerSystemTime
 /** Get current System Time
 *
 * This function gets the current system time
 * Can be called from an interrupt handler
 * Cannot be called from a thread
 *
 * @return current system time in BUSCLK units, 0 in case the InitTimer wasn't called at init.
 */
u64 iGetTimerSystemTime(void)
{
    u64 t;

    // Tn_COUNT is 16 bit precision. Therefore, each __time_intr_overflow_count is 65536 ticks
    t = *T_COUNT + (__time_intr_overflow_count << 16);

    // check if the overflow was produced but the interruption callback isn't be executed yet
    if (((*T_MODE) & (1 << 11)) != 0) {
        t += (1 << 16);
    }

    return (t << 8); // Because counter clock config was 0x02 (which means BUSCLK/256)
}
#endif


#ifdef F_GetTimerSystemTime
/** Get current System Time
 *
 * This function gets the current system time
 * Can be called from an interrupt handler
 * Can be called from a thread
 * Multithread safe
 *
 * @return current system time in BUSCLK units, 0 in case the InitTimer wasn't called at init.
 */
u64 GetTimerSystemTime(void) {
    u64 t;
    u32 oldintr = DIntr();
    t = iGetTimerSystemTime();
    if (oldintr)
        EIntr();

    return t;
}
#endif

#ifdef F_cpu_ticks
u32 cpu_ticks(void)
{
    u32 out;

    asm("mfc0\t%0, $9\n"
        : "=r"(out));
    return out;
}
#endif
