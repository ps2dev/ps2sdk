/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "heaplib.h"
#include "irx_imports.h"

extern struct irx_export_table _exp_heaplib;

#ifdef _IOP
IRX_ID("Heap_lib", 1, 1);
#endif
// Based on the module from SCE SDK 1.3.4.

typedef struct heaplib_ll_
{
	struct heaplib_ll_ *next;
	struct heaplib_ll_ *prev;
	char payload[];
} heaplib_ll_t;

typedef struct heaplib_chunk_fragment_
{
	struct heaplib_chunk_fragment_ *next_fragment;
	u32 chunk_size_blks;
	char payload[];
} heaplib_chunk_fragment_t;

typedef struct heaplib_chunk_
{
	void *chunk_validation_key;
	u32 free_size;
	u32 used_size;
	heaplib_chunk_fragment_t *chunk_fragment_prev;
	heaplib_chunk_fragment_t fragment;
} heaplib_chunk_t;

typedef struct heaplib_heap_
{
	void *heap_validation_key;
	u32 size2free_flags;
	heaplib_ll_t l;
	heaplib_chunk_t mem_chunk;
} heaplib_heap_t;

int _start(int ac, char **av)
{
	(void)ac;
	(void)av;

	return RegisterLibraryEntries(&_exp_heaplib);
}

static void linked_list_set_self(heaplib_ll_t *ll)
{
	ll->next = ll;
	ll->prev = ll;
}

#if 0
static int linked_list_next_is_self(heaplib_ll_t *ll)
{
	return ll->next == ll;
}
#endif

static void linked_list_remove(heaplib_ll_t *ll)
{
	ll->next->prev = ll->prev;
	ll->prev->next = ll->next;
}

#if 0
static int linked_list_is_circular(heaplib_ll_t *ll)
{
	return ll->prev == ll->next;
}
#endif

static void linked_list_add_after(heaplib_ll_t *ll1, heaplib_ll_t *ll2)
{
	ll2->next = ll1;
	ll2->prev = ll1->prev;
	ll1->prev = ll2;
	ll2->prev->next = ll2;
}

void HeapPrepare(void *mem, int size)
{
	u32 calc_size_blks;
	heaplib_chunk_fragment_t *item;
	heaplib_chunk_t *chunk;

	chunk = (heaplib_chunk_t *)mem;
	if ( chunk )
	{
		if ( (unsigned int)size >= 0x29 )
		{
			unsigned int calc_size;

			chunk->chunk_validation_key = (((u8 *)chunk) - 1);
			calc_size = (unsigned int)(size - 16) >> 3;
			calc_size_blks = calc_size - 1;
			item = (heaplib_chunk_fragment_t *)(&chunk->used_size + 2 * calc_size);
			chunk->free_size = size;
			chunk->chunk_fragment_prev = &chunk->fragment;
			chunk->used_size = 0;
			chunk->fragment.chunk_size_blks = calc_size_blks;
			chunk->fragment.next_fragment = item;
			item->chunk_size_blks = 0;
			chunk->fragment.next_fragment->next_fragment = &chunk->fragment;
		}
	}
}

void *heaplib_13_chunk_do_allocate(heaplib_chunk_t *chunk, unsigned int nbytes)
{
	void *result;
	u32 size_calc1;
	const heaplib_chunk_fragment_t *chunkfrag_prev_1;
	u32 size_calc2;
	heaplib_chunk_fragment_t *chunkfrag_prev_2;
	heaplib_chunk_fragment_t *fragment1;
	u32 chunk_size_blks;
	u32 size_subtract;

	if ( !chunk || chunk->chunk_validation_key != (((u8 *)chunk) - 1) )
		return 0;
	size_calc1 = nbytes + 7;
	if ( nbytes < 8 )
		size_calc1 = 15;
	chunkfrag_prev_1 = chunk->chunk_fragment_prev;
	size_calc2 = (size_calc1 >> 3) + 1;
	chunkfrag_prev_2 = chunk->chunk_fragment_prev;
	for ( fragment1 = chunkfrag_prev_2->next_fragment;; fragment1 = fragment1->next_fragment )
	{
		chunk_size_blks = fragment1->chunk_size_blks;
		if ( (int)chunk_size_blks >= (int)size_calc2 )
			break;
		chunkfrag_prev_2 = fragment1;
		if ( fragment1 == chunkfrag_prev_1 )
			return 0;
	}
	size_subtract = chunk_size_blks - size_calc2;
	if ( chunk_size_blks == size_calc2 )
	{
		chunkfrag_prev_2->next_fragment = fragment1->next_fragment;
	}
	else
	{
		fragment1->chunk_size_blks = size_subtract;
		fragment1 += size_subtract;
		fragment1->chunk_size_blks = size_calc2;
	}
	chunk->chunk_fragment_prev = chunkfrag_prev_2;
	fragment1->next_fragment = (heaplib_chunk_fragment_t *)chunk;
	result = fragment1->payload;
	chunk->used_size += fragment1->chunk_size_blks;
	return result;
}

