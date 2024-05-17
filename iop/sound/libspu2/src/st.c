/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libspu2_internal.h"

static unsigned int _st_core = 0u;
static int _spu_st_stat_int = 0;
static int _spu_st_start_voice_bit = 0;
static int _spu_st_start_tmp_voice_bit = 0;
static int _spu_st_start_add_voice_bit = 0;
static int _spu_st_start_prepare_voice_bit = 0;
static int _spu_st_start_prepare_tmp_voice_bit = 0;
static int _spu_st_start_prepared_voice_bit = 0;
static int _spu_st_stop_voice_bit = 0;
static int _spu_st_stop_saved_voice_bit = 0;
static int _spu_st_tmp_voice_bit = 0;

static SpuStEnv _spu_st_Info;
static u32 _spu_st_buf_sizeSBhalf;
static int _spu_st_save_final_block[98];
static int _spu_st_start_prepare_voice_current;
static SpuTransferCallbackProc _spu_st_cb_transfer_saved;
static SpuStCallbackProc _spu_st_cb_prepare_finished;
static int _spu_st_start_prepare_lock;
static int _spu_st_bufferP;
static int _spu_st_start_voice_smallest;
static int _spu_st_stop_voice_smallest;
static SpuIRQCallbackProc _spu_st_cb_IRQ_saved;
static SpuStCallbackProc _spu_st_cb_stream_finished;
static unsigned int _spu_st_addrIRQ;
static int _spu_st_start_tmp_voice_current;
static int _spu_st_start_add_lock;
static SpuStCallbackProc _spu_st_cb_transfer_finished;
static int _spu_st_start_prepare_voice_smallest;

static void _SpuStSetMarkSTART(int voice)
{
	u8 *v1;

	v1 = (u8 *)(_spu_st_Info.voice[voice].data_addr + 1);
	*v1 = 6;
	v1[16] = 2;
	v1[_spu_st_buf_sizeSBhalf - 16] = 2;
	FlushDcache();
}

static void _SpuStSetMarkEND(int voice)
{
	u8 *v1;

	v1 = (u8 *)(_spu_st_Info.voice[voice].data_addr + 1);
	*v1 = 2;
	v1[16] = 2;
	v1[_spu_st_buf_sizeSBhalf - 16] = 3;
	FlushDcache();
}

static void _SpuStSetMarkFINAL(int voice)
{
	unsigned int v1;
	int *v2;
	u8 *v3;
	int v4;
	u8 *v5;
	u8 *v6;

	v1 = 16 * voice;
	v2 = &_spu_st_save_final_block[v1 / 4];
	v3 = (u8 *)(_spu_st_Info.voice[v1 / 0x10].data_addr + _spu_st_Info.voice[v1 / 0x10].last_size - 16);
	*(u8 *)v2 = *v3;
	*v3 = 0;
	v3 += 1;
	v2 = (int *)((char *)v2 + 1);
	*(u8 *)v2 = *v3;
	v5 = (u8 *)v2 + 1;
	*v3 = 7;
	v6 = v3 + 1;
	for ( v4 = 0; v4 < 14; v4 += 1 )
	{
		v5[v4] = v6[v4];
		v6[v4] = 0;
	}
	FlushDcache();
}

static void _SpuStSetMarkFINALrecover(int voice)
{
	unsigned int v1;
	int *v2;
	int v3;
	u8 *v4;

	v1 = 16 * voice;
	v2 = &_spu_st_save_final_block[v1 / 4];
	v4 = (u8 *)(_spu_st_Info.voice[v1 / 0x10].data_addr + _spu_st_Info.voice[v1 / 0x10].last_size - 16);
	for ( v3 = 0; v3 < 16; v3 += 1 )
	{
		v4[v3] = ((u8 *)v2)[v3];
	}
	FlushDcache();
}

static SpuStVoiceAttr *_SpuStSetPrepareEnv(int voice)
{
	SpuStVoiceAttr *v2;

	v2 = &_spu_st_Info.voice[voice];
	_spu_t(2, 8 * (v2->buf_addr >> 4));
	_SpuStSetMarkSTART(voice);
	_spu_t(1);
	return v2;
}

