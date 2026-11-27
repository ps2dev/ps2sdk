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

// Unofficial: argument
static void sceSifCmdLoop2(SifRpcClientData_t *cd, SdrEECBInfo *cbi)
{
	while ( 1 )
	{
		int state;
		// Unofficial: make local variable
		SdrEECBData eeCBDataSend;

		while ( !cbi->m_eeCBData.mode )
			SleepThread();
		CpuSuspendIntr(&state);
		// Unofficial: was inlined
		eeCBDataSend = cbi->m_eeCBData;
		CpuResumeIntr(state);
#if SDRDRV_EECB_COMPAT
		if ( eeCBDataSend.mode )
		{
			int mode_tmp;
			int mode_cur;

			mode_tmp = eeCBDataSend.mode;
			mode_cur = mode_tmp;
			while ( 1 )
			{
				// Only the obsolete DMA0/DMA1/IRQ funcs (not implemented in libsdr 4.0.1) clashes
				if ( mode_cur & SDR_CB_DMA0 )
				{
					mode_cur &= ~SDR_CB_DMA0;
					eeCBDataSend.mode = 1;
				}
				else if ( mode_cur & SDR_CB_DMA1 )
				{
					mode_cur &= ~SDR_CB_DMA1;
					eeCBDataSend.mode = 2;
				}
				else if ( mode_cur & SDR_CB_IRQ )
				{
					mode_cur &= ~SDR_CB_IRQ;
					eeCBDataSend.mode = 3;
				}
				else if ( mode_cur & SDR_CB_DMA0INT )
				{
					mode_cur &= ~SDR_CB_DMA0INT;
					eeCBDataSend.mode = 11;
				}
				else if ( mode_cur & SDR_CB_DMA1INT )
				{
					mode_cur &= ~SDR_CB_DMA1INT;
					eeCBDataSend.mode = 12;
				}
				else if ( mode_cur & SDR_CB_IRQINT )
				{
					mode_cur &= ~SDR_CB_IRQINT;
					eeCBDataSend.mode = 13;
				}
				else
				{
					break;
				}
				sceSifCallRpc(cd, 0, 0, &eeCBDataSend, sizeof(eeCBDataSend), 0, 0, 0, 0);
			}
			eeCBDataSend.mode = mode_tmp;
		}
		// Set the high bit to make libsdr 2.0.0 and lower not process it
		eeCBDataSend.mode |= ((u32)1 << 31);
#endif
		sceSifCallRpc(cd, 0, 0, &eeCBDataSend, sizeof(eeCBDataSend), 0, 0, 0, 0);
#if SDRDRV_EECB_COMPAT
		eeCBDataSend.mode &= ~((u32)1 << 31);
#endif
		CpuSuspendIntr(&state);
		if ( cbi->m_eeCBData.mode == eeCBDataSend.mode )
		{
			cbi->m_eeCBData.mode = 0;
			iCancelWakeupThread(TH_SELF);
			CpuResumeIntr(state);
			break;
		}
		cbi->m_eeCBData.mode &= ~eeCBDataSend.mode;
		CpuResumeIntr(state);
	}
}

#if SDRDRV_OBSOLETE_FUNCS
int _sce_sdrDMA0CallBackProc(void *data)
{
	(void)data;

	g_eeCBInfo.m_eeCBData.mode |= SDR_CB_DMA0;
	iWakeupThread(g_eeCBInfo.m_thid_cb);
	return 1;
}

int _sce_sdrDMA1CallBackProc(void *data)
{
	(void)data;

	g_eeCBInfo.m_eeCBData.mode |= SDR_CB_DMA1;
	iWakeupThread(g_eeCBInfo.m_thid_cb);
	return 1;
}

int _sce_sdrIRQCallBackProc(void *data)
{
	(void)data;

	g_eeCBInfo.m_eeCBData.mode |= SDR_CB_IRQ;
	iWakeupThread(g_eeCBInfo.m_thid_cb);
	return 1;
}
#endif

int _sce_sdrDMA0IntrHandler(int core, void *common)
{
	SdrEECBInfo *cbi;

	(void)core;

	cbi = (SdrEECBInfo *)common;
	cbi->m_eeCBData.mode |= SDR_CB_DMA0INT;
	iWakeupThread(cbi->m_thid_cb);
	return 0;
}

int _sce_sdrDMA1IntrHandler(int core, void *common)
{
	SdrEECBInfo *cbi;

	(void)core;

	cbi = (SdrEECBInfo *)common;
	cbi->m_eeCBData.mode |= SDR_CB_DMA1INT;
	iWakeupThread(cbi->m_thid_cb);
	return 0;
}

int _sce_sdrSpu2IntrHandler(int core_bit, void *common)
{
	SdrEECBInfo *cbi;

	(void)core_bit;

	cbi = (SdrEECBInfo *)common;
	cbi->m_eeCBData.mode |= SDR_CB_IRQINT;
	cbi->m_eeCBData.voice_bit = core_bit;
	iWakeupThread(cbi->m_thid_cb);
	return 0;
}

void sce_sdrcb_loop(void *arg)
{
	SdrEECBInfo *cbi;
	// Unofficial: make local
	SifRpcClientData_t cd;

	cbi = (SdrEECBInfo *)arg;
	cbi->m_eeCBData.mode = 0;
	while ( sceSifBindRpc(&cd, sce_SDRST_CB, 0) >= 0 )
	{
		int i;

		for ( i = 0; i < 10000; i += 1 )
			;
		if ( cd.server )
			sceSifCmdLoop2(&cd, cbi);
	}
	Kprintf("error \n");
	while ( 1 )
		;
}
