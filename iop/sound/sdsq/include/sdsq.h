/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _SDSQ_H
#define _SDSQ_H

#ifdef _IOP
#include <tamtypes.h>
#endif

typedef struct sceSeqMidiCompBlock_
{
	u16 compOption;
	u16 compTableSize;
	u8 compTable[];
} sceSeqMidiCompBlock;

typedef struct sceSeqMidiDataBlock_
{
	unsigned int sequenceDataOffset;
	u16 Division;
	sceSeqMidiCompBlock compBlock[];
} sceSeqMidiDataBlock;

typedef struct SceSdSqMidiData_
{
	u32 readStatus;
	u32 midiNumber;
	sceSeqMidiDataBlock *midiData;
	u32 offset;
	u32 nextOffset;
	u32 division;
	u32 compMode;
	u32 compTableSize;
	u32 deltaTime;
	u8 lastStatus;
	u8 reserve[3];
	u32 messageLength;
	u8 message[8];
	u32 originalMessageLength;
	u8 originalMessage[12];
} SceSdSqMidiData;

typedef struct SceSdSqSongData_
{
	u32 readStatus;
	u32 songNumber;
	void *topAddr;
	u32 offset;
	u32 nextOffset;
	u8 message[3];
	u8 reserve;
} SceSdSqSongData;

struct SceSdSqCompTableData_
{
	u8 status;
	u8 data;
};

typedef struct SceSdSqCompTableData_ SceSdSqCompTableData;
typedef struct SceSdSqCompTableData_ SceSdSqPolyKeyData;

typedef struct SceSdSqCompTableNoteOnEvent_
{
	u8 status;
	u8 note;
	u8 velocity;
	u8 reserve;
} SceSdSqCompTableNoteOnEvent;

extern int sceSdSqGetMaxMidiNumber(void *addr);
extern int sceSdSqGetMaxSongNumber(void *addr);
extern int sceSdSqInitMidiData(void *addr, u32 midiNumber, SceSdSqMidiData *midiData);
extern int sceSdSqReadMidiData(SceSdSqMidiData *midiData);
extern int sceSdSqInitSongData(void *addr, u32 songNumber, SceSdSqSongData *songData);
extern int sceSdSqReadSongData(SceSdSqSongData *songData);
extern int sceSdSqGetMaxCompTableIndex(void *addr, u32 midiNumber);
extern int sceSdSqGetCompTableOffset(void *addr, u32 midiNumber, u32 *offset);
extern int sceSdSqGetCompTableDataByIndex(void *addr, u32 midiNumber, u32 compTableIndex, SceSdSqCompTableData *data);
extern int sceSdSqGetNoteOnEventByPolyKeyPress(
	void *addr, u32 midiNumber, const SceSdSqPolyKeyData *pData, SceSdSqCompTableNoteOnEvent *kData);
extern int sceSdSqCopyMidiData(SceSdSqMidiData *to, const SceSdSqMidiData *from);
extern int sceSdSqCopySongData(SceSdSqSongData *to, const SceSdSqSongData *from);

#define sdsq_IMPORTS_start DECLARE_IMPORT_TABLE(sdsq, 1, 1)
#define sdsq_IMPORTS_end END_IMPORT_TABLE

#define I_sceSdSqGetMaxMidiNumber DECLARE_IMPORT(4, sceSdSqGetMaxMidiNumber)
#define I_sceSdSqGetMaxSongNumber DECLARE_IMPORT(5, sceSdSqGetMaxSongNumber)
#define I_sceSdSqInitMidiData DECLARE_IMPORT(6, sceSdSqInitMidiData)
#define I_sceSdSqReadMidiData DECLARE_IMPORT(7, sceSdSqReadMidiData)
#define I_sceSdSqInitSongData DECLARE_IMPORT(8, sceSdSqInitSongData)
#define I_sceSdSqReadSongData DECLARE_IMPORT(9, sceSdSqReadSongData)
#define I_sceSdSqGetMaxCompTableIndex DECLARE_IMPORT(10, sceSdSqGetMaxCompTableIndex)
#define I_sceSdSqGetCompTableOffset DECLARE_IMPORT(11, sceSdSqGetCompTableOffset)
#define I_sceSdSqGetCompTableDataByIndex DECLARE_IMPORT(12, sceSdSqGetCompTableDataByIndex)
#define I_sceSdSqGetNoteOnEventByPolyKeyPress DECLARE_IMPORT(13, sceSdSqGetNoteOnEventByPolyKeyPress)
#define I_sceSdSqCopyMidiData DECLARE_IMPORT(14, sceSdSqCopyMidiData)
#define I_sceSdSqCopySongData DECLARE_IMPORT(15, sceSdSqCopySongData)

#endif
