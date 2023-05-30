#ifndef THCOMMON_H_
#define THCOMMON_H_

#include "thbase.h"

#include "list.h"
#include "sysmem.h"
#include "thmsgbx.h"

#include <tamtypes.h>

#define DEBUG_FLAGS (8)

#define TAG_THREAD 0x7f01
#define TAG_SEMA   0x7f02
#define TAG_EVF    0x7f03
#define TAG_MBX    0x7f04
#define TAG_VPL    0x7f05
#define TAG_FPL    0x7f06

#define MAKE_HANDLE(ptr)         (((u32)(ptr) << 5) | ((((struct heaptag *)(ptr))->id & 0x3f) << 1) | 1)
#define HANDLE_PTR(handle)       ((void *)((((u32)handle) >> 7) << 2))
#define HANDLE_ID(handle)        (((handle) >> 1) & 0x3f)
#define HANDLE_VERIFY(handle, t) (((struct heaptag *)(HANDLE_PTR(handle)))->tag == (t) && \
                                  HANDLE_ID(handle) == ((struct heaptag *)(HANDLE_PTR(handle)))->id)

#define ALIGN(i)     (((i) + 3) & (~3))
#define ALIGN_256(i) (((i) + 0xff) & (~0xff))

#define UNIMPLEMENTED()                                              \
    do {                                                             \
        Kprintf("UNIMPLEMENTED FUNCTION %s\n", __PRETTY_FUNCTION__); \
        while (1)                                                    \
            ;                                                        \
    } while (0)

#define TRACE()                                     \
    do {                                            \
        Kprintf("TRACE %s\n", __PRETTY_FUNCTION__); \
    } while (0)