void _SpuStCBPrepare(void)
{
	int v0;

	v0 = _spu_st_start_prepare_voice_bit & ~(1 << _spu_st_start_prepare_voice_current);
	_spu_st_start_prepare_voice_bit = v0;
	if ( v0 )
	{
		int v1;

		for ( v1 = _spu_st_start_prepare_voice_current + 1; v1 < 24; v1 += 1 )
		{
			if ( (v0 & (1 << v1)) != 0 )
				break;
		}
		_spu_st_start_prepare_voice_current = v1;
		if ( v1 < 24 )
		{
			_spu_t(3, _SpuStSetPrepareEnv(v1)->data_addr, _spu_st_buf_sizeSBhalf);
		}
	}
	else
	{
		SpuSetTransferCallback(_spu_st_cb_transfer_saved);
		if ( _spu_st_Info.low_priority == SPU_ON )
			_spu_FsetPCR(1);
		_spu_st_stat_int = 34;
		if ( _spu_st_cb_prepare_finished )
		{
			_spu_st_cb_prepare_finished(_spu_st_start_prepare_tmp_voice_bit, SPU_ST_PREPARE);
			_spu_st_start_prepare_tmp_voice_bit = 0;
		}
	}
	FlushDcache();
}

static int _SpuStStartPrepare(int voice_bit)
{
	int v1;
	int v3;

	for ( v1 = 0; v1 < 24; v1 += 1 )
	{
		if ( (voice_bit & (1 << v1)) != 0 )
			break;
	}
	v3 = _spu_st_stat_int & 0xF0;
	if ( v3 == 32 )
		return SPU_ST_WRONG_STATUS;
	if ( (_spu_st_stat_int & 0xF0u) >= 0x21 )
	{
		if ( v3 == 48 )
		{
			_spu_st_start_prepare_tmp_voice_bit = voice_bit;
			_spu_st_start_prepare_voice_bit = voice_bit;
			_spu_st_start_prepare_voice_current = v1;
			_spu_st_start_prepare_lock = 0;
			return SPU_ST_ACCEPT;
		}
	}
	else
	{
		if ( v3 == 16 )
		{
			unsigned int data_addr;

			_spu_st_stat_int = 32;
			_spu_st_start_prepare_tmp_voice_bit = voice_bit;
			_spu_st_start_prepare_voice_bit = voice_bit;
			_spu_st_buf_sizeSBhalf = _spu_st_Info.size >> 1;
			_spu_st_cb_transfer_saved = SpuSetTransferCallback(_SpuStCBPrepare);
			if ( _spu_st_Info.low_priority == SPU_ON )
				_spu_FsetPCR(0);
			_spu_st_bufferP = 0;
			_spu_st_start_prepare_voice_current = v1;
			data_addr = _SpuStSetPrepareEnv(v1)->data_addr;
			_spu_st_stat_int = 33;
			_spu_t(3, data_addr, _spu_st_buf_sizeSBhalf);
			return SPU_ST_ACCEPT;
		}
	}
	return SPU_ST_WRONG_STATUS;
}

int IntFunc(void)
{
	Kprintf("*********------------\n");
	Kprintf("*********------------\n");
	Kprintf("*********------------\n");
	Kprintf("*********------------\n");
	Kprintf("*********------------\n");
	FlushDcache();
	return 1;
}

static SpuStVoiceAttr *_SpuStSetTransferEnv(int voice)
{
	SpuStVoiceAttr *v2;
	int v3;

	v2 = &_spu_st_Info.voice[voice];
	v3 = (v2->buf_addr >> 4) << 4;
	if ( _spu_st_bufferP )
	{
		v3 += _spu_st_buf_sizeSBhalf;
		_SpuStSetMarkEND(voice);
	}
	else
	{
		_SpuStSetMarkSTART(voice);
	}
	_spu_t(2, v3 >> 1);
	if ( v2->status == 2 )
	{
		int v4;

		_spu_st_start_voice_bit &= ~(1 << voice);
		_spu_st_stop_voice_bit |= 1 << voice;
		_SpuStSetMarkFINAL(voice);
		if ( _spu_st_start_voice_bit )
		{
			for ( v4 = 0; v4 < 24; v4 += 1 )
			{
				if ( (_spu_st_start_voice_bit & (1 << v4)) != 0 )
					break;
			}
			_spu_st_start_voice_smallest = v4;
		}
		else
		{
			_spu_st_start_voice_smallest = 24;
			for ( v4 = 0; v4 < 24; v4 += 1 )
			{
				if ( (_spu_st_stop_voice_bit & (1 << v4)) != 0 )
					break;
			}
			_spu_st_stop_voice_smallest = v4;
		}
	}
	_spu_t(1);
	return v2;
}

