/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "rs_i.h"

static int ret = 0;
static int gMultiVoiceNum = 0;
#ifndef LIB_OSD_100
static volatile int g_AutoDmaIntrCount = 0;
static int g_AutoDmaInProcessing = 0;
#endif

SpuStEnv *gStPtr;
int gStThid;
static char seq_table[3520];  // sizeof(libsnd2_sequence_struct_t) * 20
static SpuVoiceAttr s_attr;
static SpuCommonAttr c_attr;
static char spu_malloc_rec[1032];  // sizeof(libspu2_malloc_t) * 129
static SpuReverbAttr r_attr;
static int gRpcArg[384];
static char status[24];
SpuStEnv gStBuff;

#ifndef LIB_OSD_100
static u8 *g_AutoDmaBuf;
static int g_AutoDmaBufSize;
#endif

#ifndef LIB_OSD_100
static void AutoDmaStatusCB(void)
{
	if ( g_AutoDmaIntrCount < 4 && g_AutoDmaIntrCount >= 0 )
		g_AutoDmaIntrCount += 1;
}

#ifdef LIB_OSD_110
static void AutoDmaClearBuffer(void)
{
	SpuSetTransferStartAddr(0x4800u);
	SpuWrite0(0x800u);
	SpuIsTransferCompleted(1);
}
#endif
#endif

void sce_spu2_loop(void *userdata)
{
	SifRpcDataQueue_t qd;
	SifRpcServerData_t sd;

	(void)userdata;

	CpuEnableIntr();
	EnableIntr(IOP_IRQ_DMA_SPU);
	EnableIntr(IOP_IRQ_DMA_SPU2);
	EnableIntr(IOP_IRQ_SPU);
	sceSifSetRpcQueue(&qd, GetThreadId());
	sceSifRegisterRpc(&sd, sce_SPU_DEV, (SifRpcFunc_t)spuFunc, gRpcArg, 0, 0, &qd);
	sceSifRpcLoop(&qd);
}

int AutoDmaWaitForCompletion(unsigned int played_size, int start_wait_count)
{
	for ( ; start_wait_count <= 949999; start_wait_count += 1 )
	{
		unsigned int v3;
		unsigned int v4;

		v3 = SpuAutoDMAGetStatus();
		if ( (((v3 >> 24)) & 0xFF) == 1 )
			v4 = (v3 & 0xFFFFFF) - (u32)g_AutoDmaBuf - g_AutoDmaBufSize / 2;
		else
			v4 = (v3 & 0xFFFFFF) - (u32)g_AutoDmaBuf;
		if ( v4 >= played_size )
			break;
		__asm__ __volatile__("" : "+g"(start_wait_count) : :);
	}
	return start_wait_count;
}

