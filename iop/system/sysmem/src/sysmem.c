/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "sysmem.h"
#include "xsysmem.h"

extern struct irx_export_table _exp_sysmem;

#ifdef _IOP
IRX_ID("System_Memory_Manager", 2, 3);
#endif
// Based on the module from SCE SDK 3.1.0.

static sysmem_internals_t g_sysmem_internals;
static KprintfHandler_t *g_kprintf_cb;
static void *g_kprintf_cb_userdata;

extern int sysmem_reinit(void);
static int cCpuSuspendIntr(int *state);
static int cCpuResumeIntr(int state);
static void *allocSysMemory_internal(int flags, int size, const void *mem);
static int freeSysMemory_internal(const void *ptr);
static void updateSmemCtlBlk(void);
static sysmem_alloc_block_t *search_block(const void *address);

typedef struct intrman_callbacks_
{
	int (*cbCpuSuspendIntr)(int *state);
	int (*cbCpuResumeIntr)(int state);
	// cppcheck-suppress unusedStructMember
	int (*cbQueryIntrContext)(void);
} intrman_callbacks_t;

int _start(unsigned int memsize)
{
	memsize = (memsize > 0x7FFF00) ? 0x7FFF00 : memsize;
	g_sysmem_internals.m_alloclist =
		((memsize & 0xFFFFFF00) >= ((((uiptr)((&g_sysmem_internals))) + sizeof(g_sysmem_internals) + 255) >> 8 << 8)
																 + sizeof(sysmem_alloc_table_t)) ?
			(sysmem_alloc_table_t *)((((uiptr)((&g_sysmem_internals))) + sizeof(g_sysmem_internals) + 255) >> 8 << 8) :
			NULL;
	g_sysmem_internals.m_memsize = memsize & 0xFFFFFF00;
	g_sysmem_internals.m_intr_suspend_tbl = NULL;
	g_sysmem_internals.m_allocation_count = 0;
	g_sysmem_internals.m_smemupdate_cur = NULL;
	return g_sysmem_internals.m_alloclist ? sysmem_reinit() : 0;
}

int sysmem_reinit(void)
{
	sysmem_alloc_table_t *alloclist;
	unsigned int i;
	const sysmem_alloc_block_t *blklist;

	alloclist = g_sysmem_internals.m_alloclist;
	if ( !alloclist )
		return 0;
	alloclist->m_next = NULL;
	for ( i = 0; i < (sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])); i += 1 )
	{
		alloclist->m_blkarray[i].m_next = &(alloclist->m_blkarray[i + 1]);
		alloclist->m_blkarray[i].m_info.m_allocated = 0;
		alloclist->m_blkarray[i].m_info.m_address = 0;
		alloclist->m_blkarray[i].m_info.m_pad = 0;
		alloclist->m_blkarray[i].m_info.m_size = 0;
	}
	alloclist->m_blkarray[(sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])) - 1].m_next = NULL;
	alloclist->m_blkarray[0].m_info.m_size = g_sysmem_internals.m_memsize >> 8;
	g_sysmem_internals.m_allocation_count = 1;
	if (
		AllocSysMemory(ALLOC_FIRST, (int)alloclist, NULL) != NULL
		|| (sysmem_alloc_table_t *)AllocSysMemory(ALLOC_FIRST, sizeof(*alloclist) - sizeof(alloclist->m_padding), NULL)
				 != alloclist )
	{
		g_sysmem_internals.m_alloclist = NULL;
		return 0;
	}
	for ( blklist = alloclist->m_blkarray; blklist; blklist = blklist->m_next )
		if ( !blklist->m_info.m_allocated )
			return blklist->m_info.m_address << 8;
	return 0;
}

u32 QueryMemSize()
{
	return g_sysmem_internals.m_alloclist ? g_sysmem_internals.m_memsize : 0;
}

u32 QueryMaxFreeMemSize()
{
	unsigned int retval;
	const sysmem_alloc_block_t *blklist;
	int state;

	retval = 0;
	if ( !g_sysmem_internals.m_alloclist )
		return 0;
	cCpuSuspendIntr(&state);
	for ( blklist = g_sysmem_internals.m_alloclist->m_blkarray; blklist; blklist = blklist->m_next )
		if ( !blklist->m_info.m_allocated && retval < blklist->m_info.m_size )
			retval = blklist->m_info.m_size;
	cCpuResumeIntr(state);
	retval <<= 8;
	return retval;
}

