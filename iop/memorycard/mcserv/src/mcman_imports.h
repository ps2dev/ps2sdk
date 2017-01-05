#ifndef MCMAN_IMPORTS_H
#define MCMAN_IMPORTS_H

#include "types.h"
#include "irx.h"

#define MCMAN 	0
#define XMCMAN 	1

char mcman_modname[8] = "mcman\0\0\0";
int mcman_type = MCMAN;

/* 05 */ int  (*pMcDetectCard)(int port, int slot);
/* 06 */ int  (*pMcOpen)(int port, int slot, char *filename, int flags);
/* 07 */ int  (*pMcClose)(int fd);
/* 08 */ int  (*pMcRead)(int fd, void *buf, int length);
/* 09 */ int  (*pMcWrite)(int fd, void *buf, int length);
/* 10 */ int  (*pMcSeek)(int fd, int offset, int origin);
/* 11 */ int  (*pMcFormat)(int port, int slot);
/* 12 */ int  (*pMcGetDir)(int port, int slot, char *dirname, int flags, int nument, sceMcTblGetDir *info);
/* 13 */ int  (*pMcDelete)(int port, int slot, char *filename, int flags);
/* 14 */ int  (*pMcFlush)(int fd);
/* 15 */ int  (*pMcChDir)(int port, int slot, char *newdir, char *currentdir);
/* 16 */ int  (*pMcSetFileInfo)(int port, int slot, char *filename, sceMcTblGetDir *info, int flags);
/* 17 */ int  (*pMcEraseBlock)(int port, int block, void **pagebuf, void *eccbuf);
/* 18 */ int  (*pMcReadPage)(int port, int slot, int page, void *buf);
/* 19 */ int  (*pMcWritePage)(int port, int slot, int page, void *pagebuf, void *eccbuf);
/* 20 */ void (*pMcDataChecksum)(void *buf, void *ecc);
/* 29 */ int  (*pMcReadPS1PDACard)(int port, int slot, int page, void *buf);
/* 30 */ int  (*pMcWritePS1PDACard)(int port, int slot, int page, void *buf);
/* 36 */ int  (*pMcUnformat)(int port, int slot);
/* 37 */ int  (*pMcRetOnly)(int fd);
/* 38 */ int  (*pMcGetFreeClusters)(int port, int slot);
/* 39 */ int  (*pMcGetMcType)(int port, int slot);
/* 40 */ void (*pMcSetPS1CardFlag)(int flag);

/* Available in XMCMAN only */
/* 21 */ int  (*pMcDetectCard2)(int port, int slot);
/* 22 */ int  (*pMcGetFormat)(int port, int slot);
/* 23 */ int  (*pMcGetEntSpace)(int port, int slot, char *dirname);
/* 24 */ int  (*pMcReplaceBadBlock)(void);
/* 42 */ struct irx_id *(*pMcGetModuleInfo)(void);
/* 45 */ int  (*pMcCheckBlock)(int port, int slot, int block);

#endif
