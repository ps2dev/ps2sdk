#ifndef _MODULE_DEBUG_H
#define _MODULE_DEBUG_H

// #define MINI_DRIVER

#ifndef MINI_DRIVER

#ifdef RPC_PRINTF
#include "stdio.h"
#include "rpc_printf.h"
#define M_PRINTF(format, args...)              \
    do {                                       \
        printf("MX4SIO: " format, ##args);     \
        rpc_printf("MX4SIO: " format, ##args); \
    } while (0)
#else
#include "stdio.h"
#define M_PRINTF(format, args...) printf("MX4SIO: " format, ##args);
#endif
#else
#define M_PRINTF(format, args...) \
    do {                          \
    } while (0)
#endif


#ifdef DEBUG
#define M_DEBUG M_PRINTF
#else
#define M_DEBUG(format, args...) \
    do {                         \
    } while (0)
#endif

#define U64_2XU32(val) ((u32 *)val)[1], ((u32 *)val)[0]

#endif