int heaplib_14_chunk_do_iterate(heaplib_chunk_t *chunk, void *ptr)
{
	heaplib_chunk_fragment_t *chunk_header;
	heaplib_chunk_fragment_t *fragment1;
	const heaplib_chunk_fragment_t *fragment2;
	const heaplib_chunk_fragment_t *next_chunk;
	u32 chunk_size_blks2;

	if ( !chunk || chunk->chunk_validation_key != (((u8 *)chunk) - 1) )
		return -4;
	if ( !ptr )
		return -1;
	chunk_header = (heaplib_chunk_fragment_t *)((char *)ptr - 8);
	if ( *((heaplib_chunk_t **)ptr - 2) != chunk || (int)chunk_header->chunk_size_blks <= 0 )
		return -1;
	if (
		chunk_header < &chunk->fragment || chunk_header >= (heaplib_chunk_fragment_t *)((char *)chunk + chunk->free_size) )
		return -2;
	fragment1 = chunk->chunk_fragment_prev;
	if ( fragment1 >= chunk_header || chunk_header >= fragment1->next_fragment )
	{
		while ( chunk_header != fragment1 )
		{
			fragment2 = fragment1->next_fragment;
			if ( fragment1 < fragment1->next_fragment )
				goto lbl18;
			if ( chunk_header < fragment1->next_fragment )
				goto lbl20;
			if ( fragment1 >= chunk_header )
			{
			lbl18:
				fragment1 = fragment1->next_fragment;
				if ( fragment2 >= chunk_header || chunk_header >= fragment2->next_fragment )
					continue;
			}
			if ( chunk_header >= fragment1->next_fragment )
				goto lbl21;
			goto lbl20;
		}
		return -3;
	}
lbl20:
	if ( fragment1->next_fragment < &chunk_header[chunk_header->chunk_size_blks] )
		return -3;
lbl21:
	if ( fragment1 < chunk_header && chunk_header < &fragment1[fragment1->chunk_size_blks] )
		return -3;
	chunk->used_size -= chunk_header->chunk_size_blks;
	next_chunk = &chunk_header[chunk_header->chunk_size_blks];
	if (
		next_chunk != fragment1->next_fragment
		|| (chunk_size_blks2 = next_chunk->chunk_size_blks, (int)chunk_size_blks2 <= 0) )
	{
		chunk_header->next_fragment = fragment1->next_fragment;
	}
	else
	{
		chunk_header->chunk_size_blks += chunk_size_blks2;
		chunk_header->next_fragment = fragment1->next_fragment->next_fragment;
	}
	if ( chunk_header == &fragment1[fragment1->chunk_size_blks] )
	{
		fragment1->chunk_size_blks += chunk_header->chunk_size_blks;
		fragment1->next_fragment = chunk_header->next_fragment;
	}
	else
	{
		fragment1->next_fragment = chunk_header;
	}
	chunk->chunk_fragment_prev = fragment1;
	return 0;
}

int heaplib_12_chunk_is_valid(heaplib_chunk_t *chunk)
{
	return chunk && chunk->chunk_validation_key == (((u8 *)chunk) - 1) && chunk->used_size == 0;
}

int HeapChunkSize(void *chunk_)
{
	const heaplib_chunk_t *chunk;

	chunk = (heaplib_chunk_t *)chunk_;
	return 8 * (((chunk->free_size - 16) >> 3) - chunk->used_size - 1);
}

void *CreateHeap(int heapblocksize, int flag)
{
	int calc_size;
	heaplib_heap_t *heap_new;

	calc_size = 4 * ((heapblocksize + 3) >> 2);
	heap_new = (heaplib_heap_t *)AllocSysMemory((flag & 2) != 0, calc_size, 0);
	if ( heap_new )
	{
		heap_new->heap_validation_key = (((u8 *)heap_new) + 1);
		if ( (flag & 1) != 0 )
			heap_new->size2free_flags = calc_size;
		else
			heap_new->size2free_flags = 0;
		heap_new->size2free_flags |= ((unsigned int)flag >> 1) & 1;
		linked_list_set_self(&heap_new->l);
		HeapPrepare(&heap_new->mem_chunk, calc_size - 16);
		return heap_new;
	}
	return 0;
}

