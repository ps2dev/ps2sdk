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

s16 gVabOffet[16];

static int VsGetAddr(unsigned int size_in_bytes, int mode, s16 vab_id);

s16 SsVabOpenHeadSticky(u8 *addr, s16 vab_id, unsigned int sbaddr)
{
	return _SsVabOpenHeadWithMode(addr, vab_id, VsGetAddr, sbaddr);
}

s16 SsVabFakeHead(u8 *addr, s16 vab_id, unsigned int sbaddr)
{
	return _SsVabOpenHeadWithMode(addr, vab_id, VsGetAddr, sbaddr);
}

static int VsGetAddr(unsigned int size_in_bytes, int mode, s16 vab_id)
{
	(void)size_in_bytes;
	(void)vab_id;

	return mode;
}

int _SsVabOpenHeadWithMode(u8 *addr, int vab_id, libsnd2_vab_allocate_callback alloc_fn, int mode)
{
	int vab_id_tmp;
	const VabHdr *vab_hdr_ptr;
	unsigned int form_chk;
	ProgAtr *prog_atr_ptr;
	s16 maxPrograms;
	VagAtr *vag_attr_ptr1;
	int v21;
	ProgAtr *v24;
	int v27;
	VagAtr *vag_attr_ptr2;
	int v31;
	int v32;
	int vag_idx;
	int vag_lens[256];

	vab_id_tmp = 16;
	if ( _spu_getInTransfer() == 1 )
		return -1;
	_spu_setInTransfer(1);
	if ( (s16)vab_id < 16 )
	{
		if ( (s16)vab_id == -1 )
		{
			int v11;

			for ( v11 = 0; v11 < 16; v11 += 1 )
			{
				if ( _svm_vab_used[v11] == 0 )
				{
					_svm_vab_used[v11] = 1;
					vab_id_tmp = v11;
					_svm_vab_count += 1;
					break;
				}
			}
		}
		else
		{
			if ( _svm_vab_used[(s16)vab_id] == 0 )
			{
				_svm_vab_used[(s16)vab_id] = 1;
				vab_id_tmp = vab_id;
				_svm_vab_count += 1;
			}
		}
	}
	if ( vab_id_tmp >= 16 )
	{
		_spu_setInTransfer(0);
		return -1;
	}
	_svm_vab_count += 1;
	_svm_vab_vh[vab_id_tmp] = (VabHdr *)addr;
	vab_hdr_ptr = (VabHdr *)addr;
	form_chk = *(u32 *)addr;
	_svm_vab_not_send_size = 0;
	prog_atr_ptr = (ProgAtr *)(addr + 32);
	maxPrograms = 64;
	for ( ;; )
	{
		int fake_prog_idx;
		int total_vags_size;
		unsigned int rounded_size;
		unsigned int spu_alloc_mem;
		int total_vag_size_1;

		if ( form_chk >> 8 != 0x564142 )
		{
			break;
		}
		if ( (u8)form_chk == 112 )
		{
			if ( *((int *)addr + 1) >= 5 )
				maxPrograms = 128;
		}
		kMaxPrograms = maxPrograms;
		if ( maxPrograms < (int)*((u16 *)addr + 9) )
		{
			break;
		}
		_svm_vab_pg[vab_id_tmp] = prog_atr_ptr;
		vag_attr_ptr1 = (VagAtr *)&prog_atr_ptr[maxPrograms];
		fake_prog_idx = 0;
		for ( v21 = 0; v21 < maxPrograms; v21 += 1 )
		{
			v24 = &prog_atr_ptr[v21];
			v24->m_fake_prog_idx = fake_prog_idx;
			if ( v24->tones )
				fake_prog_idx += 1;
		}
		total_vags_size = 0;
		_svm_vab_tn[vab_id_tmp] = vag_attr_ptr1;
		vag_attr_ptr2 = &vag_attr_ptr1[16 * vab_hdr_ptr->ps];
		for ( v27 = 0; v27 < 256; v27 += 1 )
		{
			if ( vab_hdr_ptr->vs >= v27 )
			{
				v31 = *(u16 *)&vag_attr_ptr2->prior;
				v32 = 4 * v31;
				if ( vab_hdr_ptr->ver >= 5 )
					v32 = 8 * v31;
				vag_lens[v27] = v32;
				total_vags_size += vag_lens[v27];
			}
			vag_attr_ptr2 = (VagAtr *)((char *)vag_attr_ptr2 + 2);
		}
		rounded_size = (total_vags_size + 63) & ~63;
		spu_alloc_mem = alloc_fn(rounded_size, mode, (s16)vab_id_tmp);
		if ( spu_alloc_mem == 0xFFFFFFFF )
		{
			return -1;
		}
		gVabOffet[(s16)vab_id_tmp] = spu_alloc_mem > 0xFFFFF;
		if ( spu_alloc_mem + rounded_size > 0x1FAFF0 )
		{
			break;
		}
		_svm_vab_start[(s16)vab_id_tmp] = spu_alloc_mem;
		total_vag_size_1 = 0;
		for ( vag_idx = 0; vag_idx <= vab_hdr_ptr->vs; vag_idx += 1 )
		{
			total_vag_size_1 += vag_lens[vag_idx];
			if ( (vag_idx & 1) != 0 )
				prog_atr_ptr[vag_idx / 2].m_vag_spu_addr_lo = (spu_alloc_mem + total_vag_size_1) >> 4;
			else
				prog_atr_ptr[vag_idx / 2].m_vag_spu_addr_hi = (spu_alloc_mem + total_vag_size_1) >> 4;
		}
		_svm_vab_total[(s16)vab_id_tmp] = total_vag_size_1;
		_svm_vab_used[(s16)vab_id_tmp] = 2;
		return (s16)vab_id_tmp;
	}
	_svm_vab_used[vab_id_tmp] = 0;
	_spu_setInTransfer(0);
	_svm_vab_count -= 1;
	return -1;
}