void _SpuStCB_IRQfinal(void)
{
	unsigned int v0;

	v0 = SpuSetCore(_st_core);
	SpuSetIRQ(SPU_OFF);
	SpuSetCore(v0);
	_spu_st_stat_int = 67;
	SpuSetIRQCallback(_spu_st_cb_IRQ_saved);
	SpuSetTransferCallback(_spu_st_cb_transfer_saved);
	if ( _spu_st_Info.low_priority == SPU_ON )
		_spu_FsetPCR(1);
	if ( _spu_st_cb_stream_finished && _spu_st_stop_voice_bit )
		_spu_st_cb_stream_finished(_spu_st_stop_voice_bit, SPU_ST_FINAL);
	_spu_st_stop_voice_bit = 0;
	_spu_st_stop_saved_voice_bit = 0;
	_spu_st_stat_int = 16;
}

static void _SpuStCB_IRQ(void)
{
	int v0;
	unsigned int v1;
	int v2;
	int v3;

	FlushDcache();
	v0 = -1;
	v1 = SpuSetCore(_st_core);
	SpuSetIRQ(SPU_OFF);
	SpuSetCore(v1);
	v2 = 0;
	if ( (_spu_st_stat_int & 0xF0) == 64 )
		v3 = 66;
	else
		v3 = 51;
	_spu_st_stat_int = v3;
	if ( _spu_st_stop_voice_bit )
	{
		int v4;

		for ( v4 = 0; v4 < 24; v4 += 1 )
		{
			if ( (_spu_st_stop_voice_bit & (1 << v4)) != 0 )
			{
				int last_size;
				unsigned int v8;
				int v9;

				last_size = _spu_st_Info.voice[v4].last_size;
#ifdef LIB_1300
				v8 = ((_spu_st_Info.voice[v4].buf_addr >> 4) << 4) + last_size - 8;
#else
				// Added in OSDSND 110U
				v8 = ((_spu_st_Info.voice[v4].buf_addr >> 4) << 4) + last_size - 16;
#endif
				if ( !_spu_st_bufferP )
					v8 += _spu_st_buf_sizeSBhalf;
				v9 = SpuSetCore(_st_core);
				_spu_FsetRXX(226 + (v4 * 6), (v8 >> 4) << 4, 1);
				_spu_core = v9;
				if ( v2 < last_size )
				{
					v2 = last_size;
					v0 = v4;
				}
			}
		}
	}
	if ( !_spu_st_start_voice_bit )
	{
		unsigned int v10;
		unsigned int v11;

#ifdef LIB_1300
		if ( v2 < 9 )
#else
		// Added in OSDSND 110U
		if ( v2 < 17 )
#endif
		{
			_SpuStCB_IRQfinal();
			return;
		}
		SpuSetIRQCallback(_SpuStCB_IRQfinal);
#ifdef LIB_1300
		v10 = ((_spu_st_Info.voice[v0].buf_addr >> 4) << 4) + v2 - 8;
#else
		// Added in OSDSND 110U
		v10 = ((_spu_st_Info.voice[v0].buf_addr >> 4) << 4) + v2 - 16;
#endif
		_spu_st_addrIRQ = v10;
		if ( !_spu_st_bufferP )
			_spu_st_addrIRQ = v10 + _spu_st_buf_sizeSBhalf;
		v11 = SpuSetCore(_st_core);
		SpuSetIRQAddr(_spu_st_addrIRQ);
		SpuSetCore(v11);
	}
	if ( _spu_st_cb_stream_finished && _spu_st_stop_saved_voice_bit )
		_spu_st_cb_stream_finished(_spu_st_stop_saved_voice_bit, SPU_ST_PLAY);
	if ( _spu_st_start_voice_bit )
	{
		_spu_st_start_tmp_voice_bit = _spu_st_start_voice_bit;
		_spu_st_stop_saved_voice_bit = _spu_st_stop_voice_bit;
		_spu_st_start_tmp_voice_current = _spu_st_start_voice_smallest;
		if ( _spu_st_bufferP )
		{
			if ( _spu_st_start_add_voice_bit && _spu_st_start_prepared_voice_bit && !_spu_st_start_add_lock )
			{
				int v16;
				unsigned int v17;
				int v18;
				int v20;

				v16 = _spu_st_start_add_voice_bit & _spu_st_start_prepared_voice_bit;
				v17 = SpuSetCore(_st_core);
				SpuSetKey(SPU_ON, v16);
				SpuSetCore(v17);
				if ( (_spu_env & 1) != 0 )
				{
					unsigned int v19;

					v19 = SpuSetCore(_st_core);
					SpuFlush(SPU_EVENT_KEY);
					SpuSetCore(v19);
				}
				_spu_st_start_prepared_voice_bit = 0;
				_spu_st_start_add_voice_bit = 0;
				v20 = _spu_st_start_voice_bit | v16;
				_spu_st_start_voice_bit = v20;
				_spu_st_start_tmp_voice_bit = v20;
				for ( v18 = 0; v18 < 24; v18 += 1 )
				{
					if ( (v20 & (1 << v18)) != 0 )
						break;
				}
				_spu_st_start_tmp_voice_current = v18;
				_spu_st_start_voice_smallest = v18;
			}
		}
		else
		{
			int v13;

			v13 = _spu_st_start_prepare_voice_bit;
			if ( _spu_st_start_prepare_voice_bit && !_spu_st_start_prepare_lock )
			{
				int v14;

				_spu_st_start_tmp_voice_bit = _spu_st_start_voice_bit | _spu_st_start_prepare_voice_bit;
				_spu_st_start_prepared_voice_bit = _spu_st_start_prepare_voice_bit;
				_spu_st_start_prepare_tmp_voice_bit = _spu_st_start_prepare_voice_bit;
				_spu_st_start_prepare_voice_bit = 0;
				for ( v14 = 0; v14 < 24; v14 += 1 )
				{
					if ( ((_spu_st_start_voice_bit | v13) & (1 << v14)) != 0 )
						break;
				}
				_spu_st_start_tmp_voice_current = v14;
			}
		}
		_spu_st_stop_voice_bit = 0;
		_spu_st_stat_int = 49;
		_spu_t(3, _SpuStSetTransferEnv(_spu_st_start_tmp_voice_current)->data_addr, _spu_st_buf_sizeSBhalf);
	}
	else
	{
		unsigned int v12;

		_spu_st_stat_int = 65;
		v12 = SpuSetCore(_st_core);
		SpuSetIRQ(SPU_ON);
		SpuSetCore(v12);
	}
	FlushDcache();
}