u32 QueryTotalFreeMemSize()
{
	int retval;
	const sysmem_alloc_block_t *blklist;
	int state;

	if ( !g_sysmem_internals.m_alloclist )
		return 0;
	retval = 0;
	cCpuSuspendIntr(&state);
	for ( blklist = g_sysmem_internals.m_alloclist->m_blkarray; blklist; blklist = blklist->m_next )
		retval += !blklist->m_info.m_allocated ? blklist->m_info.m_size : 0;
	cCpuResumeIntr(state);
	retval <<= 8;
	return retval;
}

void *AllocSysMemory(int mode, int size, void *ptr)
{
	void *retval;
	int state;

	if ( !g_sysmem_internals.m_alloclist || (unsigned int)mode >= 3 )
		return NULL;
	cCpuSuspendIntr(&state);
	retval = allocSysMemory_internal(mode, size, ptr);
	updateSmemCtlBlk();
	cCpuResumeIntr(state);
	return retval;
}

int FreeSysMemory(void *ptr)
{
	const sysmem_alloc_table_t *alloclist;
	int retval;
	int state;

	for ( alloclist = g_sysmem_internals.m_alloclist; alloclist && ptr != alloclist; alloclist = alloclist->m_next )
		;
	if ( alloclist )
		return -1;
	cCpuSuspendIntr(&state);
	retval = freeSysMemory_internal(ptr);
	if ( !retval )
		updateSmemCtlBlk();
	cCpuResumeIntr(state);
	return retval;
}

void *QueryBlockTopAddress(void *address)
{
	int retval;
	const sysmem_alloc_block_t *blk;
	int state;

	cCpuSuspendIntr(&state);
	blk = search_block(address);
	retval = blk ? ((blk->m_info.m_address << 8) | (int)(!blk->m_info.m_allocated ? FREE : USED)) : -1;
	cCpuResumeIntr(state);
	return (void *)retval;
}

int QueryBlockSize(void *address)
{
	int retval;
	const sysmem_alloc_block_t *blk;
	int state;

	cCpuSuspendIntr(&state);
	blk = search_block(address);
	retval = blk ? ((blk->m_info.m_size << 8) | (int)(!blk->m_info.m_allocated ? FREE : USED)) : -1;
	cCpuResumeIntr(state);
	return retval;
}

void GetSysMemoryInfo(int flag, sysmem_info_t *info)
{
	int state;

	cCpuSuspendIntr(&state);
	if ( flag )
	{
		info->m_meminfo.m_allocation_count = g_sysmem_internals.m_allocation_count;
		info->m_meminfo.m_memsize = g_sysmem_internals.m_memsize;
		info->m_meminfo.m_memlist_last = g_sysmem_internals.m_smemupdate_cur;
		info->m_meminfo.m_memlist_first = (sysmem_alloc_table_t *)g_sysmem_internals.m_alloclist->m_blkarray;
		cCpuResumeIntr(state);
		return;
	}
	if ( info->m_meminfo.m_memlist_last != g_sysmem_internals.m_smemupdate_cur )
	{
		info->m_blockinfo.m_block_address = (void *)-1;
		info->m_blockinfo.m_flags_memsize = -1;
		info->m_blockinfo.m_table_info = NULL;
		cCpuResumeIntr(state);
		return;
	}
	if ( !info->m_blockinfo.m_table_info )
	{
		info->m_blockinfo.m_flags_memsize = 1;
		info->m_blockinfo.m_block_address = (void *)g_sysmem_internals.m_memsize;
		cCpuResumeIntr(state);
		return;
	}
	info->m_blockinfo.m_block_address =
		(void *)((((unsigned int)info->m_blockinfo.m_table_info->m_blkarray[0].m_next >> 1) & 0x7FFF) << 8);
	info->m_blockinfo.m_flags_memsize = (unsigned int)info->m_blockinfo.m_table_info->m_blkarray[0].m_next >> 17 << 8;
	if ( ((int)info->m_blockinfo.m_table_info->m_blkarray[0].m_next & 1) )
	{
		const sysmem_alloc_table_t *alloclist;

		for ( alloclist = g_sysmem_internals.m_alloclist; alloclist && alloclist != info->m_blockinfo.m_block_address;
					alloclist = alloclist->m_next )
			;
		info->m_blockinfo.m_flags_memsize |= alloclist ? 2 : 0;
	}
	else
		info->m_blockinfo.m_flags_memsize |= 1;
	info->m_blockinfo.m_table_info = ((unsigned int)info->m_blockinfo.m_table_info->m_blkarray[0].m_next >> 17) ?
																		 info->m_blockinfo.m_table_info->m_next :
																		 NULL;
	cCpuResumeIntr(state);
}

