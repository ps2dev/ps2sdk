/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef __PS2PROF_H__
#define __PS2PROF_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Start the profiler.
 * If the profiler is already running, this function stop previous one,
 * and ignore the result.
 * Finally, it initializes a new profiler session.
 */
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_start(void);
/**
 * Stop the profiler.
 * If the profiler is not running, this function does nothing.
 * @param filename The name of the file to write the profiling data to.
 * @param should_dump If 1, the profiling data will be written to the file.
 * If 0, the profiling data will be discarded.
 */
__attribute__((__no_instrument_function__, __no_profile_instrument_function__))
void gprof_stop(const char* filename, int should_dump);

#ifdef __cplusplus
}
#endif

#endif	/* __PS2PROF_H__ */
