#ifndef _MASS_DEBUG_H
#define _MASS_DEBUG_H 1

#define MODNAME "IEEE1394_disk"
#ifdef DEBUG
#define XPRINTF(args...)  printf(MODNAME ": "args)
#define iXPRINTF(args...) Kprintf(MODNAME ": "args)
#else
#define XPRINTF(args...) \
    do {                 \
    } while (0)
#define iXPRINTF(args...) \
    do {                  \
    } while (0)
#endif

#endif /* _MASS_DEBUG_H */
