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

static void *sdrFunc(int fno, void *buffer, int length);

// Unofficial: wrap members for relative access
SdrEECBInfo g_eeCBInfo;

#if SDRDRV_IMPLEMENT_AUTODMA
// Unofficial: move to bss
static volatile int g_AutoDmaIntrCount;
// Unofficial: move to bss
static int g_AutoDmaInProcessing;
static void *g_AutoDmaBuf;
static int g_AutoDmaBufSize;

static int AutoDmaStatusCB(void *data)
{
	(void)data;

	if ( g_AutoDmaIntrCount < 4 && g_AutoDmaIntrCount >= 0 )
		g_AutoDmaIntrCount += 1;
	return 0;
}
#endif

void sce_sdr_loop(void *arg)
{
	// Unofficial: make local variable
	int rpc_arg[16];
	SifRpcDataQueue_t rpc_qd;
	SifRpcServerData_t rpc_sd;
	SdrInfo *si;
	int i;

	si = (SdrInfo *)arg;
	si->m_rpc_qd = &rpc_qd;
	si->m_rpc_sd = &rpc_sd;
	sceSifInitRpc(0);
	sceSifSetRpcQueue(&rpc_qd, GetThreadId());
	sceSifRegisterRpc(&rpc_sd, 0x80000701, sdrFunc, rpc_arg, 0, 0, &rpc_qd);
	// Unofficial: was inlined
	for ( i = 0; i < 16; i += 1 )
		sceSdrSetUserCommandFunction(0x9000 + (i * 0x10), NULL);
	sceSifRpcLoop(&rpc_qd);
}

