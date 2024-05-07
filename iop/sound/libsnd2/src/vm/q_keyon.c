/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "libsnd2_internal.h"

void SsQueueKeyOn(int voices)
{
	int v1;

	for ( v1 = 0; (char)v1 < _SsVmMaxVoice; v1 += 1 )
	{
		int v4;

		v4 = 1 << v1;
		if ( (v4 & voices) != 0 )
		{
			if ( v1 >= 16 )
			{
				int v7;

				v7 = 1 << (v1 - 16);
				_svm_okon2 |= v7;
				_svm_okof2 &= ~(u16)v7;
			}
			else
			{
				_svm_okon1 |= v4;
				_svm_okof1 &= ~(u16)v4;
			}
		}
	}
}

void SsQueueReverb(int voices, int reverb)
{
	int v2;
	int v8;
	char v9;
	int v10;

	for ( v2 = 0; (char)v2 < 24; v2 += 1 )
	{
		int v5;

		v5 = 1 << v2;
		if ( (v5 & voices) != 0 )
		{
			v8 = v5 & reverb;
			if ( v2 >= 16 )
			{
				v9 = v2 - 16;
				if ( v8 == 0 )
					v10 = _svm_orev2 & ~(1 << v9);
				else
					v10 = _svm_orev2 | (1 << v9);
				_svm_orev2 = v10;
			}
			else
			{
				if ( v8 )
					_svm_orev1 |= v5;
				else
					_svm_orev1 &= ~(u16)v5;
			}
		}
	}
}

void SsQueueRegisters(int vc, SndRegisterAttr *sra)
{
	u32 mask;

	mask = sra->mask;
	printf("SsQueueRegisters \n");
	if ( mask == 0 )
		mask = 0xFFFFFFFF;
	if ( (mask & SND_VOLL) != 0 )
	{
		_svm_sreg_buf[vc].m_vol_left = sra->volume.left & ~0x8000;
		_svm_sreg_dirty[vc] |= 1;
	}
	if ( (mask & SND_VOLR) != 0 )
	{
		_svm_sreg_buf[vc].m_vol_right = sra->volume.right & ~0x8000;
		_svm_sreg_dirty[vc] |= 2;
	}
	if ( (mask & SND_ADSR1) != 0 )
	{
		_svm_sreg_buf[vc].m_adsr1 = sra->adsr1;
		_svm_sreg_dirty[vc] |= 0x10;
	}
	if ( (mask & SND_ADSR2) != 0 )
	{
		_svm_sreg_buf[vc].m_adsr2 = sra->adsr2;
		_svm_sreg_dirty[vc] |= 0x20;
	}
	if ( (mask & SND_PITCH) != 0 )
	{
		_svm_sreg_buf[vc].m_pitch = sra->pitch;
		_svm_sreg_dirty[vc] |= 4;
	}
	if ( (mask & SND_ADDR) != 0 )
	{
		_svm_sreg_buf2[vc].m_vag_spu_addr = sra->addr;
		_svm_sreg_dirty[vc] |= 8u;
	}
}

s16 SsGetActualProgFromProg(s16 vab_id, s16 prog)
{
	if ( !((u16)vab_id < 0x11u && prog >= 0 && kMaxPrograms >= prog) )
		return -1;
	return _svm_vab_pg[vab_id][prog].m_fake_prog_idx;
}

void SsSetVoiceSettings(int vc, const SndVoiceStats *snd_v_attr)
{
	libsnd2_spu_voice_t *voice_struct;

	voice_struct = &_svm_voice[vc];
	voice_struct->m_vag_idx = snd_v_attr->vagId;
	voice_struct->m_pitch = snd_v_attr->pitch;
	voice_struct->m_voll1 = snd_v_attr->vol;
	voice_struct->m_pan = snd_v_attr->pan;
	voice_struct->m_seq_sep_no = 33;
	voice_struct->m_note = snd_v_attr->note;
	voice_struct->m_fake_program = snd_v_attr->prog_actual;
	voice_struct->m_prog = snd_v_attr->prog_num;
	voice_struct->m_tone = snd_v_attr->tone;
	voice_struct->m_vab_id = snd_v_attr->vabId;
	voice_struct->m_voll2 = snd_v_attr->vol;
}

s16 SsVoiceCheck(int vc, int vab_id, s16 note)
{
	const libsnd2_spu_voice_t *voice_struct;

	if ( (unsigned int)vc >= 0x18 )
		return -1;
	voice_struct = &_svm_voice[vc];
	if ( voice_struct->m_vab_id != vab_id >> 8 )
		return -1;
	if ( voice_struct->m_prog != (u8)vab_id )
		return -1;
	if ( voice_struct->m_note != note )
		return -1;
	return 0;
}