/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Standard C library heap allocation routines.
*/

/* This code is based on code contributed by Philip Joaqiun (jenova0). */

#include <tamtypes.h>
#include <kernel.h>
#include <malloc.h>
#include <string.h>
#ifdef DEBUG_ALLOC
#include <stdio.h>
#endif

/* Use this to set the default malloc() alignment. */
#define DEFAULT_ALIGNMENT	16

#ifndef ALIGN
#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

#ifdef DEBUG_ALLOC
#define ALLOC_MAGIC 0xa110ca73
#endif

void _ps2sdk_alloc_init();
void _ps2sdk_alloc_deinit();
void _ps2sdk_alloc_lock();
void _ps2sdk_alloc_unlock();

#ifdef F___alloc_internals
static vs32 alloc_sema = -1;
void _ps2sdk_alloc_init()
{
    ee_sema_t alloc_sema_struct;
    alloc_sema_struct.init_count = 1;
    alloc_sema_struct.max_count = 1;
    alloc_sema = CreateSema(&alloc_sema_struct);
}

void _ps2sdk_alloc_deinit()
{
    if (alloc_sema >= 0) {
	DeleteSema(alloc_sema);
    }
}

void _ps2sdk_alloc_lock()
{
    if (alloc_sema >= 0) {
	WaitSema(alloc_sema);
    }
}

void _ps2sdk_alloc_unlock()
{
    if (alloc_sema >= 0) {
	SignalSema(alloc_sema);
    }
}
#endif

/* _heap_mem_block_header structure. */
typedef struct _heap_mem_header {
#ifdef DEBUG_ALLOC
	u32     magic;
#endif
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
	
	_ps2sdk_alloc_lock();

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
#ifdef DEBUG_ALLOC
		__alloc_heap_head->magic = ALLOC_MAGIC;
#endif
		__alloc_heap_head->ptr  = ptr;
		__alloc_heap_head->size = mem_sz - sizeof(heap_mem_header_t);
		__alloc_heap_head->prev = NULL;
		__alloc_heap_head->next = NULL;

		__alloc_heap_tail = __alloc_heap_head;
		
		_ps2sdk_alloc_unlock();
		return ptr;
	}

	/* Check to see if there's free space at the bottom of the heap.  */
	if ((__alloc_heap_base + mem_sz) < (void *)__alloc_heap_head) {
		new_mem = (heap_mem_header_t *)__alloc_heap_base;
		ptr     = (void *)((u32)new_mem + sizeof(heap_mem_header_t));

#ifdef DEBUG_ALLOC
		new_mem->magic = ALLOC_MAGIC;
#endif
		new_mem->ptr  = ptr;
		new_mem->size = mem_sz - sizeof(heap_mem_header_t);
		new_mem->prev = NULL;
		new_mem->next = __alloc_heap_head;
		new_mem->next->prev = new_mem;
		__alloc_heap_head = new_mem;
		
		_ps2sdk_alloc_unlock();
		return ptr;
	}

	/* See if we can allocate the block without extending the heap. */
	prev_mem = _heap_mem_fit(__alloc_heap_head, mem_sz);
	if (prev_mem != NULL) {
		new_mem = (heap_mem_header_t *)((u32)prev_mem->ptr + prev_mem->size);
		ptr     = (void *)((u32)new_mem + sizeof(heap_mem_header_t));

#ifdef DEBUG_ALLOC
		new_mem->magic = ALLOC_MAGIC;
#endif
		new_mem->ptr  = ptr;
		new_mem->size = mem_sz - sizeof(heap_mem_header_t);
		new_mem->prev = prev_mem;
		new_mem->next = prev_mem->next;
		new_mem->next->prev = new_mem;
		prev_mem->next = new_mem;

		_ps2sdk_alloc_unlock();
		return ptr;
	}

	/* Extend the heap, but make certain the block is inserted in
	   order. */
	if ((mem_ptr = ps2_sbrk(mem_sz)) == (void *)-1) {
		_ps2sdk_alloc_unlock();
		return ptr;	/* NULL */
	}

	ptr = (void *)((u32)mem_ptr + sizeof(heap_mem_header_t));

	new_mem       = (heap_mem_header_t *)mem_ptr;
#ifdef DEBUG_ALLOC
	new_mem->magic = ALLOC_MAGIC;
#endif
	new_mem->ptr  = ptr;
	new_mem->size = mem_sz - sizeof(heap_mem_header_t);
	new_mem->prev = __alloc_heap_tail;
	new_mem->next = NULL;

	__alloc_heap_tail->next = new_mem;
	__alloc_heap_tail       = new_mem;
	
	_ps2sdk_alloc_unlock();
	return ptr;
}
#endif

#ifdef F_realloc
__attribute__((weak))
void * realloc(void *ptr, size_t size)
{
	heap_mem_header_t *prev_mem;
	void *new_ptr = NULL;

	if (!size && ptr != NULL) {
		free(ptr);
		return new_ptr;
	}

	if (ptr == NULL)
		return malloc(size);

	if ((size & (DEFAULT_ALIGNMENT - 1)) != 0)
		size = ALIGN(size, DEFAULT_ALIGNMENT);

	_ps2sdk_alloc_lock();
	prev_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));

