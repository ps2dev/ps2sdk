/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  alloc.c                  Standard C library heap allocation routines.
*/

/* This code is based on code contributed by Philip Joaqiun (jenova0). */

#include <tamtypes.h>
#include <kernel.h>
#include <malloc.h>
#include <string.h>

/* Use this to set the default malloc() alignment. */
#define DEFAULT_ALIGNMENT	16

#ifndef ALIGN
#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

/* _heap_mem_block_header structure. */
typedef struct _heap_mem_header {
	void *	ptr;
	size_t	size;
	struct _heap_mem_header * prev;
	struct _heap_mem_header * next;
} heap_mem_header_t;

extern void * __alloc_heap_base;
extern heap_mem_header_t *__alloc_heap_head;
extern heap_mem_header_t *__alloc_heap_tail;

heap_mem_header_t * _heap_mem_fit(heap_mem_header_t *head, size_t size);

#ifdef F_malloc

void * __alloc_heap_base = NULL;
heap_mem_header_t *__alloc_heap_head = NULL;
heap_mem_header_t *__alloc_heap_tail = NULL;

/* Find a the lowest block that we can allocate AFTER, returning NULL if there
   are none.  */
heap_mem_header_t * _heap_mem_fit(heap_mem_header_t *head, size_t size)
{
	heap_mem_header_t *prev_mem = head;
	u32 prev_top, next_bot;

	while (prev_mem != NULL) {
		if (prev_mem->next != NULL) {
			prev_top = (u32)prev_mem->ptr + prev_mem->size;
			next_bot = (u32)prev_mem->next - prev_top;
			if (next_bot >= size)
				return prev_mem;
		}

		prev_mem = prev_mem->next;
	}

	return prev_mem;
}

__attribute__((weak))
void * malloc(size_t size)
{
	void *ptr = NULL, *mem_ptr;
	heap_mem_header_t *new_mem, *prev_mem;
	size_t mem_sz, heap_align_bytes;

	mem_sz = size + sizeof(heap_mem_header_t);

	if ((mem_sz & (DEFAULT_ALIGNMENT - 1)) != 0)
		mem_sz = ALIGN(mem_sz, DEFAULT_ALIGNMENT);

	/* If we don't have any allocated blocks, reserve the first block from
	   the OS and initialize __alloc_heap_tail.  */
	if (__alloc_heap_head == NULL) {
		/* Align the bottom of the heap to our default alignment.  */
		if (__alloc_heap_base == NULL) {
			heap_align_bytes = (u32)ps2_sbrk(0) & (DEFAULT_ALIGNMENT - 1);
			ps2_sbrk(heap_align_bytes);
			__alloc_heap_base = ps2_sbrk(0);
		}

		/* Allocate the physical heap and setup the head block.  */
		if ((mem_ptr = ps2_sbrk(mem_sz)) == (void *)-1)
			return ptr;	/* NULL */
		
		ptr = (void *)((u32)mem_ptr + sizeof(heap_mem_header_t));

		__alloc_heap_head       = (heap_mem_header_t *)mem_ptr;
		__alloc_heap_head->ptr  = ptr;
		__alloc_heap_head->size = mem_sz - sizeof(heap_mem_header_t);
		__alloc_heap_head->prev = NULL;
		__alloc_heap_head->next = NULL;

		__alloc_heap_tail = __alloc_heap_head;
		return ptr;
	}

	/* Check to see if there's free space at the bottom of the heap.  */
	if ((__alloc_heap_base + mem_sz) < (void *)__alloc_heap_head) {
		new_mem = (heap_mem_header_t *)__alloc_heap_base;
		ptr     = (void *)((u32)new_mem + sizeof(heap_mem_header_t));

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz - sizeof(heap_mem_header_t);
		new_mem->prev = NULL;
		new_mem->next = __alloc_heap_head;
		new_mem->next->prev = new_mem;
		__alloc_heap_head = new_mem;

		return ptr;
	}

	/* See if we can allocate the block without extending the heap. */
	prev_mem = _heap_mem_fit(__alloc_heap_head, mem_sz);
	if (prev_mem != NULL) {
		new_mem = (heap_mem_header_t *)((u32)prev_mem->ptr + prev_mem->size);
		ptr     = (void *)((u32)new_mem + sizeof(heap_mem_header_t));

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz - sizeof(heap_mem_header_t);
		new_mem->prev = prev_mem;
		new_mem->next = prev_mem->next;
		new_mem->next->prev = new_mem;
		prev_mem->next = new_mem;

		return ptr;
	}

	/* Extend the heap, but make certain the block is inserted in
	   order. */
	if ((mem_ptr = ps2_sbrk(mem_sz)) == (void *)-1)
		return ptr;	/* NULL */

	ptr = (void *)((u32)mem_ptr + sizeof(heap_mem_header_t));

	new_mem       = (heap_mem_header_t *)mem_ptr;
	new_mem->ptr  = ptr;
	new_mem->size = mem_sz - sizeof(heap_mem_header_t);
	new_mem->prev = __alloc_heap_tail;
	new_mem->next = NULL;

	__alloc_heap_tail->next = new_mem;
	__alloc_heap_tail       = new_mem;
	return ptr;
}
#endif