void *spuFunc(unsigned int command, void *data, int size)
{
	(void)size;

	switch ( command )
	{
		case 0x0001:
			SpuInit();
			break;
		case 0x0002:
			ret = SpuSetCore(*((u32 *)data + 1));
			break;
		case 0x0005:
			SpuSetKey(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x0006:
			ret = SpuSetReverb(*((u32 *)data + 1));
			break;
		case 0x0007:
			ret = SpuClearReverbWorkArea(*((u32 *)data + 1));
			break;
		case 0x0008:
			SpuSetReverbEndAddr(*((u32 *)data + 1));
			break;
		case 0x000A:
			SpuSetReverbModeDepth(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x000B:
			ret = SpuSetReverbVoice(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x000C:
			ret = SpuSetIRQ(*((u32 *)data + 1));
			break;
		case 0x000D:
			ret = SpuSetIRQAddr(*((u32 *)data + 1));
			break;
		case 0x000E:
			ret = SpuSetTransferMode(*((u32 *)data + 1));
			break;
		case 0x0010:
			ret = SpuSetTransferStartAddr(*((u32 *)data + 1));
			break;
		case 0x0011:
			ret = SpuWrite(*((u8 **)data + 1), *((u32 *)data + 2));
			break;
		case 0x0012:
			ret = SpuWrite0(*((u32 *)data + 1));
			break;
		case 0x0013:
			ret = SpuIsTransferCompleted(*((u32 *)data + 1));
			break;
		case 0x0017:
			ret = SpuGetReverbEndAddr();
			break;
		case 0x0018:
			ret = SpuWritePartly(*((u8 **)data + 1), *((u32 *)data + 2));
			break;
		case 0x0019:
			SpuInitHot();
			break;
		case 0x001A:
			ret = SpuIsReverbWorkAreaReserved(*((u32 *)data + 1));
			break;
		case 0x001B:
			ret = SpuMallocWithStartAddr(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x001C:
			ret = SpuRead(*((u8 **)data + 1), *((u32 *)data + 2));
			break;
		case 0x001D:
			ret = SpuReadDecodedData(*((SpuDecodedData **)data + 1), *((u32 *)data + 2));
			break;
		case 0x001E:
			ret = SpuReserveReverbWorkArea(*((u32 *)data + 1));
			break;
		case 0x0020:
			ret = SpuSetMute(*((u32 *)data + 1));
			break;
		case 0x0021:
			ret = SpuSetNoiseClock(*((u32 *)data + 1));
			break;
		case 0x0022:
			ret = SpuSetNoiseVoice(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x0023:
			ret = SpuSetPitchLFOVoice(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x0024:
			ret = SpuStGetStatus();
			break;
		case 0x0025:
			ret = SpuStGetVoiceStatus();
			break;
		case 0x0100:
			ret = SpuInitMalloc(*((u32 *)data + 1), spu_malloc_rec);
			break;
		case 0x0101:
			ret = SpuMalloc(*((u32 *)data + 1));
			break;
		case 0x0200:
		{
			gStPtr = SpuStInit(*((u32 *)data + 1));
			ret = (int)&gStBuff;
			SpuStSetPreparationFinishedCallback(spustCB_preparation_finished);
			SpuStSetTransferFinishedCallback(spustCB_transfer_finished);
			SpuStSetStreamFinishedCallback(spustCB_stream_finished);
			break;
		}
		case 0x0201:
			ret = SpuStQuit();
			break;
		case 0x0202:
		{
			memcpy(gStPtr, &gStBuff, sizeof(SpuStEnv));
			ret = SpuStTransfer(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		}
		case 0x0203:
			ret = SpuStSetCore(*((u32 *)data + 1));
			break;
		case 0x100C:
			ret = SpuGetIRQAddr();
			break;
		case 0x1010:
			ret = SpuFlush(*((u32 *)data + 1));
			break;
		case 0x1011:
			SpuFree(*((u32 *)data + 1));
			break;
		case 0x1013:
			ret = SpuGetIRQ();
			break;
		case 0x1014:
			ret = SpuGetMute();
			break;
		case 0x1015:
			ret = SpuGetNoiseClock();
			break;
		case 0x1016:
			ret = SpuGetNoiseVoice();
			break;
		case 0x1017:
			ret = SpuGetPitchLFOVoice();
			break;
		case 0x1018:
			ret = SpuGetReverb();
			break;
		case 0x1019:
			ret = SpuGetReverbVoice();
			break;
		case 0x101A:
			ret = SpuGetTransferMode();
			break;
		case 0x101B:
			ret = SpuGetTransferStartAddr();
			break;
		case 0x101C:
			ret = SpuGetKeyStatus(*((u32 *)data + 1));
			break;
		case 0x1020:
		{
#ifdef LIB_OSD_100
			ret = SpuAutoDMAWrite(*((u8 **)data + 1), *((u32 *)data + 2), *((u32 *)data + 3));
#else
			if ( g_AutoDmaInProcessing )
			{
				ret = 0;
			}
			else
			{
				u32 *v7;
				int ii;
				int jj;
				int kk;

#ifdef LIB_OSD_110
				AutoDmaClearBuffer();
#endif
				SpuAutoDMASetCallback(AutoDmaStatusCB);
				g_AutoDmaBuf = (u8 *)*((u32 *)data + 1);
				g_AutoDmaBufSize = *((u32 *)data + 2);
				v7 = (u32 *)(g_AutoDmaBuf + 0x3000);
				memset((void *)g_AutoDmaBuf, 0, 0x3000);
				for ( ii = 0; ii < 512; ii += 128 )
				{
					for ( jj = 0; jj < 128; jj += 1 )
					{
						v7[jj] = v7[jj] / 512 * (jj + ii);
					}
					for ( kk = 0; kk < 128; kk += 1 )
					{
						v7[kk] = v7[kk] / 512 * (kk + ii);
					}
				}
				g_AutoDmaIntrCount = 10;
				ret = SpuAutoDMAWrite(*((u8 **)data + 1), *((u32 *)data + 2), *((u32 *)data + 3));
				g_AutoDmaInProcessing = 1;
			}
#endif
			break;
		}
		case 0x1021:
		{
#ifdef LIB_OSD_100
			SpuAutoDMAStop();
#else
			int n;
			u32 *v3;
			void *v4;
			int v5;
			int v11;
			int j;
			int k;
			int m;
#ifndef LIB_OSD_110
			u32 *v12;
			int v19;
			int v20;
			size_t sizea;
#endif

			if ( g_AutoDmaInProcessing )
			{
				v11 = SpuAutoDMAGetStatus();
				if ( v11 >> 24 == 1 )
					v5 = (v11 & 0xFFFFFF) - (u32)g_AutoDmaBuf - g_AutoDmaBufSize / 2;
				else
					v5 = (v11 & 0xFFFFFF) - (u32)g_AutoDmaBuf;
#ifdef LIB_OSD_110
				if ( v5 > 0xbfff )
#else
				if ( v5 >= 0x6000 )
#endif
				{
					while ( v5 <= 0xefff )
					{
						v5 = SpuAutoDMAGetStatus();
						if ( v5 >> 24 == 1 )
							v5 = (v5 & 0xFFFFFF) - (u32)g_AutoDmaBuf - g_AutoDmaBufSize / 2;
						else
							v5 = (v5 & 0xFFFFFF) - (u32)g_AutoDmaBuf;
					}
#ifndef LIB_OSD_110
					v19 = 0;
					v20 = 0x1c00;
					sizea = 0x2000;
					g_AutoDmaIntrCount = 0;
#endif
					if ( v11 >> 24 == 1 )
					{
						v3 = (u32 *)g_AutoDmaBuf;
						v4 = (void *)(g_AutoDmaBufSize / 2 + g_AutoDmaBuf);
					}
					else
					{
						v3 = (u32 *)(g_AutoDmaBufSize / 2 + g_AutoDmaBuf);
						v4 = (void *)g_AutoDmaBuf;
					}
				}
				else
				{
#ifndef LIB_OSD_110
					v19 = ((v5 + 1023) / 1024) << 10;
					v20 = 0x2000 - ((v19 + ((((v5 + 1023) / 1024) & 0x200000) != 0 ? 0x3FF : 0)) >> 10 << 7);
					sizea = 0;
					g_AutoDmaIntrCount = 0;
#endif
					if ( v11 >> 24 == 1 )
					{
						v3 = (u32 *)(g_AutoDmaBufSize / 2 + g_AutoDmaBuf);
						v4 = (void *)g_AutoDmaBuf;
					}
					else
					{
						v3 = (u32 *)g_AutoDmaBuf;
						v4 = (void *)(g_AutoDmaBufSize / 2 + g_AutoDmaBuf);
					}
				}
#ifdef LIB_OSD_110
				for ( j = 0; j < 0x2000; j += 128 )
				{
					for ( k = 0; k < 128; k += 1 )
					{
						v3[k] = v3[k] / 0x2000 * (0x2000 - j - k);
					}
					for ( m = 0; m < 128; m += 1 )
					{
						v3[m] = v3[m] / 0x2000 * (0x2000 - j - m);
					}
				}
				memset(v4, 0, g_AutoDmaBufSize / 2);
				g_AutoDmaIntrCount = 0;
				for ( n = 0; g_AutoDmaIntrCount < 2 && n <= 949999; n += 1 )
				{
					__asm__ __volatile__("" : "+g"(n) : :);
				}
#else
				v12 = &v3[v19 / 4];
				for ( j = 0; j < v20; j += 128 )
				{
					for ( k = 0; k < 128; k += 1 )
					{
						if ( !v20 )
							__builtin_trap();
						if ( v20 == -1 && *v12 == 0x80000000 )
							__builtin_trap();
						v12[k] = v12[k] / v20 * (v20 - k - j);
					}
					for ( m = 0; m < 128; m += 1 )
					{
						if ( !v20 )
							__builtin_trap();
						if ( v20 == -1 && *v12 == 0x80000000 )
							__builtin_trap();
						v12[m] = v12[m] / v20 * (v20 - m - j);
					}
				}
				if ( sizea )
					memset(v12, 0, sizea);
				if ( v5 >= 0x6000 )
				{
					n = 0;
					for ( ; g_AutoDmaIntrCount <= 0 && n <= 949999; n += 1 )
					{
						__asm__ __volatile__("" : "+g"(n) : :);
					}
					memset(v4, 0, g_AutoDmaBufSize / 2);
					for ( ; g_AutoDmaIntrCount < 2 && n <= 949999; n += 1 )
					{
						__asm__ __volatile__("" : "+g"(n) : :);
					}
				}
				else
				{
					n = AutoDmaWaitForCompletion(0xF000u, 0);
					memset(v4, 0, g_AutoDmaBufSize / 2);
					for ( ; g_AutoDmaIntrCount <= 0 && n <= 949999; n += 1 )
					{
						__asm__ __volatile__("" : "+g"(n) : :);
					}
				}
				memset(v3, 0, g_AutoDmaBufSize / 2);
				AutoDmaWaitForCompletion(0x4001 - sizea, n);
#endif
				SpuAutoDMAStop();
#ifdef LIB_OSD_110
				AutoDmaClearBuffer();
#endif
				g_AutoDmaInProcessing = 0;
			}
#endif
			break;
		}
		case 0x1022:
			ret = SpuAutoDMAGetStatus();
			break;
		case 0x1023:
			SpuSetAutoDMAAttr(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6), *((u16 *)data + 8));
			break;
		case 0x1024:
			SpuSetSerialInAttr(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x1030:
			gMultiVoiceNum = *((u32 *)data + 1);
			break;
		case 0x1031:
			SpuSetDigitalOut(*((u32 *)data + 1));
			break;
		case 0x4001:
			ret = SsBlockVoiceAllocation();
			break;
		case 0x4002:
			SsEnd();
			break;
		case 0x4003:
			SsChannelMute(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3));
			break;
		case 0x4004:
			ret = SsGetActualProgFromProg(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4005:
			ret = SsGetChannelMute(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4006:
			ret = (int)SsGetCurrentPoint(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4007:
			ret = SsGetVoiceMask();
			break;
		case 0x4008:
			SsInit();
			break;
		case 0x4009:
			ret = SsIsEos(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x400A:
			SsAllocateVoices(*((u8 *)data + 4), *((u8 *)data + 8));
			break;
		case 0x4010:
			SsPitchCorrect(*((u16 *)data + 2));
			break;
		case 0x4011:
			ret = (u16)SsPitchFromNote(*((u16 *)data + 2), *((u16 *)data + 4), *((u8 *)data + 12), *((u8 *)data + 16));
			break;
		case 0x4012:
			SsPlayBack(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4013:
			SsQueueKeyOn(*((u32 *)data + 1));
			break;
		case 0x4014:
			SsQueueReverb(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x4015:
			SsQuit();
			break;
		case 0x4017:
			SsSetTableSize(seq_table, *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4018:
			SsSetTickMode(*((u32 *)data + 1));
			break;
		case 0x4019:
			SsSepClose(*((u16 *)data + 2));
			break;
		case 0x4020:
			ret = SsSepOpen((unsigned int *)data + 1, *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4021:
			SsSepPause(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4022:
			SsSepPlay(*((u16 *)data + 2), *((u16 *)data + 4), *((u8 *)data + 12), *((u16 *)data + 8));
			break;
		case 0x4023:
			SsSepReplay(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4024:
			SsSepSetAccelerando(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3), *((u32 *)data + 4));
			break;
		case 0x4025:
			SsSepSetCrescendo(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6), *((u32 *)data + 4));
			break;
		case 0x4026:
			SsSepSetDecrescendo(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6), *((u32 *)data + 4));
			break;
		case 0x4027:
			SsSepSetRitardando(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3), *((u32 *)data + 4));
			break;
		case 0x4028:
			SsSepSetVol(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6), *((u16 *)data + 8));
			break;
		case 0x4029:
			SsSepStop(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4030:
			SsSeqGetVol(*((u16 *)data + 2), *((u16 *)data + 4), (s16 *)data + 6, (s16 *)data + 8);
			break;
		case 0x4031:
			ret = SsSeqOpen(*((unsigned int **)data + 1), *((u16 *)data + 4));
			break;
		case 0x4032:
			SsSeqPause(*((u16 *)data + 2));
			break;
		case 0x4033:
			SsSeqPlayPtoP(
				*((u16 *)data + 2),
				*((u16 *)data + 4),
				*((u8 **)data + 3),
				*((u8 **)data + 4),
				*((u8 *)data + 20),
				*((u16 *)data + 12));
			break;
		case 0x4034:
			SsSeqReplay(*((u16 *)data + 2));
			break;
		case 0x4035:
			SsSeqSetAccelerando(*((u16 *)data + 2), *((u32 *)data + 2), *((u32 *)data + 3));
			break;
		case 0x4036:
			SsSeqSetCrescendo(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3));
			break;
		case 0x4037:
			SsSeqSetDecrescendo(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3));
			break;
		case 0x4038:
			SsSeqSetRitardando(*((u16 *)data + 2), *((u32 *)data + 2), *((u32 *)data + 3));
			break;
		case 0x4039:
			SsSeqSetNext(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4040:
			SsSeqCalledTbyT();
			break;
		case 0x4041:
			SsSeqClose(*((u16 *)data + 2));
			break;
		case 0x4042:
			SsSeqPlay(*((u16 *)data + 2), *((u8 *)data + 8), *((u16 *)data + 6));
			break;
		case 0x4043:
			SsSeqSetVol(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4044:
			SsSeqSkip(*((u16 *)data + 2), *((u16 *)data + 4), *((u8 *)data + 12), *((u16 *)data + 8));
			break;
		case 0x4045:
			SsSeqStop(*((u16 *)data + 2));
			break;
		case 0x4046:
			SsSetAutoKeyOffMode(*((u16 *)data + 2));
			break;
		case 0x4047:
			SsSetCurrentPoint(*((u16 *)data + 2), *((u16 *)data + 4), *((u8 **)data + 3));
			break;
		case 0x4048:
			SsSetLoop(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4049:
			SsSetMono();
			break;
		case 0x404A:
			SsSetMVol(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4050:
			SsSetNext(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6), *((u16 *)data + 8));
			break;
		case 0x4051:
			ret = SsSetReservedVoice(*((u8 *)data + 4));
			break;
		case 0x4052:
			SsSetStereo();
			break;
		case 0x4053:
			SsSetTempo(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4054:
			SsSetVoiceMask(*((u32 *)data + 1));
			break;
		case 0x4055:
			SsStart();
			break;
		case 0x4056:
			SsStart2();
			break;
		case 0x4057:
			ret = SsUnBlockVoiceAllocation();
			break;
		case 0x4058:
			SsUtFlush();
			break;
		case 0x4059:
			ret = SsUtGetVagAddr(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x4060:
			ret = SsUtGetVagAddrFromTone(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x4061:
			ret = SsUtGetVBaddrInSB(*((u16 *)data + 2));
			break;
		case 0x4062:
			SsVabClose(*((u16 *)data + 2));
			break;
		case 0x4063:
			ret = SsVabOpenHead(*((u8 **)data + 1), *((u16 *)data + 4));
			break;
		case 0x4064:
			ret = SsVabTransBodyPartly(*((u8 **)data + 1), *((u32 *)data + 2), *((u16 *)data + 6));
			break;
		case 0x4065:
			ret = SsVabTransCompleted(*((u16 *)data + 2));
			break;
		case 0x4066:
			SsVabTransBody(*((u8 **)data + 1), *((u16 *)data + 4));
			break;
		case 0x4067:
			ret = SsVoiceCheck(*((u32 *)data + 1), *((u32 *)data + 2), *((u16 *)data + 6));
			break;
		case 0x4068:
			ret = SsVoKeyOff(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x4069:
			ret = SsVoKeyOn(*((u32 *)data + 1), *((u32 *)data + 2), *((u16 *)data + 6), *((u16 *)data + 8));
			break;
		case 0x4070:
			ret = SsVabOpenHeadSticky(*((u8 **)data + 1), *((u16 *)data + 4), *((u32 *)data + 3));
			break;
#if 0
		case 0x5001:
			StInit();
			break;
		case 0x5002:
			StQuit();
			break;
		case 0x5003:
			StCalledVSync();
			break;
		case 0x5005:
			ret = (s16)StVabOpen(*((s16 **)data + 1), *((u32 *)data + 2), *((u32 *)data + 3));
			break;
		case 0x5006:
			ret = (s16)StVabOpenFakeBody(*((u32 *)data + 1), *((u32 *)data + 2));
			break;
		case 0x5007:
			StVabOpenCompleted();
			break;
		case 0x5008:
			ret = (s16)StVabClose(*((u16 *)data + 2));
			break;
		case 0x5009:
			ret = (s16)StBgmOpen(*((u16 *)data + 2), *((u32 *)data + 2));
			break;
		case 0x500A:
			StSetTickMode(*((u16 *)data + 2));
			break;
		case 0x500B:
			ret = (s16)StBgmClose(*((u16 *)data + 2));
			break;
		case 0x500C:
			StSetReverbType(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x500D:
			StSetReverbDepth(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x500E:
			StSetReverbDelaytime(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x500F:
			StSetReverbFeedback(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x5010:
			ret = StGetSlotStatus(*((u16 *)data + 2));
			break;
		case 0x5011:
			StSetSbClear(*((u32 *)data + 1));
			break;
		case 0x5012:
			StSetMasterVol(*((u16 *)data + 2), *((u16 *)data + 4), *((u16 *)data + 6));
			break;
		case 0x5013:
			ret = (s16)StSetBgmVol(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x5014:
			StBgmPlay(*((u16 *)data + 2));
			break;
		case 0x5015:
			StBgmStop(*((u16 *)data + 2), *((u16 *)data + 4), *((u32 *)data + 3));
			break;
		case 0x5016:
			StSetBgmTempo(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x5017:
			ret = (s16)StGetBgmTempo(*((u16 *)data + 2));
			break;
		case 0x5018:
			ret = (s16)StGetBgmStatus();
			break;
		case 0x5019:
			ret = (s16)StGetBgmChStatus();
			break;
		case 0x501A:
			ret = (s16)StDmaWrite(*((s16 **)data + 1), *((u32 *)data + 2), *((u32 *)data + 3));
			break;
		case 0x501B:
			ret = (s16)StDmaRead(*((u32 *)data + 1), *((u8 **)data + 2), *((u32 *)data + 3));
			break;
		case 0x5100:
			ret = SetTimer(&common);
			break;
		case 0x5101:
			ret = ReleaseTimer();
			break;
		case 0x5200:
			ret = StSePlay(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
		case 0x5201:
			ret = StSetSeVol(*((u16 *)data + 2), *((u16 *)data + 4));
			break;
#endif
		case 0x6128:
		{
			SpuGetCommonAttr(&c_attr);
			return &c_attr;
		}
		case 0x6240:
		{
			s_attr.voice = *((u32 *)data + 2);
			SpuGetVoiceAttr(&s_attr);
			return &s_attr;
		}
		case 0x6314:
		{
			SpuGetReverbModeParam(&r_attr);
			return &r_attr;
		}
		case 0x6418:
		{
			SpuGetAllKeysStatus(status);
			return status;
		}
		case 0x7128:
			SpuSetCommonAttr((SpuCommonAttr *)data);
			break;
		case 0x7240:
			SpuSetVoiceAttr((SpuVoiceAttr *)data);
			break;
		case 0x7314:
			SpuSetReverbModeParam((SpuReverbAttr *)data);
			break;
		case 0x7440:
			SpuSetKeyOnWithAttr((SpuVoiceAttr *)data);
			break;
		case 0x7508:
			SpuSetEnv((const SpuEnv *)data);
			break;
		case 0x7600:
		{
			int i;

			for ( i = 0; i < gMultiVoiceNum; i += 1 )
				SpuSetVoiceAttr((SpuVoiceAttr *)data + i);
			break;
		}
		case 0x8100:
			SpuSetTransferCallback(DMA1CallBackProc);
			break;
		case 0x8200:
			SpuSetIRQCallback(IRQCallBackProc);
			break;
		case 0x8600:
			SpuAutoDMASetCallback(DMA0CallBackProc);
			break;
		case 0xE621:
		{
			iop_thread_t param;

			param.attr = TH_C;
			param.thread = sce_spust_loop;
			param.priority = 34;
#ifdef LIB_OSD_100
			param.stacksize = 2048;
#else
			param.stacksize = 4096;
#endif
			param.option = 0;
			gStThid = CreateThread(&param);
			StartThread(gStThid, 0);
			break;
		}
	}

	return &ret;
}
