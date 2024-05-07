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

void SsUtAllKeyOff(s16 mode)
{
	int vc_index;
	SpuVoiceAttr voice_attr;

	(void)mode;

	voice_attr.mask =
		SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA | SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
	voice_attr.pitch = 0x1000;
	voice_attr.addr = 0x5000;
	voice_attr.adsr1 = 0x80FF;
	voice_attr.volume.left = 0;
	voice_attr.volume.right = 0;
	voice_attr.adsr2 = 0x4000;
	for ( vc_index = 0; vc_index < _SsVmMaxVoice; vc_index += 1 )
	{
		if ( (_snd_vmask & (1 << vc_index)) == 0 )
		{
			libsnd2_spu_voice_t *voice_struct;

			voice_struct = &_svm_voice[vc_index];
			voice_struct->m_unk02 = 24;
			voice_struct->m_key_stat = 0;
			voice_struct->m_seq_sep_no = 255;
			voice_struct->m_fake_program = 0;
			voice_struct->m_prog = 0;
			voice_struct->m_tone = 255;
			voice_struct->m_voll2 = 0;
			voice_attr.voice = 1 << vc_index;
			SpuSetVoiceAttr(&voice_attr);
			_svm_cur.m_voice_idx = vc_index;
			_SsVmKeyOffNow();
		}
	}
}