#define TRACE_ERROR()                                                         \
    do {                                                                      \
        Kprintf("TRACE %s %s:%d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
    } while (0)

#define RESERVED_REGCTX_SIZE 0xb8

// context state, saved and restored by intrman
struct regctx
{
    u32 unk, at, v0, v1, a0,
        a1, a2, a3, t0, t1,
        t2, t3, t4, t5, t6,
        t7, s0, s1, s2, s3,
        s4, s5, s6, s7, t8,
        t9, unk68, unk6c, gp,
        sp, fp, ra, hi, lo, sr,
        pc, I_CTRL, unk2;
};

struct heaptag
{
    u16 tag;
    u16 id;
};

struct event
{
    u32 attr;
    u32 waiter_count;
    struct list_head waiters;
    u32 option;
};

struct event_flag
{
    struct heaptag tag;
    struct list_head evf_list;
    struct event event;
    u32 bits;
    u32 init_bits;
};

struct semaphore
{
    struct heaptag tag;
    struct list_head sema_list;
    struct event event;
    u32 count;
    u32 max_count;
    u32 initial_count;
};

struct mbox
{
    struct heaptag tag;
    struct list_head mbox_list;
    struct event event;
    u32 msg_count;
    iop_message_t *newest_msg;
};

struct fpl_block
{
    struct fpl_block *next;
};

struct fpool
{
    struct heaptag tag;
    struct list_head fpl_list;
    struct event event;
    void *memory;
    struct fpl_block *free;
    u32 free_blocks;
    u32 block_size;
    u32 blocks;
    u32 mem_size;
};

struct vpool
{
    struct heaptag tag;
    struct list_head vpl_list;
    struct event event;
    void *heap;
    u32 free_size;
};

struct alarm
{
    struct heaptag tag;
    struct list_head alarm_list;
    u64 target;
    unsigned int (*cb)(void *);
    void *userptr;
};

struct thread
{
    struct heaptag tag;
    struct list_head queue;
    u8 status;
    u16 priority;
    struct regctx *saved_regs;
    u32 unk14;
    u32 unk18;
    u16 wait_type;
    u16 wakeup_count;
    union
    {
        struct event *wait_event;
        u32 wait_usecs;
    };
    // originally singly linked list of all threads
    // struct thread *thread_list;
    struct list_head thread_list;
    u32 event_bits;
    u16 event_mode;
    u16 init_priority;
    u32 run_clocks_hi;
    u32 run_clocks_lo;
    void *entry;
    void *stack_top;
    u32 stack_size;
    u32 gp;
    u32 attr;
    u32 option;
    // nothing seems to use wait_return, would be $ra
    // at the point of calling the wait function
    // leftover debugging feature?
    // u32 wait_return;
    u32 *reason_counter;
    u32 irq_preemption_count;
    u32 thread_preemption_count;
    u32 release_count;
};

struct thread_context
{
    struct thread *current_thread;
    struct thread *run_next;
    u32 queue_map[4]; // bitset of active priorities
    u32 debug_flags;
    struct list_head ready_queue[128];
    s32 timer_id;
    u32 (*timer_func)();
    u32 time_hi;
    u32 time_lo;
    u32 last_timer;
    // struct thread *thread_list;
    // switched to a doubly linked thread list
    struct list_head thread_list;
    struct thread *idle_thread;
    struct list_head semaphore;
    struct list_head event_flag;
    struct list_head mbox;
    struct list_head vpool;
    struct list_head fpool;
    struct list_head alarm;
    struct list_head alarm_pool;
    u16 thread_id;
    u16 sema_id;
    u16 evflag_id;
    u16 mbox_id;
    u32 unused_or_padding;
    u32 alarm_id;
    u32 alarm_count;
    struct list_head sleep_queue;
    struct list_head delay_queue;
    struct list_head dormant_queue;
    // struct list_head unused_list1;
    // struct list_head unused_list2;
    struct list_head delete_queue;
    void *heap;
    s32 sytem_status_flag;
    u32 thread_switch_count;
    u32 thread_resume_count;
    u32 min_wait;
    u32 unk4c8;
    u32 unk_clock_mult;
    u32 unk_clock_div;
};

extern struct thread_context thctx;

static inline u64 add64(u32 hi1, u32 lo1, u32 hi2, u32 lo2)
{

    u32 lo = lo1 + lo2;
    u32 hi = hi1 + hi2 + (lo1 > (lo1 + lo2));
    return lo | ((u64)hi << 32);
}

static inline u64 as_u64(u32 a1, u32 a2)
{
    return (u64)a1 << 32 | a2;
}

// Maybe uninline these to reduce code size if needed

/*
** Schedule at at the end of queeu
*/
static inline void readyq_insert_back(struct thread *thread)
{
    u32 prio = thread->priority;
    thctx.queue_map[prio >> 5] |= 1 << (prio & 0x1f);
    list_insert(&thctx.ready_queue[prio], &thread->queue);
}

/*
** Schedule at at the front of queue
*/
static inline void readyq_insert_front(struct thread *thread)
{
    u32 prio = thread->priority;
    thctx.queue_map[prio >> 5] |= 1 << (prio & 0x1f);
    list_insert(thctx.ready_queue[prio].next, &thread->queue);
}

/*
** Remove from the queue
*/
static inline void readyq_remove(struct thread *thread, u32 prio)
{
    if (list_node_is_last(&thread->queue)) {
        thctx.queue_map[prio >> 5] &= ~(1 << (prio & 0x1f));
    }

    list_remove(&thread->queue);
}

u32 readyq_highest();

struct alarm *alarm_alloc();
void alarm_free(struct alarm *alarm);
void alarm_insert(struct list_head *list, struct alarm *alarm);

void update_timer_compare(int timid, u64 time, struct list_head *alarm_list);

void *heap_alloc(u16 tag, u32 bytes);
int heap_free(struct heaptag *tag);

int thread_start(struct thread *thread, int intr_state);
int thread_init_and_start(struct thread *thread, int intr_state);
int thread_leave(int unk, int unk2, int intr_state, int release);

unsigned int thread_delay_cb(void *user);

int read_sys_time(iop_sys_clock_t *clock);

void waitlist_insert(struct thread *thread, struct event *event, s32 priority);

int check_thread_stack();

#endif // THCOMMON_H_
