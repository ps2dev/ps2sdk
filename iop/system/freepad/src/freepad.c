/*
 * freepad - IOP pad driver
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#include "irx.h"
#include "types.h"
#include "stdio.h"
#include "loadcore.h"
#include "thevent.h"
#include "thbase.h"
#include "rpcserver.h"
#include "freepad.h"

struct irx_export_table _exp_padman;

#define BANNER "FREEPAD %s\n"
#define VERSION "v1.0"

IRX_ID(MODNAME, 1, 2);

// Global variables
u16 version = 0x300;

s32 _start(char **argv, int argc)
{
	printf(BANNER, VERSION);
	
	D_PRINTF("Debug Version\n");

	if(RegisterLibraryEntries(&_exp_padman) != 0) 
	{
		M_PRINTF("RegisterLibraryEntries failed.\n");
		return 1;	
	}

	if(InitRpcServers() != 0)
	{
		M_PRINTF("Failed to init RPC servers.\n");
		return 1;
	}


	return 0;
}

void WaitClearEvent(u32 eventflag, u32 bits, u32 unused1, u32 unused2)
{
	u32 resbits;

	WaitEventFlag(eventflag, bits | EF_EXIT_THREAD, 1, &resbits);

	if( resbits & EF_EXIT_THREAD )
	{
		iop_thread_info_t tinfo;

		ReferThreadStatus(0, &tinfo);
		SetEventFlag(eventflag, EF_EXIT_THREAD);
		ExitThread();
	}

	ClearEventFlag(eventflag, ~bits);
}