sysmem_internals_t *GetSysmemInternalData(void)
{
	return &g_sysmem_internals;
}

static void *allocSysMemory_internal(int flags, int size, const void *mem)
{
	unsigned int size_rounded;

	size_rounded = (unsigned int)(size + 255) >> 8;
	if ( !size_rounded )
		return NULL;
	switch ( flags )
	{
		case ALLOC_FIRST:
		{
			sysmem_alloc_block_t *i;
			sysmem_alloc_block_t *next;
			sysmem_alloc_block_info_t blkinfo;

			// Unofficial: properly check if m_alloclist is not NULL
			if ( !g_sysmem_internals.m_alloclist )
				return NULL;
			for ( i = g_sysmem_internals.m_alloclist->m_blkarray; i; i = i->m_next )
				if ( !i->m_info.m_allocated && i->m_info.m_size >= size_rounded )
					break;
			if ( !i )
				return NULL;
			if ( i->m_info.m_size == size_rounded )
			{
				i->m_info.m_allocated = 1;
				return (void *)(uiptr)(i->m_info.m_address << 8);
			}
			g_sysmem_internals.m_allocation_count += 1;
			blkinfo.m_address = i->m_info.m_address + size_rounded;
			blkinfo.m_size = i->m_info.m_size - size_rounded;
			blkinfo.m_allocated = i->m_info.m_allocated;
			blkinfo.m_pad = i->m_info.m_pad;
			i->m_info.m_allocated = 1;
			i->m_info.m_size = size_rounded;
			// Unofficial: check next if not NULL
			for ( next = i->m_next; next && blkinfo.m_size; next = next->m_next )
			{
				sysmem_alloc_block_info_t blkinfo_tmp;

				blkinfo_tmp = next->m_info;
				next->m_info = blkinfo;
				blkinfo = blkinfo_tmp;
			}
			return (void *)(uiptr)(i->m_info.m_address << 8);
		}
		case ALLOC_LAST:
		{
			sysmem_alloc_block_t *i;
			sysmem_alloc_block_t *list;
			sysmem_alloc_block_t *next;
			unsigned int retaddr;
			sysmem_alloc_block_info_t blkinfo;

			// Unofficial: properly check if m_alloclist is not NULL
			if ( !g_sysmem_internals.m_alloclist )
				return NULL;
			i = NULL;
			for ( list = g_sysmem_internals.m_alloclist->m_blkarray; list; list = list->m_next )
				if ( !list->m_info.m_allocated && list->m_info.m_size >= size_rounded )
					i = list;
			if ( !i )
				return NULL;
			if ( i->m_info.m_size == size_rounded )
			{
				i->m_info.m_allocated = 1;
				return (void *)(uiptr)(i->m_info.m_address << 8);
			}
			g_sysmem_internals.m_allocation_count += 1;
			i->m_info.m_size -= size_rounded;
			retaddr = i->m_info.m_address + i->m_info.m_size;
			blkinfo.m_address = retaddr;
			blkinfo.m_size = size_rounded;
			blkinfo.m_allocated = 1;
			blkinfo.m_pad = 0;
			// Unofficial: check next if not NULL
			for ( next = i->m_next; next && blkinfo.m_size; next = next->m_next )
			{
				sysmem_alloc_block_info_t blkinfo_tmp;

				blkinfo_tmp = next->m_info;
				next->m_info = blkinfo;
				blkinfo = blkinfo_tmp;
			}
			return (void *)(uiptr)(retaddr << 8);
		}
		case ALLOC_ADDRESS:
		{
			sysmem_alloc_block_t *i;
			sysmem_alloc_block_t *next;
			unsigned int mem_rounded;
			sysmem_alloc_block_info_t blkinfo;

			if ( (((uiptr)mem) & 0xFF) )
				return NULL;
			mem_rounded = (unsigned int)mem >> 8;
			// Unofficial: properly check if m_alloclist is not NULL
			if ( !g_sysmem_internals.m_alloclist )
				return NULL;
			for ( i = g_sysmem_internals.m_alloclist->m_blkarray; i; i = i->m_next )
			{
				if ( mem_rounded < i->m_info.m_address )
					return NULL;
				if ( !i->m_info.m_allocated && (u32)(i->m_info.m_address + i->m_info.m_size) >= mem_rounded + size_rounded )
					break;
			}
			if ( !i )
				return NULL;
			if ( i->m_info.m_address < mem_rounded )
			{
				unsigned int sz_tmp;
				sysmem_alloc_block_t *blk_tmp;
				sysmem_alloc_block_info_t blkinfo_nest;

				g_sysmem_internals.m_allocation_count += 1;
				sz_tmp = i->m_info.m_address + i->m_info.m_size - mem_rounded;
				i->m_info.m_size -= sz_tmp;
				blkinfo_nest.m_address = i->m_info.m_address + i->m_info.m_size;
				blkinfo_nest.m_size = sz_tmp;
				blkinfo_nest.m_allocated = 0;
				blkinfo_nest.m_pad = 0;
				blk_tmp = i->m_next;
				i = blk_tmp;
				while ( blkinfo_nest.m_size )
				{
					sysmem_alloc_block_info_t blkinfo_tmp;

					blkinfo_tmp = blk_tmp->m_info;
					blk_tmp->m_info = blkinfo_nest;
					blkinfo_nest = blkinfo_tmp;
					blk_tmp = blk_tmp->m_next;
				}
			}
			if ( i->m_info.m_size == size_rounded )
			{
				i->m_info.m_allocated = 1;
				return (void *)(uiptr)(i->m_info.m_address << 8);
			}
			g_sysmem_internals.m_allocation_count += 1;
			blkinfo.m_address = i->m_info.m_address + size_rounded;
			blkinfo.m_size = i->m_info.m_size - size_rounded;
			blkinfo.m_allocated = i->m_info.m_allocated;
			blkinfo.m_pad = i->m_info.m_pad;
			i->m_info.m_allocated = 1;
			i->m_info.m_size = size_rounded;
			// Unofficial: check next if not NULL
			for ( next = i->m_next; next && blkinfo.m_size; next = next->m_next )
			{
				sysmem_alloc_block_info_t blkinfo_tmp;

				blkinfo_tmp = next->m_info;
				next->m_info = blkinfo;
				blkinfo = blkinfo_tmp;
			}
			return (void *)(uiptr)(i->m_info.m_address << 8);
		}
		default:
			return NULL;
	}
}

