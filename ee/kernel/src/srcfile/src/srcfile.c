/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "kernel.h"
#include "internal.h"

struct SyscallData{
	int syscall;
	void *function;
};

#define NUM_ENTRIES 6
static struct SyscallData entries[NUM_ENTRIES] = {
	{0xfc, &SetAlarm},
	{0xfd, &SetAlarm},
	{0xfe, &ReleaseAlarm},
	{0xff, &ReleaseAlarm},
	{0x12c, &Intc12Handler},	//Overwrites INTC 12 handler without using SetINTCHandler
	{0x08, &ResumeIntrDispatch}
};

void *_start(int syscall) __attribute__((section(".start")));

void *_start(int syscall)
{
	int i;

	for(i = 0; i < NUM_ENTRIES; i++)
	{
		if(syscall == entries[i].syscall)
			return entries[i].function;
	}

	return NULL;
}
