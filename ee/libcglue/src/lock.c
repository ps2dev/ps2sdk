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
#include <sys/lock.h>
#include <kernel.h>

// Structure representing the lock
struct __lock {
    int32_t sem_id;
    int32_t thread_id;
    int32_t count;
};

#ifdef F___retarget_lock_init
void __retarget_lock_init(_LOCK_T *lock)
{
    ee_sema_t sema;

    sema.init_count = 1;
    sema.max_count  = 255;
    sema.option     = 0;
    sema.attr       = 0;
    sema.wait_threads = 0;
    (*lock)->sem_id = CreateSema(&sema);
}
#endif

#ifdef F___retarget_lock_acquire
void __retarget_lock_acquire(_LOCK_T lock)
{
	WaitSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_release
void __retarget_lock_release(_LOCK_T lock)
{
	SignalSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_try_acquire
int __retarget_lock_try_acquire(_LOCK_T lock)
{
	return PollSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_close
void __retarget_lock_close(_LOCK_T lock)
{
	DeleteSema(lock->sem_id);
}
#endif

#ifdef F___retarget_lock_init_recursive
void __retarget_lock_init_recursive(_LOCK_T *lock)
{
	ee_sema_t sema;

    sema.init_count = 1;
    sema.max_count  = 255;
    sema.option     = 0;
    sema.attr       = 0;
    sema.wait_threads = 0;
    (*lock)->sem_id = CreateSema(&sema);
    (*lock)->count = 0;
    (*lock)->thread_id = GetThreadId();
}
#endif

#ifdef F___retarget_lock_acquire_recursive
void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    int32_t thread_id = GetThreadId();
    if (lock->count == 0 || lock->thread_id != thread_id) {
        WaitSema(lock->sem_id);
    }
    lock->count++;
}
#endif

#ifdef F___retarget_lock_release_recursive
void __retarget_lock_release_recursive(_LOCK_T lock)
{
    int32_t thread_id = GetThreadId();
    if (lock->count == 1 || lock->thread_id != thread_id) {
        SignalSema(lock->sem_id);
    }
    lock->count--;
}
#endif

#ifdef F___retarget_lock_try_acquire_recursive
int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    int res = 0;
    int32_t thread_id = GetThreadId();
    if (lock->count == 0 || lock->thread_id != thread_id) {
        res = PollSema(lock->sem_id) <= 0;
    }
    lock->count++;
    return res;
}
#endif

#ifdef F___retarget_lock_close_recursive
void __retarget_lock_close_recursive(_LOCK_T lock)
{
    DeleteSema(lock->sem_id);
}
#endif