static void _SpuStCB_Transfer(void)
{
	SpuStVoiceAttr *v0;
	unsigned int v1;

	FlushDcache();
	_spu_st_start_tmp_voice_bit &= ~(1 << _spu_st_start_tmp_voice_current);
	v0 = &_spu_st_Info.voice[_spu_st_start_tmp_voice_current];
	if ( v0->status == 2 )
	{
		_SpuStSetMarkFINALrecover(_spu_st_start_tmp_voice_current);
		v0->status = 6;
	}
	v1 = SpuSetCore(_st_core);
	_spu_FsetRXX(6 * _spu_st_start_tmp_voice_current + 226, (v0->buf_addr >> 4) << 4, 1);
	SpuSetCore(v1);
	if ( _spu_st_start_tmp_voice_bit )
	{
		int v6;

		for ( v6 = _spu_st_start_tmp_voice_current + 1; v6 < 24; v6 += 1 )
		{
			if ( (_spu_st_start_tmp_voice_bit & (1 << v6)) != 0 )
				break;
		}
		_spu_st_start_tmp_voice_current = v6;
		_spu_t(3, _SpuStSetTransferEnv(v6)->data_addr, _spu_st_buf_sizeSBhalf);
	}
	else
	{
		int v2;
		unsigned int v3;
		unsigned int v4;
		unsigned int v5;

		if ( !_spu_st_bufferP && _spu_st_cb_prepare_finished && _spu_st_start_prepare_tmp_voice_bit )
		{
			_spu_st_cb_prepare_finished(_spu_st_start_prepare_tmp_voice_bit, SPU_ST_PLAY);
			_spu_st_start_prepare_tmp_voice_bit = 0;
		}
		if ( _spu_st_cb_transfer_finished && _spu_st_start_voice_bit )
			_spu_st_cb_transfer_finished(_spu_st_start_voice_bit, SPU_ST_PLAY);
		v2 = _spu_st_stop_voice_smallest;
		if ( _spu_st_start_voice_smallest < 24 )
			v2 = _spu_st_start_voice_smallest;
		v3 = (_spu_st_Info.voice[v2].buf_addr >> 4) << 4;
		_spu_st_addrIRQ = v3;
		_spu_st_bufferP = _spu_st_bufferP != 1;
		if ( _spu_st_bufferP != 1 )
			_spu_st_addrIRQ = v3 + _spu_st_buf_sizeSBhalf;
		v4 = SpuSetCore(_st_core);
		SpuSetIRQAddr(_spu_st_addrIRQ);
		SpuSetCore(v4);
		v5 = SpuSetCore(_st_core);
		SpuSetIRQ(SPU_ON);
		SpuSetCore(v5);
		_spu_st_stat_int = 64;
		if ( _spu_st_start_voice_smallest < 24 )
			_spu_st_stat_int = 50;
	}
	FlushDcache();
}