static void *sdrFunc(int fno, void *buffer, int length)
{
	int ret;

	ret = 0;
	switch ( fno & 0xFFF0 )
	{
#if SDRDRV_IMPLEMENT_LIBOSDS
		case 0x6010:
			StInit();
			break;
		case 0x6020:
			StQuit();
			break;
		case 0x6030:
			StCalledVSync();
			break;
		case 0x6040:
			/* nothing */
			break;
		case 0x6050:
			ret = (s16)StVabOpen(*((u8 **)buffer + 1), *((u32 *)buffer + 2), *((u32 **)buffer + 3));
			break;
		case 0x6060:
			ret = (s16)StVabOpenFakeBody(*((u32 *)buffer + 1), *((u32 *)buffer + 2));
			break;
		case 0x6070:
			StVabOpenCompleted();
			break;
		case 0x6080:
			ret = (s16)StVabClose(*((u16 *)buffer + 2));
			break;
		case 0x6090:
			ret = (s16)StBgmOpen(*((u16 *)buffer + 2), *((u32 *)buffer + 2));
			break;
		case 0x60A0:
			StSetTickMode(*((s16 *)buffer + 2));
			break;
		case 0x60B0:
			ret = (s16)StBgmClose(*((u16 *)buffer + 2));
			break;
		case 0x60C0:
			StSetReverbType(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x60D0:
			StSetReverbDepth(*((s16 *)buffer + 2), *((s16 *)buffer + 4), *((s16 *)buffer + 6));
			break;
		case 0x60E0:
			StSetReverbDelaytime(*((s16 *)buffer + 2), *((s16 *)buffer + 4));
			break;
		case 0x60F0:
			StSetReverbFeedback(*((s16 *)buffer + 2), *((s16 *)buffer + 4));
			break;
		case 0x6100:
			ret = StGetSlotStatus(*((u16 *)buffer + 2));
			break;
		case 0x6110:
			StSetSbClear(*((u32 *)buffer + 1));
			break;
		case 0x6120:
			StSetMasterVol(*((s16 *)buffer + 2), *((s16 *)buffer + 4), *((s16 *)buffer + 6));
			break;
		case 0x6130:
			ret = (s16)StSetBgmVol(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x6140:
			StBgmPlay(*((u16 *)buffer + 2));
			break;
		case 0x6150:
			StBgmStop(*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u32 *)buffer + 3));
			break;
		case 0x6160:
			StSetBgmTempo(*((s16 *)buffer + 2), *((s16 *)buffer + 4));
			break;
		case 0x6170:
			ret = (s16)StGetBgmTempo(*((s16 *)buffer + 2));
			break;
		case 0x6180:
			ret = (s16)StGetBgmStatus(*((s16 *)buffer + 2));
			break;
		case 0x6190:
			ret = (s16)StGetBgmChStatus();
			break;
		case 0x61A0:
			ret = (s16)StDmaWrite(*((u8 **)buffer + 1), *((u32 **)buffer + 2), *((u32 *)buffer + 3));
			break;
		case 0x61B0:
			ret = (s16)StDmaRead(*((u32 **)buffer + 1), *((u8 **)buffer + 2), *((u32 *)buffer + 3));
			break;
		case 0x6200:
			ret = SetTimer(&g_timer_flag);
			break;
		case 0x6210:
			ret = ReleaseTimer();
			break;
		case 0x6300:
			ret = StSePlay(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x6310:
			ret = StSetSeVol(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
#endif
		case 0x8000:
			ret = sceSdInit(*((u32 *)buffer + 1));
			break;
		case 0x8010:
			sceSdSetParam(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x8020:
			ret = sceSdGetParam(*((u16 *)buffer + 2));
			break;
		case 0x8030:
			sceSdSetSwitch(*((u16 *)buffer + 2), *((u32 *)buffer + 2));
			break;
		case 0x8040:
			ret = sceSdGetSwitch(*((u16 *)buffer + 2));
			break;
		case 0x8050:
			sceSdSetAddr(*((u16 *)buffer + 2), *((u32 *)buffer + 2));
			break;
		case 0x8060:
			ret = sceSdGetAddr(*((u16 *)buffer + 2));
			break;
		case 0x8070:
			sceSdSetCoreAttr(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x8080:
			ret = sceSdGetCoreAttr(*((u16 *)buffer + 2));
			break;
		case 0x8090:
			ret = sceSdNote2Pitch(*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u16 *)buffer + 6), *((u16 *)buffer + 8));
			break;
		case 0x80A0:
			ret = sceSdPitch2Note(*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u16 *)buffer + 6));
			break;
		case 0x80B0:
			ret = sceSdProcBatch(*((sceSdBatch **)buffer + 1), *((u32 **)buffer + 2), *((u32 *)buffer + 3));
			break;
		case 0x80C0:
			ret = sceSdProcBatchEx(
				*((sceSdBatch **)buffer + 1), *((u32 **)buffer + 2), *((u32 *)buffer + 3), *((u32 *)buffer + 4));
			break;
		case 0x80D0:
			ret = sceSdVoiceTrans(
				*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u8 **)buffer + 3), *((u32 **)buffer + 4), *((u32 *)buffer + 5));
			break;
		case 0x80E0:
