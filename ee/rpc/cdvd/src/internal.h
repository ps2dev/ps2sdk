
#ifndef LIBCDVD_INTERNAL_H
#define LIBCDVD_INTERNAL_H

extern s32 cdDebug;

extern volatile s32 cdCallbackNum;
extern volatile s32 cbSema;

extern s32 cdThreadId;
extern ee_thread_t cdThreadParam;

extern s32 bindNCmd;
extern s32 bindSCmd;

extern s32 nCmdSemaId;
extern s32 sCmdSemaId;

extern s32 nCmdNum;

extern u8 sCmdRecvBuff[];
extern u8 nCmdRecvBuff[];

void cdSemaInit(void);

void cdCallback(void *funcNum);

s32  cdSyncS(s32 mode);

#endif	/* LIBCDVD_INTERNAL_H */
