/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# POSIX threads sample: thread creation and joining, mutex,
# condition variable and thread specific data.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <kernel.h>

#if defined(SCREEN_DEBUG)
#include <debug.h>
#endif

#if defined(SCREEN_DEBUG)
#define custom_printf(...) \
    do {                   \
        printf(__VA_ARGS__);     \
        scr_printf(__VA_ARGS__); \
    } while (0)
#else
#define custom_printf(...) printf(__VA_ARGS__)
#endif

#define NUM_THREADS 4
#define INCREMENTS  1000
#define ITEMS       10
#define STACK_SIZE  (64 * 1024)

/* Create a thread with an explicit stack size. */
static int create_thread(pthread_t *thread, void *(*entry)(void *), void *arg)
{
    pthread_attr_t attr;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, STACK_SIZE);
    ret = pthread_create(thread, &attr, entry, arg);
    pthread_attr_destroy(&attr);
    return ret;
}

static int report(const char *name, int ok)
{
    custom_printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

/* Test 1: threads receive an argument and return a value to pthread_join(). */
static void *double_worker(void *arg)
{
    return (void *)((intptr_t)arg * 2);
}

static int test_create_join(void)
{
    pthread_t threads[NUM_THREADS];
    int i, ok = 1;

    for (i = 0; i < NUM_THREADS; i++) {
        if (create_thread(&threads[i], double_worker, (void *)(intptr_t)(i + 1)) != 0) {
            custom_printf("pthread_create failed for thread %d\n", i);
            ok = 0;
            threads[i] = 0;
        }
    }
    for (i = 0; i < NUM_THREADS; i++) {
        void *result = NULL;

        if (threads[i] == 0)
            continue;
        if (pthread_join(threads[i], &result) != 0 || (intptr_t)result != (intptr_t)(i + 1) * 2) {
            custom_printf("thread %d returned %d, expected %d\n", i, (int)(intptr_t)result, (i + 1) * 2);
            ok = 0;
        }
    }
    return report("create and join", ok);
}

/* Test 2: several threads increment one counter protected by a mutex. */
static pthread_mutex_t counter_mutex;
static int counter;

static void *counter_worker(void *arg)
{
    int i;

    (void)arg;
    for (i = 0; i < INCREMENTS; i++) {
        pthread_mutex_lock(&counter_mutex);
        counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

static int test_mutex(void)
{
    pthread_t threads[NUM_THREADS];
    int i, ok = 1;

    pthread_mutex_init(&counter_mutex, NULL);
    counter = 0;
    for (i = 0; i < NUM_THREADS; i++) {
        if (create_thread(&threads[i], counter_worker, NULL) != 0) {
            custom_printf("pthread_create failed for thread %d\n", i);
            ok = 0;
            threads[i] = 0;
        }
    }
    for (i = 0; i < NUM_THREADS; i++) {
        if (threads[i] != 0)
            pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&counter_mutex);

    custom_printf("counter is %d, expected %d\n", counter, NUM_THREADS * INCREMENTS);
    return report("mutex", ok && counter == NUM_THREADS * INCREMENTS);
}

/* Test 3: a producer thread hands ITEMS values to the main thread, one at a time. */
static pthread_mutex_t box_mutex;
static pthread_cond_t box_not_empty;
static pthread_cond_t box_not_full;
static int box_value;
static int box_full;

static void *producer(void *arg)
{
    int i;

    (void)arg;
    for (i = 1; i <= ITEMS; i++) {
        pthread_mutex_lock(&box_mutex);
        while (box_full)
            pthread_cond_wait(&box_not_full, &box_mutex);
        box_value = i;
        box_full = 1;
        pthread_cond_signal(&box_not_empty);
        pthread_mutex_unlock(&box_mutex);
    }
    return NULL;
}

static int test_condition_variable(void)
{
    pthread_t thread;
    int i, sum = 0, expected = ITEMS * (ITEMS + 1) / 2;

    pthread_mutex_init(&box_mutex, NULL);
    pthread_cond_init(&box_not_empty, NULL);
    pthread_cond_init(&box_not_full, NULL);
    box_full = 0;

    if (create_thread(&thread, producer, NULL) != 0) {
        custom_printf("pthread_create failed\n");
        return report("condition variable", 0);
    }
    for (i = 0; i < ITEMS; i++) {
        pthread_mutex_lock(&box_mutex);
        while (!box_full)
            pthread_cond_wait(&box_not_empty, &box_mutex);
        sum += box_value;
        box_full = 0;
        pthread_cond_signal(&box_not_full);
        pthread_mutex_unlock(&box_mutex);
    }
    pthread_join(thread, NULL);

    pthread_cond_destroy(&box_not_empty);
    pthread_cond_destroy(&box_not_full);
    pthread_mutex_destroy(&box_mutex);

    custom_printf("sum of received values is %d, expected %d\n", sum, expected);
    return report("condition variable", sum == expected);
}

/* Test 4: every thread keeps its own value under the same key. */
static pthread_key_t tls_key;

static void tls_destructor(void *value)
{
    free(value);
}

static void *tls_worker(void *arg)
{
    int id = (int)(intptr_t)arg;
    int *value = malloc(sizeof(int));
    int *stored;

    if (value == NULL)
        return (void *)0;
    *value = id * 10;
    pthread_setspecific(tls_key, value);

    usleep(1000); /* let the other threads run and set their own values */

    stored = pthread_getspecific(tls_key);
    return (void *)(intptr_t)(stored != NULL && *stored == id * 10);
}

static int test_thread_specific_data(void)
{
    pthread_t threads[NUM_THREADS];
    int i, ok = 1;

    if (pthread_key_create(&tls_key, tls_destructor) != 0) {
        custom_printf("pthread_key_create failed\n");
        return report("thread specific data", 0);
    }
    for (i = 0; i < NUM_THREADS; i++) {
        if (create_thread(&threads[i], tls_worker, (void *)(intptr_t)(i + 1)) != 0) {
            custom_printf("pthread_create failed for thread %d\n", i);
            ok = 0;
            threads[i] = 0;
        }
    }
    for (i = 0; i < NUM_THREADS; i++) {
        void *result = NULL;

        if (threads[i] == 0)
            continue;
        if (pthread_join(threads[i], &result) != 0 || (intptr_t)result != 1) {
            custom_printf("thread %d did not see its own value\n", i);
            ok = 0;
        }
    }
    pthread_key_delete(tls_key);
    return report("thread specific data", ok);
}

int main(int argc, char *argv[])
{
    int failures = 0;

    (void)argc;
    (void)argv;

#if defined(SCREEN_DEBUG)
    init_scr();
    sleep(3);
#endif

    custom_printf("\n\nStarting PTHREAD TESTS...\n");

    failures += test_create_join();
    failures += test_mutex();
    failures += test_condition_variable();
    failures += test_thread_specific_data();

    if (failures == 0)
        custom_printf("All tests passed\n");
    else
        custom_printf("%d test(s) failed\n", failures);

    SleepThread();

    return 0;
}