#if SDRDRV_IMPLEMENT_AUTODMA
		{
			int i;
			int j;
			int k;

			if ( !g_AutoDmaInProcessing && *((u32 *)buffer + 2) != 2 )
			{
				u32 *buf_init_ptr;

				sceSdSetTransCallback(1, AutoDmaStatusCB);
				g_AutoDmaBuf = (void *)(uiptr) * ((u32 *)buffer + 3);
				g_AutoDmaBufSize = *((u32 *)buffer + 4);
				buf_init_ptr = (u32 *)((u8 *)g_AutoDmaBuf + 0x3000);
				memset(g_AutoDmaBuf, 0, 0x3000);
				for ( i = 0; i < 512; i += 128 )
				{
					for ( j = 0; j < 128; j += 1 )
					{
						buf_init_ptr[j] = buf_init_ptr[j] / 512 * (i + j);
					}
					for ( k = 0; k < 128; k += 1 )
					{
						buf_init_ptr[k + 128] = buf_init_ptr[k + 128] / 512 * (i + k);
					}
				}
				g_AutoDmaIntrCount = 10;
				ret = sceSdBlockTrans(*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u8 **)buffer + 3), *((u32 *)buffer + 4));
				g_AutoDmaInProcessing = 1;
			}
			else if ( g_AutoDmaInProcessing && *((u32 *)buffer + 2) == 2 )
			{
				u32 *buf_init_ptr;
				u32 *buf_cler_ptr;
				int cur_status_1;
				int cur_status_2;

				cur_status_1 = sceSdBlockTransStatus(1, 0);
				// Unofficial: also handle cases other than 0, 1
				cur_status_2 = (cur_status_1 & 0xFFFFFF) - (uiptr)(u8 *)g_AutoDmaBuf
										 - ((cur_status_1 >> 24) == 1 ? (g_AutoDmaBufSize / 2) : 0);
				// Unofficial: also handle cases other than 0, 1
				buf_init_ptr =
					(u32 *)((u8 *)g_AutoDmaBuf
									+ ((cur_status_1 >> 24) == ((cur_status_2 < 0xC000) ? 1 : 0) ? (g_AutoDmaBufSize / 2) : 0));
				buf_cler_ptr =
					(u32 *)((u8 *)g_AutoDmaBuf
									+ ((cur_status_1 >> 24) == ((cur_status_2 < 0xC000) ? 0 : 1) ? (g_AutoDmaBufSize / 2) : 0));
				while ( cur_status_2 < 0xF000 )
				{
					cur_status_2 = sceSdBlockTransStatus(1, 0);
					// Unofficial: also handle cases other than 0, 1
					cur_status_2 = (cur_status_2 & 0xFFFFFF) - (uiptr)(u8 *)g_AutoDmaBuf
											 - ((cur_status_2 >> 24) == 1 ? (g_AutoDmaBufSize / 2) : 0);
				}
				for ( i = 0; i < 0x2000; i += 128 )
				{
					for ( j = 0; j < 128; j += 1 )
					{
						buf_init_ptr[j] = buf_init_ptr[j] / 0x2000 * (0x2000 - i - j);
					}
					for ( k = 0; k < 128; k += 1 )
					{
						buf_init_ptr[k + 128] = buf_init_ptr[k + 128] / 0x2000 * (0x2000 - i - k);
					}
				}
				memset(buf_cler_ptr, 0, g_AutoDmaBufSize / 2);
				g_AutoDmaIntrCount = 0;
				for ( i = 0; g_AutoDmaIntrCount < 2 && i <= 949999; i += 1 )
					;
				ret = sceSdBlockTrans(*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u8 **)buffer + 3), *((u32 *)buffer + 4));
				g_AutoDmaInProcessing = 0;
			}
			break;
		}
#else
			ret = sceSdBlockTrans(
				*((u16 *)buffer + 2), *((u16 *)buffer + 4), *((u8 **)buffer + 3), *((u32 *)buffer + 4), *((u32 *)buffer + 5));
