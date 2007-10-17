/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Event flags for threads.
*/

#ifndef IOP_THEVENT_H
#define IOP_THEVENT_H

#include "types.h"

/* Defines for WaitEventFlag */
#define WEF_AND		0
#define WEF_OR		1
#define WEF_CLEAR	0x10

typedef struct {
	u32	attr;
	u32	option;
	u32	bits;
} iop_event_t;

typedef struct {
	u32 attr;			// set by CreateEventFlag
	u32 option;			// set by CreateEventFlag
	u32 initBits;		// initial 'bits' value set by CreateEventFlag
	u32 currBits;		// current 'bits' value
	u32 numThreads;		// number of threads waiting on this event
} iop_event_info_t;


#define thevent_IMPORTS_start DECLARE_IMPORT_TABLE(thevent, 1, 1)
#define thevent_IMPORTS_end END_IMPORT_TABLE

int CreateEventFlag(iop_event_t *event);
#define I_CreateEventFlag DECLARE_IMPORT(4, CreateEventFlag)
int DeleteEventFlag(int ef);
#define I_DeleteEventFlag DECLARE_IMPORT(5, DeleteEventFlag)

int SetEventFlag(int ef, u32 bits);
#define I_SetEventFlag DECLARE_IMPORT(6, SetEventFlag)
int iSetEventFlag(int ef, u32 bits);
#define I_iSetEventFlag DECLARE_IMPORT(7, iSetEventFlag)

int ClearEventFlag(int ef, u32 bits);
#define I_ClearEventFlag DECLARE_IMPORT(8, ClearEventFlag)
int iClearEventFlag(int ef, u32 bits);
#define I_iClearEventFlag DECLARE_IMPORT(9, iClearEventFlag)

int WaitEventFlag(int ef, u32 bits, int mode, u32 *resbits);
#define I_WaitEventFlag DECLARE_IMPORT(10, WaitEventFlag)

int PollEventFlag(int ef, u32 bits, int mode, u32 *resbits);
#define I_PollEventFlag DECLARE_IMPORT(11, PollEventFlag)

int ReferEventFlagStatus(int ef, iop_event_info_t* info);
#define I_ReferEventFlagStatus DECLARE_IMPORT(13, ReferEventFlagStatus)
int iReferEventFlagStatus(int ef, iop_event_info_t* info);
#define I_iReferEventFlagStatus DECLARE_IMPORT(14, iReferEventFlagStatus)


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


#endif /* IOP_THEVENT_H */