static int _SpuStStart(unsigned int voice_bit)
{
	FlushDcache();
	if ( (_spu_st_stat_int & 0xF0) == 32 )
	{
		if ( _spu_st_stat_int == 34 )
		{
			int v3;
			unsigned int v5;
			const SpuStVoiceAttr *v6;
			unsigned int v7;

			_spu_st_stat_int = 48;
			for ( v3 = 0; v3 < 24; v3 += 1 )
			{
				if ( (voice_bit & (1 << v3)) != 0 )
					break;
			}
			v5 = SpuSetCore(_st_core);
			SpuSetKey(SPU_ON, voice_bit);
			SpuSetCore(v5);
			if ( (_spu_env & 1) != 0 )
				SpuFlush(SPU_EVENT_KEY);
			_spu_st_start_voice_bit = voice_bit;
			_spu_st_start_tmp_voice_bit = voice_bit;
			_spu_st_stop_saved_voice_bit = 0;
			_spu_st_stop_voice_bit = 0;
			_spu_st_cb_transfer_saved = SpuSetTransferCallback(_SpuStCB_Transfer);
			if ( _spu_st_Info.low_priority == SPU_ON )
				_spu_FsetPCR(0);
			_spu_st_cb_IRQ_saved = SpuSetIRQCallback(_SpuStCB_IRQ);
			_spu_st_bufferP = 1;
			_spu_st_start_tmp_voice_current = v3;
			_spu_st_start_voice_smallest = v3;
			v6 = _SpuStSetTransferEnv(v3);
			v7 = SpuSetCore(_st_core);
			SpuSetIRQ(SPU_OFF);
			SpuSetCore(v7);
			_spu_st_stat_int = 49;
			_spu_t(3, v6->data_addr, _spu_st_buf_sizeSBhalf);
			FlushDcache();
			return SPU_ST_ACCEPT;
		}
	}
	else
	{
		if ( (_spu_st_stat_int & 0xF0u) >= 0x21 )
		{
			if ( (_spu_st_stat_int & 0xF0) == 48 )
			{
				_spu_st_start_add_voice_bit = voice_bit;
				_spu_st_start_add_lock = 0;
				return SPU_ST_ACCEPT;
			}
		}
	}
	return SPU_ST_WRONG_STATUS;
}

