/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * The lock API functions required by newlib.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/lock.h>
#include <kernel.h>

// Structure representing the lock
struct __lock {
    int32_t sem_id;
    int32_t thread_id;
    int32_t count;
};

#ifdef F___lock___sfp_recursive_mutex
struct __lock __lock___sfp_recursive_mutex;
#endif

#ifdef F___lock___atexit_recursive_mutex
struct __lock __lock___atexit_recursive_mutex;
#endif

#ifdef F___lock___at_quick_exit_mutex
struct __lock __lock___at_quick_exit_mutex;
#endif

#ifdef F___lock___malloc_recursive_mutex
struct __lock __lock___malloc_recursive_mutex;
#endif

#ifdef F___lock___env_recursive_mutex
struct __lock __lock___env_recursive_mutex;
#endif

#ifdef F___lock___tz_mutex
struct __lock __lock___tz_mutex;
#endif

#ifdef F___lock___dd_hash_mutex
struct __lock __lock___dd_hash_mutex;
#endif

#ifdef F___lock___arc4random_mutex
struct __lock __lock___arc4random_mutex;
#endif

static inline void __common_lock_init(_LOCK_T lock)
{
    ee_sema_t sema;
    sema.init_count = 1;
    sema.max_count  = 1;
    sema.option     = 0;
    sema.attr       = 0;
    sema.wait_threads = 0;
    lock->sem_id = CreateSema(&sema);
    lock->count = -1;
    lock->thread_id = -1;
}

static inline void __common_lock_init_recursive(_LOCK_T lock)
{
    ee_sema_t sema;
    sema.init_count = 1;
    sema.max_count  = 1;
    sema.option     = 0;
    sema.attr       = 0;
    sema.wait_threads = 0;
    lock->sem_id = CreateSema(&sema);
    lock->count = 0;
    lock->thread_id = -1;
}

static inline void __common_lock_close(_LOCK_T lock)
{
    DeleteSema(lock->sem_id);
}

static inline void __common_lock_close_recursive(_LOCK_T lock)
{
    DeleteSema(lock->sem_id);
}

#ifdef F___retarget_lock_init
void __retarget_lock_init(_LOCK_T *lock)
{
    _LOCK_T new_lock = (_LOCK_T)malloc(sizeof(struct __lock));
    __common_lock_init(new_lock);
    *lock = new_lock;
}
#endif

#ifdef F___retarget_lock_init_recursive
void __retarget_lock_init_recursive(_LOCK_T *lock)
{
    _LOCK_T new_lock = (_LOCK_T)malloc(sizeof(struct __lock));
    __common_lock_init_recursive(new_lock);
    *lock = new_lock;
}
#endif

#ifdef F___retarget_lock_close
void __retarget_lock_close(_LOCK_T lock)
{
	__common_lock_close(lock);
    free(lock);
}
#endif

#ifdef F___retarget_lock_close_recursive
void __retarget_lock_close_recursive(_LOCK_T lock)
{
    __common_lock_close_recursive(lock);
    free(lock);
}
#endif

#ifdef F___retarget_lock_acquire
void __retarget_lock_acquire(_LOCK_T lock)
{
	WaitSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_acquire_recursive
void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    bool starting = false;
    int32_t thread_id = GetThreadId();
    starting = lock->count == 0;
    if (starting) {
        lock->thread_id = thread_id;
    }
    if (lock->thread_id == thread_id) {
        lock->count++;
        if (starting) {
            WaitSema(lock->sem_id);
        }
    } else {
        WaitSema(lock->sem_id);
        // Reached here means that the lock was acquired by another thread
        // so now we need to make it ours
        // We can't put the lock->count++ before the WaitSema because it will
        // cause a deadlock
        lock->thread_id = thread_id;
        lock->count++;
    }
}
#endif

#ifdef F___retarget_lock_try_acquire
int __retarget_lock_try_acquire(_LOCK_T lock)
{
	return PollSema(lock->sem_id) > 0 ? 0 : 1;
}
#endif

