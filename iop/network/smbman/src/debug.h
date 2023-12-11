/*
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
#define DPRINTF(args...) printf("SMBMAN: "args)
#else
#define DPRINTF(args...) \
    do {                 \
    } while (0)
#endif

#endif
