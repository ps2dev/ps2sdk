/*
 * thevent.h - Event flags for threads.
 *
 * Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
 *
 * See the file LICENSE included with this distribution for licensing terms.
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

#endif /* IOP_THEVENT_H */