#ifdef F___retarget_lock_try_acquire_recursive
int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    int res = 0;
    bool starting = false;
    int32_t thread_id = GetThreadId();
    starting = lock->count == 0;
    if (starting) {
        lock->thread_id = thread_id;
    }
    if (lock->thread_id == thread_id) {
        lock->count++;
        if (starting) {
            res = PollSema(lock->sem_id) > 0 ? 0 : 1;
        }
    } else {
        res = PollSema(lock->sem_id) > 0 ? 0 : 1;
        // Reached here means that the lock was acquired by another thread
        // so now we need to make it ours
        // We can't put the lock->count++ before the WaitSema because it will
        // cause a deadlock
        lock->thread_id = thread_id;
        lock->count++;
    }
    return res;
}
#endif

#ifdef F___retarget_lock_release
void __retarget_lock_release(_LOCK_T lock)
{
	SignalSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_release_recursive
void __retarget_lock_release_recursive(_LOCK_T lock)
{
    bool tobeRelease = false;
    int32_t thread_id = GetThreadId();
    if (lock->thread_id != thread_id) {
        // error this shouldn't never happen
        perror("Error: Trying to release a lock that was not acquired by the current thread");
        exit(1);
    }

    tobeRelease = lock->count == 1;
    lock->count--;
    if (lock->count == 0) {
        lock->thread_id = -1;
    }
    if (tobeRelease) {
        SignalSema(lock->sem_id);
    }
}
#endif

#ifdef F___locks_init
extern struct __lock __lock___malloc_recursive_mutex;
extern struct __lock __lock___atexit_recursive_mutex;
extern struct __lock __lock___at_quick_exit_mutex;
extern struct __lock __lock___sfp_recursive_mutex;
extern struct __lock __lock___env_recursive_mutex;
extern struct __lock __lock___tz_mutex;
extern struct __lock __lock___dd_hash_mutex;
extern struct __lock __lock___arc4random_mutex;

void __locks_init()
{
    _LOCK_T lock_malloc = &__lock___malloc_recursive_mutex;
    _LOCK_T lock_atexit = &__lock___atexit_recursive_mutex;
    _LOCK_T lock_quick_exit = &__lock___at_quick_exit_mutex;
    _LOCK_T lock_sfp = &__lock___sfp_recursive_mutex;
    _LOCK_T lock_env = &__lock___env_recursive_mutex;
    _LOCK_T lock_tz = &__lock___tz_mutex;
    _LOCK_T lock_dd_hash = &__lock___dd_hash_mutex;
    _LOCK_T lock_arc4random = &__lock___arc4random_mutex;
    
    __common_lock_init_recursive(lock_malloc);
    __common_lock_init_recursive(lock_atexit);
    __common_lock_init(lock_quick_exit);
    __common_lock_init_recursive(lock_sfp);
    __common_lock_init_recursive(lock_env);
    __common_lock_init(lock_tz);
    __common_lock_init(lock_dd_hash);
    __common_lock_init(lock_arc4random);
}
#endif

#ifdef F___locks_deinit
extern struct __lock __lock___malloc_recursive_mutex;
extern struct __lock __lock___atexit_recursive_mutex;
extern struct __lock __lock___at_quick_exit_mutex;
extern struct __lock __lock___sfp_recursive_mutex;
extern struct __lock __lock___env_recursive_mutex;
extern struct __lock __lock___tz_mutex;
extern struct __lock __lock___dd_hash_mutex;
extern struct __lock __lock___arc4random_mutex;

void __locks_deinit()
{
    _LOCK_T lock_malloc = &__lock___malloc_recursive_mutex;
    _LOCK_T lock_atexit = &__lock___atexit_recursive_mutex;
    _LOCK_T lock_quick_exit = &__lock___at_quick_exit_mutex;
    _LOCK_T lock_sfp = &__lock___sfp_recursive_mutex;
    _LOCK_T lock_env = &__lock___env_recursive_mutex;
    _LOCK_T lock_tz = &__lock___tz_mutex;
    _LOCK_T lock_dd_hash = &__lock___dd_hash_mutex;
    _LOCK_T lock_arc4random = &__lock___arc4random_mutex;
    

    __common_lock_close_recursive(lock_malloc);
    __common_lock_close_recursive(lock_atexit);
    __common_lock_close(lock_quick_exit);
    __common_lock_close_recursive(lock_sfp);
    __common_lock_close_recursive(lock_env);
    __common_lock_close(lock_tz);
    __common_lock_close(lock_dd_hash);
    __common_lock_close(lock_arc4random);
}
#endif