int SpuStTransfer(int flag, unsigned int voice_bit)
{
	unsigned int v4;

	if ( !_spu_st_stat_int )
		return 0;
	v4 = voice_bit & 0xFFFFFF;
	if ( (voice_bit & 0xFFFFFF) == 0 )
	{
		return SPU_ST_INVALID_ARGUMENT;
	}
	if ( flag == SPU_ST_PREPARE )
	{
		return _SpuStStartPrepare(v4);
	}
	if ( flag >= SPU_ST_PREPARE && flag <= SPU_ST_PLAY )
	{
		return _SpuStStart(v4);
	}
	return SPU_ST_INVALID_ARGUMENT;
}

static void _SpuStReset(void)
{
	int v0;

	_spu_st_cb_transfer_finished = 0;
	_spu_st_cb_prepare_finished = 0;
	_spu_st_cb_stream_finished = 0;
	_spu_st_cb_transfer_saved = 0;
	_spu_st_cb_IRQ_saved = 0;
	_spu_st_start_voice_bit = 0;
	_spu_st_start_tmp_voice_bit = 0;
	_spu_st_start_add_voice_bit = 0;
	_spu_st_start_prepare_voice_bit = 0;
	_spu_st_start_prepare_tmp_voice_bit = 0;
	_spu_st_start_prepared_voice_bit = 0;
	_spu_st_stop_voice_bit = 0;
	_spu_st_stop_saved_voice_bit = 0;
	_spu_st_tmp_voice_bit = 0;
	_spu_st_start_voice_smallest = 24;
	_spu_st_start_tmp_voice_current = 24;
	_spu_st_start_prepare_voice_current = 24;
	_spu_st_start_prepare_voice_smallest = 24;
	_spu_st_stop_voice_smallest = 24;
	for ( v0 = 0; v0 < 24; v0 += 1 )
	{
		_spu_st_Info.voice[v0].status = 6;
		_spu_st_Info.voice[v0].last_size = 0;
		_spu_st_Info.voice[v0].buf_addr = 0;
		_spu_st_Info.voice[v0].data_addr = 0;
	}
	_spu_st_Info.size = 0;
	_spu_st_Info.low_priority = SPU_OFF;
	_spu_st_buf_sizeSBhalf = 0;
	_spu_st_bufferP = 0;
	_spu_st_start_prepare_lock = 0;
	_spu_st_start_add_lock = 0;
}

SpuStEnv *SpuStInit(int mode)
{
	(void)mode;

	_spu_st_stat_int = 16;
	_SpuStReset();
	return &_spu_st_Info;
}

int SpuStQuit(void)
{
	if ( _spu_st_stat_int != 16 )
		return SPU_ST_WRONG_STATUS;
	_spu_st_stat_int = 0;
	_SpuStReset();
	return SPU_ST_ACCEPT;
}

int SpuStGetStatus(void)
{
	switch ( _spu_st_stat_int & 0xF0 )
	{
		case 0:
			return SPU_ST_NOT_AVAILABLE;
		case 16:
			return SPU_ST_IDLE;
		case 32:
			return SPU_ST_PREPARE;
		case 48:
			return SPU_ST_TRANSFER;
		case 64:
			return SPU_ST_FINAL;
		default:
			return SPU_ST_WRONG_STATUS;
	}
}

unsigned int SpuStGetVoiceStatus(void)
{
	return _spu_st_start_voice_bit;
}

SpuStCallbackProc SpuStSetPreparationFinishedCallback(SpuStCallbackProc func)
{
	SpuStCallbackProc result;

	result = _spu_st_cb_prepare_finished;
	if ( func != _spu_st_cb_prepare_finished )
		_spu_st_cb_prepare_finished = func;
	return result;
}

SpuStCallbackProc SpuStSetTransferFinishedCallback(SpuStCallbackProc func)
{
	SpuStCallbackProc result;

	result = _spu_st_cb_transfer_finished;
	if ( func != _spu_st_cb_transfer_finished )
		_spu_st_cb_transfer_finished = func;
	return result;
}

SpuStCallbackProc SpuStSetStreamFinishedCallback(SpuStCallbackProc func)
{
	SpuStCallbackProc result;

	result = _spu_st_cb_stream_finished;
	if ( func != _spu_st_cb_stream_finished )
		_spu_st_cb_stream_finished = func;
	return result;
}

unsigned int SpuStSetCore(unsigned int core)
{
	unsigned int result;

	result = _st_core;
	_st_core = core & 1;
	return result;
}
