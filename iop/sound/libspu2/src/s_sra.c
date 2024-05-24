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

void _spu_setReverbAttr(const libspu2_reverb_param_entry_t *p_rev_param_entry)
{
	u32 flags;

	flags = p_rev_param_entry->flags;
	if ( flags == 0 )
		flags = 0xFFFFFFFF;
	if ( (flags & 1) != 0 )
		_spu_MGFsetRXX2(370, p_rev_param_entry->dAPF1);
	if ( (flags & 2) != 0 )
		_spu_MGFsetRXX2(372, p_rev_param_entry->dAPF2);
	if ( (flags & 4) != 0 )
		_spu_RXX[20 * _spu_core + 954] = p_rev_param_entry->vIIR;
	if ( (flags & 8) != 0 )
		_spu_RXX[20 * _spu_core + 955] = p_rev_param_entry->vCOMB1;
	if ( (flags & 0x10) != 0 )
		_spu_RXX[20 * _spu_core + 956] = p_rev_param_entry->vCOMB2;
	if ( (flags & 0x20) != 0 )
		_spu_RXX[20 * _spu_core + 957] = p_rev_param_entry->vCOMB3;
	if ( (flags & 0x40) != 0 )
		_spu_RXX[20 * _spu_core + 958] = p_rev_param_entry->vCOMB4;
	if ( (flags & 0x80) != 0 )
		_spu_RXX[20 * _spu_core + 959] = p_rev_param_entry->vWALL;
	if ( (flags & 0x100) != 0 )
		_spu_RXX[20 * _spu_core + 960] = p_rev_param_entry->vAPF1;
	if ( (flags & 0x200) != 0 )
		_spu_RXX[20 * _spu_core + 961] = p_rev_param_entry->vAPF2;
	if ( (flags & 0x400) != 0 )
		_spu_MGFsetRXX2(374, p_rev_param_entry->mLSAME);
	if ( (flags & 0x800) != 0 )
		_spu_MGFsetRXX2(376, p_rev_param_entry->mRSAME);
	if ( (flags & 0x1000) != 0 )
		_spu_MGFsetRXX2(378, p_rev_param_entry->mLCOMB1);
	if ( (flags & 0x2000) != 0 )
		_spu_MGFsetRXX2(380, p_rev_param_entry->mRCOMB1);
	if ( (flags & 0x4000) != 0 )
		_spu_MGFsetRXX2(382, p_rev_param_entry->mLCOMB2);
	if ( (flags & 0x8000) != 0 )
		_spu_MGFsetRXX2(384, p_rev_param_entry->mRCOMB2);
	if ( (flags & 0x10000) != 0 )
		_spu_MGFsetRXX2(386, p_rev_param_entry->dLSAME);
	if ( (flags & 0x20000) != 0 )
		_spu_MGFsetRXX2(388, p_rev_param_entry->dRSAME);
	if ( (flags & 0x40000) != 0 )
		_spu_MGFsetRXX2(390, p_rev_param_entry->mLDIFF);
	if ( (flags & 0x80000) != 0 )
		_spu_MGFsetRXX2(392, p_rev_param_entry->mRDIFF);
	if ( (flags & 0x100000) != 0 )
		_spu_MGFsetRXX2(394, p_rev_param_entry->mLCOMB3);
	if ( (flags & 0x200000) != 0 )
		_spu_MGFsetRXX2(396, p_rev_param_entry->mRCOMB3);
	if ( (flags & 0x400000) != 0 )
		_spu_MGFsetRXX2(398, p_rev_param_entry->mLCOMB4);
	if ( (flags & 0x800000) != 0 )
		_spu_MGFsetRXX2(400, p_rev_param_entry->mRCOMB4);
	if ( (flags & 0x1000000) != 0 )
		_spu_MGFsetRXX2(402, p_rev_param_entry->dLDIFF);
	if ( (flags & 0x2000000) != 0 )
		_spu_MGFsetRXX2(404, p_rev_param_entry->dRDIFF);
	if ( (flags & 0x4000000) != 0 )
		_spu_MGFsetRXX2(406, p_rev_param_entry->mLAPF1);
	if ( (flags & 0x8000000) != 0 )
		_spu_MGFsetRXX2(408, p_rev_param_entry->mRAPF1);
	if ( (flags & 0x10000000) != 0 )
		_spu_MGFsetRXX2(410, p_rev_param_entry->mLAPF2);
	if ( (flags & 0x20000000) != 0 )
		_spu_MGFsetRXX2(412, p_rev_param_entry->mRAPF2);
	if ( (flags & 0x40000000) != 0 )
		_spu_RXX[20 * _spu_core + 962] = p_rev_param_entry->vLIN;
	if ( (flags & 0x80000000) != 0 )
		_spu_RXX[20 * _spu_core + 963] = p_rev_param_entry->vRIN;
}
