#ifndef _MASS_DEBUG_H
#define _MASS_DEBUG_H 1

#if defined(BUILDING_USBHDFSD)
#define MODNAME "usbhdfsd"
#elif defined(BUILDING_IEEE1394_DISK)
#define MODNAME "IEEE1394_disk"
#else
#define MODNAME "VFAT"
#endif

#ifndef MINI_DRIVER
#define M_PRINTF(format, args...) printf(MODNAME ": " format, ##args)
#else
#define M_PRINTF(format, args...) \
    do {                          \
    } while (0)
#endif

#ifdef DEBUG
#define XPRINTF(args...) printf(MODNAME ": " args)
#define iXPRINTF(args...) Kprintf(MODNAME ": "args)
#else
#define XPRINTF(args...) \
    do {                 \
    } while (0)
#define iXPRINTF(args...) \
    do {                  \
    } while (0)
#endif

#define M_DEBUG XPRINTF

#endif /* _MASS_DEBUG_H */