void DeleteHeap(void *heap_)
{
	heaplib_ll_t *item;
	heaplib_ll_t *next;
	heaplib_heap_t *heap;

	heap = (heaplib_heap_t *)heap_;
	if ( heap->heap_validation_key == (char *)&heap->heap_validation_key + 1 )
	{
		item = heap->l.next;
		heap->heap_validation_key = 0;
		if ( item != &heap->l )
		{
			do
			{
				next = item->next;
				*(u32 *)item->payload = 0;
				FreeSysMemory(item);
				item = next;
			} while ( next != &heap->l );
		}
		heap->mem_chunk.chunk_validation_key = 0;
		FreeSysMemory(heap);
	}
}

void *AllocHeapMemory(void *heap_, size_t nbytes)
{
	heaplib_ll_t *item;
	const heaplib_ll_t *list_head;
	heaplib_chunk_t *heap_item;
	int size2free_flags;
	u32 size_rounded;
	heaplib_ll_t *heap_new;
	int BlockSize;
	heaplib_chunk_t *payload;
	heaplib_heap_t *heap;

	heap = (heaplib_heap_t *)heap_;
	if ( heap->heap_validation_key != (((u8 *)heap) + 1) )
		return 0;
	item = heap->l.next;
	list_head = &heap->l;
	for ( heap_item = (heaplib_chunk_t *)item->payload;; heap_item = (heaplib_chunk_t *)item->payload )
	{
		void *result;

		result = heaplib_13_chunk_do_allocate(heap_item, nbytes);
		if ( result )
			return result;
		if ( item == list_head )
		{
			size2free_flags = heap->size2free_flags;
			if ( size2free_flags >= 4 )
			{
				size_rounded = 2 * (size2free_flags >> 1);
				if ( (int)(size_rounded - 40) < (int)(nbytes) )
					size_rounded = nbytes + 40;
				heap_new = (heaplib_ll_t *)AllocSysMemory(heap->size2free_flags & 1, size_rounded, 0);
				if ( heap_new )
				{
					BlockSize = QueryBlockSize(heap_new);
					linked_list_add_after(heap->l.next, heap_new);
					payload = (heaplib_chunk_t *)heap_new->payload;
					HeapPrepare(payload, BlockSize - 8);
					return heaplib_13_chunk_do_allocate(payload, nbytes);
				}
			}
			return 0;
		}
		item = item->next;
	}
	return 0;
}

int FreeHeapMemory(void *heap_, void *ptr)
{
	const heaplib_ll_t *list_head;
	heaplib_ll_t *item;
	heaplib_chunk_t *chunk_item;
	heaplib_heap_t *heap;

	heap = (heaplib_heap_t *)heap_;
	if ( !heap )
		return -4;
	list_head = &heap->l;
	if ( heap->heap_validation_key != (((u8 *)heap) + 1) )
		return -4;
	item = heap->l.next;
	for ( chunk_item = (heaplib_chunk_t *)item->payload; heaplib_14_chunk_do_iterate(chunk_item, ptr);
				chunk_item = (heaplib_chunk_t *)item->payload )
	{
		if ( item == list_head )
			return -1;
		item = item->next;
	}
	if ( item != list_head && heaplib_12_chunk_is_valid(chunk_item) != 0 )
	{
		linked_list_remove(item);
		*(u32 *)item->payload = 0;
		FreeSysMemory(item);
	}
	return 0;
}

int HeapTotalFreeSize(void *heap_)
{
	int calc_size;
	heaplib_ll_t *list_head;
	heaplib_ll_t *chunk_item;
	heaplib_heap_t *heap;

	heap = (heaplib_heap_t *)heap_;
	calc_size = 0;
	if ( !heap )
		return -4;
	list_head = &heap->l;
	if ( heap->heap_validation_key != (((u8 *)heap) + 1) )
		return -4;
	for ( chunk_item = heap->l.next;; chunk_item = chunk_item->next )
	{
		calc_size += HeapChunkSize((heaplib_chunk_t *)chunk_item->payload);
		if ( chunk_item == list_head )
			break;
	}
	return calc_size;
}
