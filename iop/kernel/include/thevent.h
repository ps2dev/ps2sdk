/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Event flags for threads.
 */

#ifndef __THEVENT_H__
#define __THEVENT_H__

#include <types.h>
#include <irx.h>

/* Defines for WaitEventFlag */
#define WEF_AND   0
#define WEF_OR    1
#define WEF_CLEAR 0x10

// Attributes for event flags
/** Only one thread can wait on the event flag. */
#define EA_SINGLE 0
/** Multiple threads can wait on the event flag. */
#define EA_MULTI  2

typedef struct
{
    u32 attr;
    u32 option;
    u32 bits;
} iop_event_t;

typedef struct
{
    /** set by CreateEventFlag */
    u32 attr;
    /** set by CreateEventFlag */
    u32 option;
    /** initial 'bits' value set by CreateEventFlag */
    u32 initBits;
    /** current 'bits' value */
    u32 currBits;
    /** number of threads waiting on this event */
    int numThreads;
    int reserved1;
    int reserved2;
} iop_event_info_t;

int CreateEventFlag(iop_event_t *event);
int DeleteEventFlag(int ef);

int SetEventFlag(int ef, u32 bits);
int iSetEventFlag(int ef, u32 bits);

int ClearEventFlag(int ef, u32 bits);
int iClearEventFlag(int ef, u32 bits);

int WaitEventFlag(int ef, u32 bits, int mode, u32 *resbits);

int PollEventFlag(int ef, u32 bits, int mode, u32 *resbits);

int ReferEventFlagStatus(int ef, iop_event_info_t *info);
int iReferEventFlagStatus(int ef, iop_event_info_t *info);

// clang-format off
#define thevent_IMPORTS \
	thevent_IMPORTS_start \
 \
 	I_CreateEventFlag \
	I_DeleteEventFlag \
 \
 	I_SetEventFlag \
	I_iSetEventFlag \
 \
 	I_ClearEventFlag \
	I_iClearEventFlag \
 \
 	I_WaitEventFlag \
 \
 	I_PollEventFlag \
 \
 	I_ReferEventFlagStatus \
	I_iReferEventFlagStatus \
 \
	thevent_IMPORTS_end END_IMPORT_TABLE
// clang-format on

#define thevent_IMPORTS_start DECLARE_IMPORT_TABLE(thevent, 1, 1)
#define thevent_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateEventFlag       DECLARE_IMPORT(4, CreateEventFlag)
#define I_DeleteEventFlag       DECLARE_IMPORT(5, DeleteEventFlag)
#define I_SetEventFlag          DECLARE_IMPORT(6, SetEventFlag)
#define I_iSetEventFlag         DECLARE_IMPORT(7, iSetEventFlag)
#define I_ClearEventFlag        DECLARE_IMPORT(8, ClearEventFlag)
#define I_iClearEventFlag       DECLARE_IMPORT(9, iClearEventFlag)
#define I_WaitEventFlag         DECLARE_IMPORT(10, WaitEventFlag)
#define I_PollEventFlag         DECLARE_IMPORT(11, PollEventFlag)
#define I_ReferEventFlagStatus  DECLARE_IMPORT(13, ReferEventFlagStatus)
#define I_iReferEventFlagStatus DECLARE_IMPORT(14, iReferEventFlagStatus)

#endif /* __THEVENT_H__ */
