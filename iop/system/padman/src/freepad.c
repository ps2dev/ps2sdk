/*
 * Copyright (c) 2007 Lukasz Bruun <mail@lukasz.dk>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

/**
 * @file
 * IOP pad driver
 */

#include "irx.h"
#include "types.h"
#include "ctype.h"
#include "stdio.h"
#include "loadcore.h"
#include "sysclib.h"
#include "sysmem.h"
#include "thevent.h"
#include "thbase.h"
#include "rpcserver.h"
#include "freepad.h"

extern struct irx_export_table _exp_padman;

IRX_ID("padman", 3, 6);

extern int padman_init;
extern int thpri_hi;
extern int thpri_lo;

extern int vblank_end;

static int ParseParams(int argc, char *argv[])
{
	int i, new_thpri_hi, new_thpri_lo;
	const char *param;

	new_thpri_hi = PADMAN_THPRI_HI;
	new_thpri_lo = PADMAN_THPRI_LO;

	if(argc > 1)
	{
		argv++;

		for(i = 1; i < argc; i++,argv++)
		{
			if(strncmp("thpri=", *argv, 6) == 0)
			{
				//Parse high priority
				param = &(*argv)[6];
				if(isdigit(*param))
					new_thpri_hi = strtol(param, NULL, 10);

				//Skip value for high priority
				while(isdigit(*param))
					param++;

				//Valid high priority parameter
				if(new_thpri_hi - 9 < 115)
				{
					if(*param == ',')
					{
						++param;
						//Parse low priority
						if(isdigit(*param))
							new_thpri_lo = strtol(param, NULL, 10);

						if(new_thpri_lo - 9 >= 115)
						{
							M_KPRINTF("invalid priority %d\n", new_thpri_lo);
							return 0;
						}
					}

					if(new_thpri_lo < new_thpri_hi)
						M_PRINTF("high prio thread must be higher than low prio thread\n");
				}
				else
				{
					M_KPRINTF("invalid priority %d\n", new_thpri_hi);
					return 0;
				}
			}
		}
	}

	thpri_hi = new_thpri_hi;
	thpri_lo = new_thpri_lo;

	return 1;
}

int _start(int argc, char *argv[])
{
	padman_init = 0;
	vblank_end = 0;
	thpri_hi = PADMAN_THPRI_HI;
	thpri_lo = PADMAN_THPRI_LO;

	D_PRINTF("Debug Version\n");

	if(argc >= 2)
		ParseParams(argc, argv);

	if(thpri_hi != PADMAN_THPRI_HI || thpri_lo != PADMAN_THPRI_LO)
		M_PRINTF("thread priority: high=%d, low=%d\n", thpri_hi, thpri_lo);

	if(RegisterLibraryEntries(&_exp_padman) != 0)
	{
		M_PRINTF("RegisterLibraryEntries failed.\n");
		return MODULE_NO_RESIDENT_END;
	}

	if(InitRpcServers(thpri_lo) == 0)
	{
		M_PRINTF("Failed to init RPC servers.\n");
		return MODULE_NO_RESIDENT_END;
	}

	printf("Pad Driver for OSD\n");

	return MODULE_RESIDENT_END;
}

//This may have been a drop-in replacement for WaitEventFlag, which explains the similar, but unused 3rd and 4th parameters.
void WaitClearEvent(int eventflag, u32 bits, int mode, u32 *resbits_out)
{
	u32 resbits;

	WaitEventFlag(eventflag, bits | EF_EXIT_THREAD, WEF_OR, &resbits);

	if( resbits & EF_EXIT_THREAD )
	{	//Yes, it's unused. Probably leftover code.
		iop_thread_info_t tinfo;

		ReferThreadStatus(TH_SELF, &tinfo);
		SetEventFlag(eventflag, EF_EXIT_THREAD);
		ExitThread();
	}

	ClearEventFlag(eventflag, ~bits);
}