static int freeSysMemory_internal(const void *ptr)
{
	unsigned int ptr_rounded;
	sysmem_alloc_block_t *blklist;
	sysmem_alloc_block_t *prev_blk;
	int blk_modcnt;
	sysmem_alloc_block_t *merge_blk;
	const sysmem_alloc_block_t *next;
	const sysmem_alloc_block_t *i;

	ptr_rounded = (unsigned int)ptr >> 8;
	if ( (((uiptr)ptr) & 0xFF) )
		return -1;
	prev_blk = NULL;
	// Unofficial: properly check if m_alloclist is not NULL
	if ( !g_sysmem_internals.m_alloclist )
		return -1;
	for ( blklist = &g_sysmem_internals.m_alloclist->m_blkarray[2]; blklist; blklist = blklist->m_next )
	{
		if ( blklist->m_info.m_size && blklist->m_info.m_address == ptr_rounded )
			break;
		prev_blk = blklist;
	}
	if ( !blklist || !blklist->m_info.m_allocated )
		return -1;
	blk_modcnt = 0;
	merge_blk = NULL;
	next = blklist->m_next;
	blklist->m_info.m_allocated = 0;
	if ( next && next->m_info.m_size && !next->m_info.m_allocated )
	{
		blk_modcnt += 1;
		g_sysmem_internals.m_allocation_count -= 1;
		merge_blk = blklist->m_next;
		blklist->m_info.m_size += merge_blk->m_info.m_size;
	}
	if ( prev_blk && !prev_blk->m_info.m_allocated )
	{
		merge_blk = blklist;
		blk_modcnt += 1;
		g_sysmem_internals.m_allocation_count -= 1;
		prev_blk->m_info.m_size += blklist->m_info.m_size;
	}
	if ( !blk_modcnt )
		return 0;
	blk_modcnt -= 1;
	for ( i = merge_blk; blk_modcnt != -1; blk_modcnt -= 1 )
		i = i->m_next;
	for ( ; i; i = i->m_next )
	{
		merge_blk->m_info = i->m_info;
		merge_blk = merge_blk->m_next;
	}
	return 0;
}

