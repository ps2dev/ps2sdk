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
 * The global init/deinit code for our crt0.
 */

#include <stdlib.h>
#include <unistd.h>

void __init_cwd(int argc, char ** argv);
void _libcglue_timezone_update();
void _libcglue_rtc_update();
void __fdman_init();
void pthread_init();
void pthread_terminate();
void __fdman_deinit();
void __locks_init();
void __locks_deinit();

int chdir(const char *path);

#ifdef F___gprof_init
/* Note: This function is being called from _init and it is overrided when compiling with -pg */
__attribute__((weak))
void __gprof_init() {}
#else
void __gprof_init();
#endif

#ifdef F___gprof_cleanup
/* Note: This function is being called from _exit and it is overrided when compiling with -pg */
__attribute__((weak))
void __gprof_cleanup() {}
#else
void __gprof_cleanup();
#endif

#ifdef F___libpthreadglue_init
/* Note: This function is being called from __libcglue_init.
* It is a weak function because can be override by user program
*/
__attribute__((weak))
void __libpthreadglue_init()
{
    pthread_init();
}
#else
void __libpthreadglue_init();
#endif

#ifdef F___libpthreadglue_deinit
/* Note: This function is being registered by _libcglue_init as an exit handler.
* It is a weak function because can be override by user program
*/
__attribute__((weak))
void __libpthreadglue_deinit()
{
	pthread_terminate();
}
#else
void __libpthreadglue_deinit();
#endif

#ifdef F__libcglue_init
extern void _fini(void) __attribute__((weak));

/* Keep the normal exit path in the same archive member as libcglue init,
 * so minimal startup overrides do not link newlib exit processing. */
static int exit_initialized;

void __libcglue_exit(int status)
{
    if (exit_initialized)
        exit(status);
}

__attribute__((weak))
void _libcglue_init()
{
	/* Initialize lock API */
	__locks_init();

	/* Initialize filedescriptor management */
	__fdman_init();

	/* Initialize pthread library */
	__libpthreadglue_init();

    _libcglue_timezone_update();
    _libcglue_rtc_update();
	
	__gprof_init();

    /* Register before crt0 runs constructors. LIFO order keeps pthreads
     * available for user callbacks and finalizers, then newlib flushes stdio. */
    if (atexit(__libpthreadglue_deinit) != 0 || (_fini && atexit(_fini) != 0))
        _exit(EXIT_FAILURE);
    exit_initialized = 1;
}
#endif

#ifdef F__libcglue_deinit
__attribute__((weak))
void _libcglue_deinit()
{
	__gprof_cleanup();
	__fdman_deinit();
	__locks_deinit();
}
#endif

#ifdef F__libcglue_args_parse
__attribute__((weak))
void _libcglue_args_parse(int argc, char ** argv)
{
    /* Initialize cwd from this program's path */
	__init_cwd(argc, argv);
}
#endif

