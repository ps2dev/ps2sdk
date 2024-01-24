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

#endif
