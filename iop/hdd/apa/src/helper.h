
#ifndef _HELPER_H
#define _HELPER_H

/* 06 */ void dev9Shutdown();

typedef struct {
u32 devicePresent;
u32 supportPACKET;
u32 totalLBA;
u32 securityStatus;
} t_shddInfo;

#define ATAD_MODE_READ 0x00
#define ATAD_MODE_WRITE 0x01

/* 04 */ t_shddInfo *atadInit(u32 device);
/* 09 */ int atadDmaTransfer(int device, void *buf, u32 lba, u32 size, u32 mode);
/* 11 */ int atadSceUnlock(s32 device, u8 *key);
/* 13 */ int atadIdle(s32 device, u8 time);
/* 14 */ int atadSceIdentifyDrive(s32 device, u16 *buffer);
/* 15 */ int atadGetStatus(s32 device);
/* 16 */ int atadUpdateAttrib(s32 device);
/* 17 */ int atadFlushCache(s32 device);

#endif
