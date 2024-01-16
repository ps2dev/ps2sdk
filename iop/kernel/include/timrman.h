/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2005 linuzappz <linuzappz@hotmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Timer manager.
 */

#ifndef __TIMRMAN_H__
#define __TIMRMAN_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*	Documentation on the old module's SetTimerCompare() and SetTimerMode() functions is difficult because SCE had replaced them
        with a single function within later SDK versions: SetupHardTimer().

    source = counter input (see below).
    size = counter width (either 16 or 32).
    prescale = counter input scale (e.g. for a prescale of 256 with a SYSCLOCK (36.8MHz) source, 36864000/256=144 ticks per msec)

    Mode bits:
        Value	Bit(s)	Description
        0x0001	1	Enable gate
        0x0002	2	Gate count mode
        0x0004	3	Gate count mode
        0x0008	4	Reset on target
        0x0010	5	Interrupt on target
        0x0020	6	Interrupt on overflow
        0x0040	7	??? (Repeat?)
        0x0080	8	??? (Clears interrupt bit on assertion?)
        0x0100	9	"uses hblank on counters 1 and 3, and PSXCLOCK on counter 0"
        0x0200	10	1/8 prescale	(counters 0-2 only)
        0x0400	11	Interrupt	(status)
        0x0800	12	Target		(status)
        0x1000	13	Overflow	(status)
        0x2000	14	1/8 prescale	(counters 3-5 only)
        0x4000	15	1/16 prescale
        0x6000	14+15	1/256 prescale

    Sources:
        TC_SYSCLOCK	1	36.864MHz	(MIPS R3000A in 36.864MHz IOP mode)
        TC_PIXEL	2	13.5MHz		(Note: regardless of the actual screen mode)
        TC_HLINE	4	NTSC 15.73426573KHz (858 pixel clock), PAL 15.625KHz (864 pixel clock)
        TC_HOLD		8	??? (The FPS2BIOS code has this value too, but it isn't documented by Sony)

    Available timers:
        Name	Source(s)		Gate signal	Width	Prescale	Notes
        ----------------------------------------------------------------------------------------
        RTC0	SYSCLOCK|PIXEL|HOLD	H-blank		16	1		In use by PADMAN
        RTC1	SYSCLOCK|HLINE|HOLD	V-blank		16	1		In use by PADMAN
        RTC2	SYSCLOCK		None		16	8
        RTC3	SYSCLOCK|HLINE		V-blank		32	1
        RTC4	SYSCLOCK		None		32	256
        RTC5	SYSCLOCK		None		32	256	*/

void *GetTimersTable();

int AllocHardTimer(int source, int size, int prescale);
int ReferHardTimer(int source, int size, int mode, int modemask);
int FreeHardTimer(int timid);

void SetTimerMode(int timid, int mode);

u32 GetTimerStatus(int timid);

void SetTimerCounter(int timid, u32 count);
u32 GetTimerCounter(int timid);

void SetTimerCompare(int timid, u32 compare);
u32 GetTimerCompare(int timid);

void SetHoldMode(int holdnum, int mode);
u32 GetHoldMode(int holdnum);

u32 GetHoldReg(int holdnum);

int GetHardTimerIntrCode(int timid);

// guessed name
u32 GetTimerMode(int timid);
u32 (*GetTimerReadFunc(int timid))();

// clang-format off
#define timrman_IMPORTS \
	timrman_IMPORTS_start \
 \
 	I_AllocHardTimer \
	I_ReferHardTimer \
	I_FreeHardTimer \
 \
	I_SetTimerMode \
 \
	I_GetTimerStatus \
 \
	I_SetTimerCounter \
	I_GetTimerCounter \
 \
	I_SetTimerCompare \
	I_GetTimerCompare \
 \
	I_SetHoldMode \
	I_GetHoldMode \
 \
	I_GetHardTimerIntrCode \
 \
	timrman_IMPORTS_end END_IMPORT_TABLE
// clang-format on

#define timrman_IMPORTS_start DECLARE_IMPORT_TABLE(timrman, 1, 1)
#define timrman_IMPORTS_end   END_IMPORT_TABLE

#define I_GetTimersTable       DECLARE_IMPORT(3, GetTimersTable)
#define I_AllocHardTimer       DECLARE_IMPORT(4, AllocHardTimer)
#define I_ReferHardTimer       DECLARE_IMPORT(5, ReferHardTimer)
#define I_FreeHardTimer        DECLARE_IMPORT(6, FreeHardTimer)
#define I_SetTimerMode         DECLARE_IMPORT(7, SetTimerMode)
#define I_GetTimerStatus       DECLARE_IMPORT(8, GetTimerStatus)
#define I_SetTimerCounter      DECLARE_IMPORT(9, SetTimerCounter)
#define I_GetTimerCounter      DECLARE_IMPORT(10, GetTimerCounter)
#define I_SetTimerCompare      DECLARE_IMPORT(11, SetTimerCompare)
#define I_GetTimerCompare      DECLARE_IMPORT(12, GetTimerCompare)
#define I_SetHoldMode          DECLARE_IMPORT(13, SetHoldMode)
#define I_GetHoldMode          DECLARE_IMPORT(14, GetHoldMode)
#define I_GetHoldReg           DECLARE_IMPORT(15, GetHoldReg)
#define I_GetHardTimerIntrCode DECLARE_IMPORT(16, GetHardTimerIntrCode)
#define I_GetTimerMode         DECLARE_IMPORT(17, GetTimerMode)
#define I_GetTimerReadFunc     DECLARE_IMPORT(18, GetTimerReadFunc)

#ifdef __cplusplus
}
#endif

#endif /* __TIMRMAN_H__ */
