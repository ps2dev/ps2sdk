#ifndef _MODULE_DEBUG_H
#define _MODULE_DEBUG_H

// #define MINI_DRIVER

#ifndef MINI_DRIVER
#define M_PRINTF(format, args...) printf("BDM: " format, ##args)
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

// u64 when being used in a printf statement is split into two u32 values
#define U64_2XU32(val) \
    u32 val##_u32[2]; \
    memcpy(val##_u32, &val, sizeof(val##_u32))

#ifdef DEBUG
#define DEBUG_U64_2XU32(val) U64_2XU32(val)
#else
#define DEBUG_U64_2XU32(val) do { } while (0)
#endif

#endif