#ifdef DEBUG_ALLOC
	if (prev_mem->magic != ALLOC_MAGIC) {
		fprintf(stderr, "realloc: Pointer at %p was not malloc()ed before, or got overwritten.\n", ptr);

		_ps2sdk_alloc_unlock();
		return NULL;
	}
#endif

	/* Don't do anything if asked for same sized block. */
	/* If the new size is shorter, let's just shorten the block. */
	if (prev_mem->size >= size) {
		/* However, if this is the last block, we have to shrink the heap. */
		if (!prev_mem->next)
			ps2_sbrk(ptr + size - ps2_sbrk(0));
		prev_mem->size = size;
		
		_ps2sdk_alloc_unlock();
		return ptr;
	}
	
	
	/* We are asked for a larger block of memory. */
	
	/* Are we the last memory block ? */
	if (!prev_mem->next) {
		/* Yes, let's just extend the heap then. */
		if (ps2_sbrk(size - prev_mem->size) == (void*) -1)
			return NULL;
		prev_mem->size = size;
		
		_ps2sdk_alloc_unlock();
		return ptr;
	}
	
	/* Is the next block far enough so we can extend the current block ? */
	if ((prev_mem->next->ptr - ptr) > size) {
		prev_mem->size = size;
		
		_ps2sdk_alloc_unlock();
		return ptr;
	}
	
	_ps2sdk_alloc_unlock();
	
	/* We got out of luck, let's allocate a new block of memory. */
	if ((new_ptr = malloc(size)) == NULL)
		return new_ptr;

        /* New block is larger, we only copy the old data. */
	memcpy(new_ptr, ptr, prev_mem->size);

	free(ptr);
	return new_ptr;
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
	heap_mem_header_t *old_mem;
	void *ptr = NULL;

	if (align <= DEFAULT_ALIGNMENT)
		return malloc(size);

	/* Allocate with extra alignment bytes just in case it isn't aligned
	   properly by malloc.  */
	if ((ptr = malloc(size + align)) == NULL)
		return ptr;	/* NULL */

	/* If malloc returned it aligned for us we're fine.  */
	if (((u32)ptr & (align - 1)) == 0)
		return ptr;

	_ps2sdk_alloc_lock();
	cur_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));
	cur_mem->size -= align;

	/* Otherwise, align the pointer and fixup our hearder accordingly.  */
	ptr = (void *)ALIGN((u32)ptr, align);
	
	old_mem = cur_mem;

	/* Copy the heap_mem_header_t locally, before repositioning (to make
	   sure we don't overwrite ourselves.  */
	memcpy(&new_mem, cur_mem, sizeof(heap_mem_header_t));
	cur_mem = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));
	memcpy(cur_mem, &new_mem, sizeof(heap_mem_header_t));

	if (cur_mem->prev)
		cur_mem->prev->next = cur_mem;
	if (cur_mem->next)
		cur_mem->next->prev = cur_mem;
	
	if (__alloc_heap_head == old_mem)
		__alloc_heap_head = cur_mem;
	
	if (__alloc_heap_tail == old_mem)
		__alloc_heap_tail = cur_mem;

	cur_mem->ptr = ptr;
	
	_ps2sdk_alloc_unlock();
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

	_ps2sdk_alloc_lock();
	
	if (!__alloc_heap_head) {
		_ps2sdk_alloc_unlock();
		return;
	}

#ifdef DEBUG_ALLOC
	cur = (heap_mem_header_t *)((u32)ptr - sizeof(heap_mem_header_t));
	if (cur->magic != ALLOC_MAGIC) {
		fprintf(stderr, "free: Pointer at %p was not malloc()ed before, or got overwritten.\n", ptr);
		
		_ps2sdk_alloc_unlock();
		return;
	}
#endif

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
		
		_ps2sdk_alloc_unlock();
		return;
	}

	cur = __alloc_heap_head;
	while (ptr != cur->ptr)  {
		/* ptr isn't in our list */
		if (cur->next == NULL) {
			_ps2sdk_alloc_unlock();
			return;
		}

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
	
	_ps2sdk_alloc_unlock();
}
#endif

/* These are here in case C++ needs them.  */
#ifdef F___builtin_alloc
__attribute__((weak))
void * __builtin_new(size_t size) { return malloc(size); }

__attribute__((weak))
void __builtin_delete(void *ptr) { free(ptr); }
#endif

#ifdef F___mem_walk
void * __mem_walk_begin() {
	return __alloc_heap_head;
}

void __mem_walk_read(void * token, u32 * size, void ** ptr, int * valid) {
        heap_mem_header_t * cur = (heap_mem_header_t *) token;
    
#ifdef DEBUG_ALLOC
	if (cur->magic != ALLOC_MAGIC) {
    		*valid = 0;
		return;
	}
#endif
	*valid = 1;
    
	*size = cur->size;
	*ptr = cur->ptr;
}

void * __mem_walk_inc(void * token) {
	heap_mem_header_t * cur = (heap_mem_header_t *) token;
    
	return cur->next;
}

int __mem_walk_end(void * token) {
	return token == NULL;
}
#endif
