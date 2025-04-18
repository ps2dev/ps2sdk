/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <sdr_i.h>

IRX_ID("sdr_driver", 4, 1);
// Based on the module from SDK 3.1.0.

extern struct irx_export_table _exp_sdrdrv;
// Unofficial: wrap members for relative access
SdrInfo g_sdrInfo;

static int module_start(int ac, char **av)
{
	int code;
	int i;
	iop_thread_t thprarm;
	int state;
	// Unofficial: make local
	int initial_priority_main;

	CpuSuspendIntr(&state);
	code = RegisterLibraryEntries(&_exp_sdrdrv);
	CpuResumeIntr(state);
	if ( code )
		return 1;
	Kprintf("SDR driver version 4.0.1 (C) SCEI\n");
	initial_priority_main = 24;
	g_eeCBInfo.m_initial_priority_cb = 24;
	g_sdrInfo.m_thid_main = 0;
	g_eeCBInfo.m_thid_cb = 0;
	for ( i = 1; i < ac; i += 1 )
	{
		if ( !strncmp("thpri=", av[i], 6) )
		{
			const char *p;

			p = av[i] + 6;
			if ( isdigit(*p) )
			{
				initial_priority_main = strtol(p, 0, 10);
				if ( (unsigned int)(initial_priority_main - 9) >= 0x73 )
				{
					Kprintf(" SDR driver error: invalid priority %d\n", initial_priority_main);
					initial_priority_main = 24;
				}
			}
			while ( isdigit(*p) )
				p += 1;
			if ( *p == ',' && isdigit(p[1]) )
			{
				g_eeCBInfo.m_initial_priority_cb = strtol(&p[1], 0, 10);
				if ( (unsigned int)(g_eeCBInfo.m_initial_priority_cb - 9) >= 0x73 )
				{
					Kprintf(" SDR driver error: invalid priority %d\n", g_eeCBInfo.m_initial_priority_cb);
					g_eeCBInfo.m_initial_priority_cb = 24;
				}
			}
			if ( g_eeCBInfo.m_initial_priority_cb < initial_priority_main )
			{
				Kprintf(" SDR driver ERROR:\n");
				Kprintf("   callback th. priority is higher than main th. priority.\n");
				g_eeCBInfo.m_initial_priority_cb = initial_priority_main;
			}
			Kprintf(
				" SDR driver: thread priority: main=%d, callback=%d\n",
				initial_priority_main,
				g_eeCBInfo.m_initial_priority_cb);
		}
	}
	thprarm.attr = 0x2000000;
	thprarm.thread = sce_sdr_loop;
	// Unofficial: original stack size was 2048
	thprarm.stacksize = 4096;
	thprarm.option = 0;
	thprarm.priority = initial_priority_main;
	g_sdrInfo.m_thid_main = CreateThread(&thprarm);
	if ( g_sdrInfo.m_thid_main <= 0 )
		return 1;
	StartThread(g_sdrInfo.m_thid_main, &g_sdrInfo);
	Kprintf(" Exit rsd_main \n");
	return 2;
}

static int module_stop(int ac, char **av)
{
	int code;
	int state;

	(void)ac;
	(void)av;

	CpuSuspendIntr(&state);
	code = ReleaseLibraryEntries(&_exp_sdrdrv);
	CpuResumeIntr(state);
	if ( code )
		return 2;
	sceSdSetTransIntrHandler(0, 0, 0);
	sceSdSetTransIntrHandler(1, 0, 0);
	sceSdSetSpu2IntrHandler(0, 0);
	if ( g_eeCBInfo.m_thid_cb > 0 )
	{
		TerminateThread(g_eeCBInfo.m_thid_cb);
		DeleteThread(g_eeCBInfo.m_thid_cb);
		g_eeCBInfo.m_thid_cb = 0;
	}
	if ( g_sdrInfo.m_thid_main > 0 )
	{
		sceSifRemoveRpc(g_sdrInfo.m_rpc_sd, g_sdrInfo.m_rpc_qd);
		sceSifRemoveRpcQueue(g_sdrInfo.m_rpc_qd);
		TerminateThread(g_sdrInfo.m_thid_main);
		DeleteThread(g_sdrInfo.m_thid_main);
		g_sdrInfo.m_thid_main = 0;
	}
	Kprintf(" sdrdrv: unloaded! \n");
	return 1;
}

int _start(int ac, char **av)
{
	return (ac >= 0) ? module_start(ac, av) : module_stop(-ac, av);
}

int sceSdrChangeThreadPriority(int priority_main, int priority_cb)
{
	int cur_priority;
	int ret;
	iop_thread_info_t thstatus;

	if ( (unsigned int)(priority_main - 9) >= 0x73 || (unsigned int)(priority_cb - 9) >= 0x73 )
		return -403;
	if ( priority_cb < priority_main )
	{
		Kprintf(" SDR driver ERROR:\n");
		Kprintf("   callback th. priority is higher than main th. priority.\n");
	}
	cur_priority = (priority_cb < priority_main) ? priority_main : priority_cb;
	ReferThreadStatus(0, &thstatus);
	ChangeThreadPriority(0, 8);
	ret = 0;
	if ( g_sdrInfo.m_thid_main > 0 )
		ret = ChangeThreadPriority(g_sdrInfo.m_thid_main, priority_main);
	if ( ret < 0 )
		return ret;
	if ( g_eeCBInfo.m_thid_cb > 0 )
		ret = ChangeThreadPriority(g_eeCBInfo.m_thid_cb, cur_priority);
	if ( ret < 0 )
		return ret;
	g_eeCBInfo.m_initial_priority_cb = cur_priority;
	ChangeThreadPriority(0, thstatus.currentPriority);
	return 0;
}
