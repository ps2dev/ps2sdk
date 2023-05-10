/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * startup structs.
 */

#ifndef __STARTUP_H__
#define __STARTUP_H__


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ARGS   16
#define MAX_ARGLEN 256

// System provided arguments when loaded normally
struct sargs {
    int32_t argc;
    char* argv[MAX_ARGS];
    char payload[MAX_ARGLEN];
};

// Sent arguments through __start (by ps2link for instance)
struct sargs_start {
    int32_t pid;
    struct sargs args;
};

#ifdef __cplusplus
}
#endif

#endif /* __STARTUP_H__ */