static void updateSmemCtlBlk(void)
{
	sysmem_alloc_table_t *alloclist;

	g_sysmem_internals.m_smemupdate_cur += 1;
	for ( alloclist = g_sysmem_internals.m_alloclist; alloclist->m_next; alloclist = alloclist->m_next )
		if ( alloclist->m_blkarray[(sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])) - 4].m_info.m_size )
		{
			alloclist->m_next =
				(sysmem_alloc_table_t *)allocSysMemory_internal(ALLOC_FIRST, sizeof(sysmem_alloc_table_t), NULL);
			if ( alloclist->m_next )
			{
				sysmem_alloc_table_t *next;
				unsigned int i;

				next = alloclist->m_next;
				alloclist->m_blkarray[(sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])) - 1].m_next =
					&(next->m_blkarray[0]);
				next->m_next = NULL;
				for ( i = 0; i < (sizeof(next->m_blkarray) / sizeof(next->m_blkarray[0])); i += 1 )
				{
					next->m_blkarray[i].m_next = &(next->m_blkarray[i + 1]);
					next->m_blkarray[i].m_info.m_allocated = 0;
					next->m_blkarray[i].m_info.m_address = 0;
					next->m_blkarray[i].m_info.m_pad = 0;
					next->m_blkarray[i].m_info.m_size = 0;
				}
				next->m_blkarray[(sizeof(next->m_blkarray) / sizeof(next->m_blkarray[0])) - 1].m_next = NULL;
			}
		}
	alloclist = g_sysmem_internals.m_alloclist;
	// Unofficial: properly check if m_alloclist is not NULL
	if ( alloclist && alloclist->m_next )
	{
		sysmem_alloc_table_t *next;

		for ( ; alloclist->m_next->m_next; alloclist = alloclist->m_next )
			;
		next = alloclist->m_next;
		if (
			next
			&& !alloclist->m_blkarray[(sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])) - 4].m_info.m_size )
		{
			alloclist->m_blkarray[(sizeof(alloclist->m_blkarray) / sizeof(alloclist->m_blkarray[0])) - 1].m_next = NULL;
			alloclist->m_next = NULL;
			freeSysMemory_internal(next);
		}
	}
}

static sysmem_alloc_block_t *search_block(const void *address)
{
	sysmem_alloc_block_t *blklist;

	// Unofficial: properly check if m_alloclist is not NULL
	if ( !g_sysmem_internals.m_alloclist )
		return NULL;
	for ( blklist = g_sysmem_internals.m_alloclist->m_blkarray; blklist; blklist = blklist->m_next )
		if (
			(uiptr)address >= (uiptr)(blklist->m_info.m_address << 8)
			&& (uiptr)address < (uiptr)((blklist->m_info.m_address + blklist->m_info.m_size) << 8) )
			break;
	return blklist;
}

static int cCpuSuspendIntr(int *state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(g_sysmem_internals.m_intr_suspend_tbl);
	return (intrman_callbacks && intrman_callbacks->cbCpuSuspendIntr) ? intrman_callbacks->cbCpuSuspendIntr(state) : 0;
}

static int cCpuResumeIntr(int state)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(g_sysmem_internals.m_intr_suspend_tbl);
	return (intrman_callbacks && intrman_callbacks->cbCpuResumeIntr) ? intrman_callbacks->cbCpuResumeIntr(state) : 0;
}

#if 0
static int cQueryIntrContext(void)
{
	intrman_callbacks_t *intrman_callbacks = (intrman_callbacks_t *)(g_sysmem_internals.m_intr_suspend_tbl);
	return ( intrman_callbacks && intrman_callbacks->cbQueryIntrContext ) ? intrman_callbacks->cbQueryIntrContext() : 0;
}
#endif

int Kprintf(const char *format, ...)
{
	int ret;
	va_list va;

	if ( !g_kprintf_cb )
		return 0;
	va_start(va, format);
	ret = g_kprintf_cb(g_kprintf_cb_userdata, format, va);
	va_end(va);
	return ret;
}

void KprintfSet(KprintfHandler_t *new_cb, void *context)
{
	if ( g_kprintf_cb && new_cb )
		new_cb(context, (const char *)(uiptr)Kprintf(NULL), NULL);
	g_kprintf_cb = new_cb;
	g_kprintf_cb_userdata = context;
}