#ifdef F_realloc
__attribute__((weak))
void * realloc(void *ptr, size_t size)
{
	heap_mem_header_t *prev_mem;
	void *new = NULL;

	if (!size && ptr != NULL) {
		free(ptr);
		return new;
	}

	if (ptr == NULL)
		return malloc(size);

	prev_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));

	/* Don't do anything if asked for same sized block. */
	if (prev_mem->size == size)
		return ptr;

	if ((new = malloc(size)) == NULL)
		return new;

	memcpy(new, ptr, prev_mem->size);

	free(ptr);
	return new;
}
#endif

#ifdef F_calloc
__attribute__((weak))
void * calloc(size_t n, size_t size)
{
	void *ptr = NULL;
	size_t sz = n * size;

	if ((ptr = malloc(sz)) == NULL)
		return ptr;

	memset(ptr, 0, sz);
	return ptr;
}
#endif

#ifdef F_memalign
__attribute__((weak))
void * memalign(size_t align, size_t size)
{
	heap_mem_header_t new_mem;
	heap_mem_header_t *cur_mem;
	void *ptr = NULL;

	if (align <= DEFAULT_ALIGNMENT)
		return malloc(size);

	/* Allocate with extra alignment bytes just in case it isn't aligned
	   properly by malloc.  */
	if ((ptr = malloc(size + align)) == NULL)
		return ptr;	/* NULL */

	cur_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));
	cur_mem->size -= align;

	/* If malloc returned it aligned for us we're fine.  */
	if (((u32)ptr & (align - 1)) == 0)
		return ptr;

	/* Otherwise, align the pointer and fixup our hearder accordingly.  */
	ptr = (void *)ALIGN((u32)ptr, align);

	/* Copy the heap_mem_header_t locally, before repositioning (to make
	   sure we don't overwrite ourselves.  */
	memcpy(&new_mem, cur_mem, sizeof(heap_mem_header_t));
	cur_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));
	memcpy(cur_mem, &new_mem, sizeof(heap_mem_header_t));

	if (cur_mem->prev)
		cur_mem->prev->next = cur_mem;
	if (cur_mem->next)
		cur_mem->next->prev = cur_mem;

	cur_mem->ptr = ptr;
	return ptr;
}
#endif

#ifdef F_free
__attribute__((weak))
void free(void *ptr)
{
	heap_mem_header_t *cur;
	void *heap_top;
	size_t size;

	if (!ptr)
		return;

	if (!__alloc_heap_head)
		return;

	/* Freeing the head pointer is a special case.  */
	if (ptr == __alloc_heap_head->ptr) {
		size = __alloc_heap_head->size +
			(size_t)(__alloc_heap_head->ptr - (void *)__alloc_heap_head);

		__alloc_heap_head = __alloc_heap_head->next;

		if (__alloc_heap_head != NULL) {
			__alloc_heap_head->prev = NULL;
		} else {
			__alloc_heap_tail = NULL;

			ps2_sbrk(-size);
		}

		return;
	}

	cur = __alloc_heap_head;
	while (ptr != cur->ptr)  {
		/* ptr isn't in our list */
		if (cur->next == NULL)
			return;

		cur = cur->next;
	}

	/* Deallocate the block.  */
	if (cur->next != NULL) {
		cur->next->prev = cur->prev;
	} else {
		/* If this block was the last one in the list, shrink the heap.  */
		__alloc_heap_tail = cur->prev;

		/* We need to free (heap top) - (prev->ptr + prev->size), or else
		   we'll end up with an unallocatable block of heap.  */
		heap_top = ps2_sbrk(0);
		size = (u32)heap_top - (u32)(cur->prev->ptr + cur->prev->size);
		ps2_sbrk(-size);
	}

	cur->prev->next = cur->next;
}
#endif

/* These are here in case C++ needs them.  */
#ifdef F___builtin_alloc
__attribute__((weak))
void * __builtin_new(size_t size) { return malloc(size); }

__attribute__((weak))
void __builtin_delete(void *ptr) { free(ptr); }
#endif
