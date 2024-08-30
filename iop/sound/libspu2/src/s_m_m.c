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

int SpuMalloc(int size)
{
	int found_block_idx;
	u32 rev_size_zero;
	unsigned int size_adjusted;
	int p_allocated;
	libspu2_malloc_t *p_cur_block;
	u32 addr_area;
	u32 addr_area_1;
	u32 p_alloc_last_addr;
	libspu2_malloc_t *p_last_block_2;
	u32 addr_area_2;
	u32 size_area_1;
	libspu2_malloc_t *p_last_block_3;

	found_block_idx = -1;
	if ( _spu_rev_reserve_wa )
		rev_size_zero = 0x200000 - _spu_rev_offsetaddr;
	else
		rev_size_zero = 0;
	size_adjusted = 2 * (size >> 1);
	if ( (_spu_memList->addr_area & 0x40000000) != 0 )
	{
		found_block_idx = 0;
	}
	else
	{
		int cur_idx;

		_spu_gcSPU();
		for ( cur_idx = 0; cur_idx < _spu_AllocBlockNum; cur_idx += 1 )
		{
			if (
				!((_spu_memList[cur_idx].addr_area & 0x40000000) == 0
					&& ((_spu_memList[cur_idx].addr_area & 0x80000000) == 0 || _spu_memList[cur_idx].size_area < size_adjusted)) )
			{
				found_block_idx = cur_idx;
				break;
			}
		}
	}
	p_allocated = found_block_idx;
	if ( found_block_idx == -1 )
	{
		return -1;
	}
	p_cur_block = &_spu_memList[p_allocated];
	addr_area = _spu_memList[p_allocated].addr_area;
	if ( (addr_area & 0x40000000) != 0 )
	{
		libspu2_malloc_t *p_last_block_1;

		if ( found_block_idx >= _spu_AllocBlockNum )
		{
			return -1;
		}
		if ( p_cur_block->size_area - rev_size_zero < size_adjusted )
		{
			return -1;
		}
		p_last_block_1 = &_spu_memList[found_block_idx + 1];
		p_last_block_1->addr_area = ((p_cur_block->addr_area & 0xFFFFFFF) + size_adjusted) | 0x40000000;
		p_last_block_1->size_area = p_cur_block->size_area - size_adjusted;
		addr_area_1 = p_cur_block->addr_area;
		_spu_AllocLastNum = found_block_idx + 1;
		p_cur_block->size_area = size_adjusted;
		p_cur_block->addr_area = addr_area_1 & 0xFFFFFFF;
		_spu_gcSPU();
		return _spu_memList[p_allocated].addr_area;
	}
	else
	{
		unsigned int size_area;

		size_area = p_cur_block->size_area;
		if ( size_adjusted < size_area )
		{
			p_alloc_last_addr = addr_area + size_adjusted;
			if ( _spu_AllocLastNum < _spu_AllocBlockNum )
			{
				p_last_block_2 = &_spu_memList[_spu_AllocLastNum];
				addr_area_2 = p_last_block_2->addr_area;
				size_area_1 = p_last_block_2->size_area;
				p_last_block_2->addr_area = p_alloc_last_addr | 0x80000000;
				p_last_block_2->size_area = size_area - size_adjusted;
				_spu_AllocLastNum += 1;
				p_last_block_2[1].addr_area = addr_area_2;
				p_last_block_2[1].size_area = size_area_1;
			}
		}
		p_last_block_3 = &_spu_memList[found_block_idx];
		p_last_block_3->size_area = size_adjusted;
		p_last_block_3->addr_area = p_last_block_3->addr_area & 0xFFFFFFF;
		_spu_gcSPU();
		return _spu_memList[found_block_idx].addr_area;
	}
}
