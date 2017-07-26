/**
 * @file
 * Sound driver library.
 */

#ifndef __LIBSD_H__
#define __LIBSD_H__

#include <libsd-common.h>

#ifdef __cplusplus
extern "C" {
#endif

int  SdInit(int flag);
void SdSetParam(u16 entry, u16 value);
u16  SdGetParam(u16 entry);
void SdSetSwitch(u16 entry, u32 value);
u32  SdGetSwitch(u16 entry);
void SdSetAddr(u16 entry, u32 value);
u32  SdGetAddr(u16 entry);
void SdSetCoreAttr(u16 entry, u16 value);
u16  SdGetCoreAttr(u16 entry);
u16  SdNote2Pitch(u16 center_note, u16 center_fine, u16 note, s16 fine);
u16  SdPitch2Note(u16 center_note, u16 center_fine, u16 pitch);
int  SdProcBatch(sceSdBatch* batch, u32 returns[], u32 num);
int  SdProcBatchEx(sceSdBatch* batch, u32 returns[], u32 num, u32 voice);
int  SdVoiceTrans(s16 channel, u16 mode, u8 *m_addr, u8 *s_addr, u32 size);
int  SdBlockTrans(s16 channel, u16 mode, u8 *m_addr, u32 size, ...);
u32  SdVoiceTransStatus (s16 channel, s16 flag);
u32  SdBlockTransStatus (s16 channel, s16 flag);
int  SdSetEffectAttr (int core, sceSdEffectAttr *attr);
void SdGetEffectAttr (int core, sceSdEffectAttr *attr);
int  SdClearEffectWorkArea (int core, int channel, int effect_mode);

#ifdef __cplusplus
}
#endif

#endif /* __LIBSD_H__ */
