/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
*/

/**
 * @file
 * audsrv helpers
 */

#include <stdio.h>
#include <thbase.h>
#include <thsemap.h>
#include <loadcore.h>
#include <sysmem.h>
#include <intrman.h>
#include <sysclib.h>

#include "common.h"

/** Helper function to easily create threads
 * @param func       thread procedure
 * @param priority   thread priority (usually 40)
 * @param param      optional argument for thread procedure
 * @returns thread_id (int), negative on error
 *
 * Creates a thread based on the given parameter. Upon completion,
 * thread is started.
 */
int create_thread(void *func, int priority, void *param)
{
	int tid;
	iop_thread_t thr;

	thr.attr = TH_C;
	thr.thread = func;
	thr.option = 0;
	thr.priority = priority;
	thr.stacksize = 4096;
	tid = CreateThread(&thr);
	if (tid < 0)
	{
		return 0;
	}

	StartThread(tid, param);
	return tid;
}

/** Helper to print buffer in hex. Useful for debugging.
 * @param ptr   pointer to buffer
 * @param len   buffer length
 */
void print_hex_buffer(unsigned char *ptr, int len)
{
	int p;

	for (p=0; p<len; p++)
	{
		if (p > 0 && (p & 0x0f) == 0)
		{
			printf("\n");
		}

		printf("%02x ", ptr[p]);
	}
}
