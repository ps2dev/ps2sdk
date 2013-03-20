#ifndef _MASS_DEBUG_H
#define _MASS_DEBUG_H 1

#ifdef DEBUG
#define XPRINTF(args...) printf(args)
#define iXPRINTF(args...) Kprintf(args)
#else
#define XPRINTF(args...)
#define iXPRINTF(args...)
#endif

#endif  /* _MASS_DEBUG_H */