#endif
		break;
		case 0x80F0:
			ret = sceSdVoiceTransStatus(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
		case 0x8100:
			ret = sceSdBlockTransStatus(*((u16 *)buffer + 2), *((u16 *)buffer + 4));
			break;
#if SDRDRV_OBSOLETE_FUNCS
		case 0x8110:
			ret = (int)sceSdSetTransCallback(
				*((u32 *)buffer + 1) ? 1 : 0,
				*((u32 *)buffer + 2) ? (*((u32 *)buffer + 1) ? _sce_sdrDMA1CallBackProc : _sce_sdrDMA0CallBackProc) : 0);
			break;
		case 0x8120:
			ret = (int)sceSdSetIRQCallback(*((u32 *)buffer + 1) ? _sce_sdrIRQCallBackProc : 0);
			break;
#endif
		case 0x8130:
			ret = sceSdSetEffectAttr(fno & 0xF, (sceSdEffectAttr *)buffer);
			break;
		case 0x8140:
			sceSdGetEffectAttr(fno & 0xF, &g_sdrInfo.m_e_attr);
			return &g_sdrInfo.m_e_attr;
		case 0x8150:
			ret = sceSdClearEffectWorkArea(*((u32 *)buffer + 1), *((u32 *)buffer + 2), *((u32 *)buffer + 3));
			break;
		case 0x8160:
			ret = (int)sceSdSetTransIntrHandler(
				*((u32 *)buffer + 1) ? 1 : 0,
				*((u32 *)buffer + 2) ? (*((u32 *)buffer + 1) ? _sce_sdrDMA1IntrHandler : 0) : _sce_sdrDMA0IntrHandler,
				&g_eeCBInfo);
			break;
		case 0x8170:
			ret = (int)sceSdSetSpu2IntrHandler(*((u32 *)buffer + 1) ? _sce_sdrSpu2IntrHandler : 0, &g_eeCBInfo);
			break;
		case 0x8180:
			ret = sceSdStopTrans(*((u32 *)buffer + 1));
			break;
		case 0x8190:
			ret = sceSdCleanEffectWorkArea(*((u32 *)buffer + 1), *((u32 *)buffer + 2), *((u32 *)buffer + 3));
			break;
		case 0x81A0:
			ret = sceSdSetEffectMode(fno & 0xF, (sceSdEffectAttr *)buffer);
			break;
		case 0x81C0:
			ret = sceSdProcBatch((sceSdBatch *)buffer + 1, (u32 *)&g_sdrInfo.m_procbat_returns[1], *((u16 *)buffer + 1));
			break;
		case 0x81D0:
			ret = sceSdProcBatchEx(
				(sceSdBatch *)buffer + 1, (u32 *)&g_sdrInfo.m_procbat_returns[1], *((u16 *)buffer + 1), *((u32 *)buffer + 1));
			break;
		case 0x8F10:
			ret = sceSdrChangeThreadPriority(*((u32 *)buffer + 1), *((u32 *)buffer + 2));
			break;
		case 0x9000:
		case 0x9010:
		case 0x9020:
		case 0x9030:
		case 0x9040:
		case 0x9050:
		case 0x9060:
		case 0x9070:
		case 0x9080:
		case 0x9090:
		case 0x90A0:
		case 0x90B0:
		case 0x90C0:
		case 0x90D0:
		case 0x90E0:
		case 0x90F0:
		{
			ret = g_sdrInfo.m_sceSdr_vUserCommandFunction[(fno & 0xF0) >> 4] ?
							g_sdrInfo.m_sceSdr_vUserCommandFunction[(fno & 0xF0) >> 4](fno, buffer, length) :
							0;
			break;
		}
		case 0xE620:
		{
			iop_thread_t thparam;

			thparam.attr = 0x2000000;
			thparam.thread = sce_sdrcb_loop;
			thparam.stacksize = 2048;
			thparam.option = 0;
			thparam.priority = g_eeCBInfo.m_initial_priority_cb;
			g_eeCBInfo.m_thid_cb = CreateThread(&thparam);
			StartThread(g_eeCBInfo.m_thid_cb, &g_eeCBInfo);
			Kprintf("SDR callback thread created\n");
			break;
		}
		case 0xE630:
		{
			if ( g_eeCBInfo.m_thid_cb > 0 )
			{
				TerminateThread(g_eeCBInfo.m_thid_cb);
				DeleteThread(g_eeCBInfo.m_thid_cb);
				g_eeCBInfo.m_thid_cb = 0;
				Kprintf("SDR callback thread deleted\n");
			}
			break;
		}
		default:
			Kprintf("SDR driver ERROR: unknown command %x \n", fno & 0xFFF0);
			break;
	}
	// Unofficial: always return pointer to procbat_returns
	g_sdrInfo.m_procbat_returns[0] = ret;
	return g_sdrInfo.m_procbat_returns;
}

sceSdrUserCommandFunction sceSdrSetUserCommandFunction(int command, sceSdrUserCommandFunction func)
{
	sceSdrUserCommandFunction oldf;

	if ( (command < 0x9000) || (command > 0x90F0) )
		return (sceSdrUserCommandFunction)-1;
	oldf = g_sdrInfo.m_sceSdr_vUserCommandFunction[(command & 0xF0) >> 4];
	g_sdrInfo.m_sceSdr_vUserCommandFunction[(command & 0xF0) >> 4] = func;
	return oldf;
}
