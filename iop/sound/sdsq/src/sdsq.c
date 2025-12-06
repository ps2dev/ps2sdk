/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifdef _IOP
#include "irx_imports.h"
#else
#include <stdint.h>
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#endif
#include <sdsq.h>

#ifdef _IOP
IRX_ID("Sound_Data_SQ", 2, 1);
#endif
// Based on the module from SCE SDK 3.1.0.

struct sceHardSynthVersionChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	u16 reserved;
	u8 versionMajor;
	u8 versionMinor;
};
typedef struct sceHardSynthVersionChunk_ sceHardSynthVersionChunk;
typedef struct sceHardSynthVersionChunk_ sceSeqVersionChunk;

typedef struct sceSeqHeaderChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int fileSize;
	unsigned int songChunkAddr;
	unsigned int midiChunkAddr;
	unsigned int seSequenceChunkAddr;
	unsigned int seSongChunkAddr;
} sceSeqHeaderChunk;

typedef struct sceSeqMidiChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxMidiNumber;
	unsigned int midiOffsetAddr[];
} sceSeqMidiChunk;

typedef struct sceSeqSongChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxSongNumber;
	unsigned int songOffsetAddr[];
} sceSeqSongChunk;

typedef struct sceSeqSeSequenceChunk_
{
	unsigned int Creator;
	unsigned int Type;
	unsigned int chunkSize;
	unsigned int maxSeSequenceSetNumber;
	unsigned int tableOffset;
	u8 seSequenceMasterVolume;
	char seSequenceMasterPanpot;
	u16 seSequenceMasterTimeScale;
	unsigned int dmy0;
	unsigned int dmy1;
	unsigned int seSequenceSetOffsetAddr[];
} sceSeqSeSequenceChunk;

struct sdsq_info
{
	sceSeqVersionChunk *m_vers;
	sceSeqHeaderChunk *m_sequ;
	sceSeqMidiChunk *m_midi;
	sceSeqSongChunk *m_song;
};

#ifdef _IOP
extern struct irx_export_table _exp_sdsq;
#endif

static unsigned int do_get_vers_sequ_chunk(sceSeqVersionChunk *indata, struct sdsq_info *dinfo)
{
	dinfo->m_vers = 0;
	dinfo->m_sequ = 0;
	dinfo->m_midi = 0;
	dinfo->m_song = 0;
	dinfo->m_vers = indata;
	if ( indata->Creator != 0x53434549 || indata->Type != 0x56657273 )
	{
		dinfo->m_vers = 0;
		return 0x8104000E;
	}
	if ( (indata->chunkSize & 0x80000000) )
	{
		dinfo->m_vers = 0;
		return 0x8104002F;
	}
	dinfo->m_sequ = (sceSeqHeaderChunk *)((char *)indata + indata->chunkSize);
	if ( dinfo->m_sequ->Creator != 0x53434549 || dinfo->m_sequ->Type != 0x53657175 )
	{
		dinfo->m_vers = 0;
		dinfo->m_sequ = 0;
		return 0x8104002F;
	}
	return 0;
}

static unsigned int do_get_midi_chunk(void *indata, struct sdsq_info *dinfo)
{
	if ( dinfo->m_sequ->midiChunkAddr == 0xFFFFFFFF )
		return 0x81049024;
	if ( (dinfo->m_sequ->midiChunkAddr & 0x80000000) )
		return 0x8104002F;
	dinfo->m_midi = (sceSeqMidiChunk *)((char *)indata + dinfo->m_sequ->midiChunkAddr);
	if ( dinfo->m_midi->Creator != 0x53434549 || dinfo->m_midi->Type != 0x4D696469 )
	{
		dinfo->m_midi = 0;
		return 0x8104002F;
	}
	return 0;
}

static unsigned int do_get_song_chunk(void *indata, struct sdsq_info *dinfo)
{
	if ( dinfo->m_sequ->songChunkAddr == 0xFFFFFFFF )
		return 0x81049025;
	if ( (dinfo->m_sequ->songChunkAddr & 0x80000000) )
		return 0x8104002F;
	dinfo->m_song = (sceSeqSongChunk *)((char *)indata + dinfo->m_sequ->songChunkAddr);
	if ( dinfo->m_song->Creator != 0x53434549 || dinfo->m_song->Type != 0x536F6E67 )
	{
		dinfo->m_song = 0;
		return 0x8104002F;
	}
	return 0;
}

static unsigned int do_get_midi_data_block(void *indata, unsigned int idx, sceSeqMidiDataBlock **datablk)
{
	unsigned int result;
	struct sdsq_info dinfo;

	result = do_get_vers_sequ_chunk((sceSeqVersionChunk *)indata, &dinfo);
	if ( result )
		return result;
	result = do_get_midi_chunk(indata, &dinfo);
	if ( result )
		return result;
	if ( dinfo.m_midi->maxMidiNumber < idx )
		return 0x81049026;
	if ( dinfo.m_midi->midiOffsetAddr[idx] == 0xFFFFFFFF )
		return 0x81049026;
	*datablk = (sceSeqMidiDataBlock *)((char *)dinfo.m_midi + dinfo.m_midi->midiOffsetAddr[idx]);
	return 0;
}

int sceSdSqGetMaxMidiNumber(void *addr)
{
	int result;
	struct sdsq_info dinfo;

	result = (int)do_get_vers_sequ_chunk((sceSeqVersionChunk *)addr, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_midi_chunk(addr, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_midi->maxMidiNumber;
}

int sceSdSqGetMaxSongNumber(void *addr)
{
	int result;
	struct sdsq_info dinfo;

	result = (int)do_get_vers_sequ_chunk((sceSeqVersionChunk *)addr, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_song_chunk(addr, &dinfo);
	if ( result )
		return result;
	return (int)dinfo.m_song->maxSongNumber;
}

int sceSdSqInitMidiData(void *addr, u32 midiNumber, SceSdSqMidiData *midiData)
{
	int result;
	struct sdsq_info dinfo;

	result = (int)do_get_vers_sequ_chunk((sceSeqVersionChunk *)addr, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_midi_chunk(addr, &dinfo);
	if ( result )
		return result;
	if ( dinfo.m_midi->maxMidiNumber < midiNumber )
		return (int)0x81049026;
	if ( dinfo.m_midi->midiOffsetAddr[midiNumber] == 0xFFFFFFFF )
		return (int)0x81049026;
	midiData->readStatus = 0;
	midiData->midiNumber = midiNumber;
	midiData->midiData = (sceSeqMidiDataBlock *)((char *)dinfo.m_midi + dinfo.m_midi->midiOffsetAddr[midiNumber]);
	midiData->offset = 0;
	midiData->nextOffset = midiData->midiData->sequenceDataOffset;
	midiData->division = midiData->midiData->Division;
	midiData->compMode = (midiData->nextOffset == 6) ? 0 : 1;
	midiData->compTableSize = (midiData->nextOffset == 6) ? 0 : midiData->midiData->compBlock[0].compTableSize;
	midiData->deltaTime = 0;
	midiData->lastStatus = 0;
	midiData->reserve[0] = 0;
	midiData->reserve[1] = 0;
	midiData->reserve[2] = 0;
	midiData->messageLength = 1;
	midiData->message[0] = 0;
	midiData->message[1] = 0;
	midiData->message[2] = 0;
	midiData->message[3] = 0;
	midiData->message[4] = 0;
	midiData->message[5] = 0;
	midiData->message[6] = 0;
	midiData->message[7] = 0;
	midiData->originalMessageLength = 1;
	midiData->originalMessage[0] = 0;
	midiData->originalMessage[1] = 0;
	midiData->originalMessage[2] = 0;
	midiData->originalMessage[3] = 0;
	midiData->originalMessage[4] = 0;
	midiData->originalMessage[5] = 0;
	midiData->originalMessage[6] = 0;
	midiData->originalMessage[7] = 0;
	midiData->originalMessage[8] = 0;
	midiData->originalMessage[9] = 0;
	midiData->originalMessage[10] = 0;
	midiData->originalMessage[11] = 0;
	return 0;
}

int sceSdSqReadMidiData(SceSdSqMidiData *midiData)
{
	char niceflag;
	u8 cur_message;
	sceSeqMidiDataBlock *midiData_1;
	u32 nextOffset;
	u8 lastStatus;
	u8 *midiData_offs;
	u32 midiData_curval1_1;
	u8 midiData_curval2;
	const u8 *midiData_offs_plusone;
	u32 nextMessageLength;
	u8 *currofssplusone;
	u8 *someaddoffsone;
	int endflg;

	endflg = 3;
	niceflag = 0;
	if ( midiData->readStatus )
		return (int)0x81048001;
	if ( !midiData->originalMessageLength )
	{
		midiData->readStatus = 2;
		return (int)0x8104002F;
	}
	cur_message = midiData->originalMessage[midiData->originalMessageLength - 1];
	midiData_1 = midiData->midiData;
	nextOffset = midiData->nextOffset;
	lastStatus = midiData->lastStatus;
	midiData->originalMessageLength = 0;
	midiData->messageLength = 0;
	midiData_offs = (u8 *)midiData_1 + nextOffset;
	midiData->offset = nextOffset;
	midiData_curval1_1 = 0;
	if ( !(cur_message & 0x80) || lastStatus == 255 )
	{
		u8 midiData_curval1;

		midiData_curval1 = *midiData_offs;
		midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
		midiData->originalMessageLength += 1;
		midiData_offs += 1;
		midiData_curval1_1 = midiData_curval1 & 0x7F;
		while ( midiData_curval1 & 0x80 )
		{
			midiData_curval1 = *midiData_offs;
			midiData_offs += 1;
			midiData->originalMessage[midiData->originalMessageLength] = midiData_curval1;
			midiData->originalMessageLength += 1;
			midiData_curval1_1 = (midiData_curval1_1 << 7) + (midiData_curval1 & 0x7F);
		}
	}
	midiData->deltaTime = midiData_curval1_1;
	midiData_curval2 = *midiData_offs;
	midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
	if ( (midiData_curval2 & 0x80) )
	{
		midiData_offs += 1;
		midiData->originalMessageLength += 1;
	}
	else
	{
		midiData_curval2 = lastStatus;
	}
	midiData->message[0] = midiData_curval2;
	midiData->lastStatus = midiData_curval2;
	midiData_offs_plusone = 0;
	switch ( midiData_curval2 & 0xF0 )
	{
		case 0x90:
		case 0xB0:
		case 0xE0:
		{
			endflg = 4;
			break;
		}
		case 0x80:
		case 0xC0:
		case 0xD0:
		{
			break;
		}
		case 0xA0:
		{
			if ( !midiData->compMode )
			{
				endflg = 4;
			}
			else
			{
				int someoffsx;

				midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
				midiData->message[1] = midiData->originalMessage[midiData->originalMessageLength];
				midiData->message[2] = 8 * (midiData->originalMessage[midiData->originalMessageLength] & 0xF);
				midiData->originalMessageLength = midiData->originalMessageLength + 1;
				someoffsx = 2 * ((midiData->message[0] & 0xF) | (midiData->message[1] & 0x70));
				midiData->message[1] = midiData->message[1] & 0x7F;
				midiData_offs_plusone = midiData_offs + 1;
				midiData->message[0] = *((u8 *)&midiData->midiData->compBlock[1].compOption + someoffsx);
				midiData->message[1] = *((u8 *)&midiData->midiData->compBlock[1].compOption + someoffsx + 1);
				midiData->messageLength = 3;
				endflg = 1;
			}
			break;
		}
		case 0xF0:
		{
			int i;
			u8 midiData_curval7;
			int msg2ew;

			if ( midiData_curval2 != 255 )
			{
				midiData->readStatus = 2;
				return (int)0x8104002F;
			}
			midiData->messageLength = 1;
			midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
			midiData->message[1] = midiData->originalMessage[midiData->originalMessageLength];
			midiData->messageLength = midiData->messageLength + 1;
			midiData->originalMessageLength = midiData->originalMessageLength + 1;
			someaddoffsone = midiData_offs + 1;
			midiData_curval7 = *someaddoffsone;
			midiData->originalMessage[midiData->originalMessageLength] = *someaddoffsone;
			midiData->message[2] = midiData->originalMessage[midiData->originalMessageLength];
			midiData->messageLength = midiData->messageLength + 1;
			midiData_offs_plusone = someaddoffsone + 1;
			midiData->originalMessageLength += 1;
			msg2ew = midiData_curval7;
			for ( i = 1; msg2ew >= i; i += 1 )
			{
				midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs_plusone;
				midiData_offs_plusone += 1;
				midiData->message[i + 2] = midiData->originalMessage[midiData->originalMessageLength];
				msg2ew = midiData->message[2];
				midiData->messageLength += 1;
				midiData->originalMessageLength += 1;
			}
			niceflag = 1;
			if ( midiData->message[0] == 0xFF && midiData->message[1] == 0x2F && midiData->message[2] == 0x00 )
				midiData->readStatus = 1;
			endflg = 1;
			break;
		}
		default:
		{
			midiData->readStatus = 2;
			return (int)0x8104002F;
		}
	}
	nextMessageLength = 0;
	if ( endflg >= 4 )
	{
		midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
		midiData->message[1] = *midiData_offs;
		midiData->originalMessageLength = midiData->originalMessageLength + 1;
		currofssplusone = midiData_offs + 1;
		midiData->originalMessage[midiData->originalMessageLength] = *currofssplusone;
		midiData_offs_plusone = currofssplusone + 1;
		midiData->message[2] = midiData->originalMessage[midiData->originalMessageLength];
		nextMessageLength = 3;
		endflg = 2;
	}
	if ( endflg >= 3 )
	{
		midiData->originalMessage[midiData->originalMessageLength] = *midiData_offs;
		midiData_offs_plusone = midiData_offs + 1;
		midiData->message[1] = midiData->originalMessage[midiData->originalMessageLength];
		nextMessageLength = 2;
	}
	if ( endflg >= 2 )
	{
		midiData->messageLength = nextMessageLength;
		midiData->originalMessageLength = midiData->originalMessageLength + 1;
	}
	if ( !niceflag )
		midiData->message[midiData->messageLength - 1] &= ~0x80u;
	midiData->nextOffset = (u32)(midiData_offs_plusone - (u8 *)midiData->midiData);
	return 0;
}

int sceSdSqInitSongData(void *addr, u32 songNumber, SceSdSqSongData *songData)
{
	int result;
	struct sdsq_info dinfo;

	result = (int)do_get_vers_sequ_chunk((sceSeqVersionChunk *)addr, &dinfo);
	if ( result )
		return result;
	result = (int)do_get_song_chunk(addr, &dinfo);
	if ( result )
		return result;
	if ( dinfo.m_song->maxSongNumber < songNumber )
		return (int)0x81049027;
	if ( dinfo.m_song->songOffsetAddr[songNumber] == 0xFFFFFFFF )
		return (int)0x81049027;
	songData->readStatus = 0;
	songData->songNumber = songNumber;
	songData->topAddr = (char *)(dinfo.m_song) + dinfo.m_song->songOffsetAddr[songNumber];
	songData->offset = 0;
	songData->nextOffset = 0;
	songData->message[0] = 0;
	songData->message[1] = 0;
	songData->message[2] = 0;
	songData->reserve = 0;
	return 0;
}

int sceSdSqReadSongData(SceSdSqSongData *songData)
{
	const u8 *curOffset;

	if ( songData->readStatus )
		return (int)0x81048001;
	curOffset = (u8 *)songData->topAddr + songData->nextOffset;
	if ( (*curOffset & 0xF0) != 160 || (curOffset[1] & 0x80u) )
	{
		songData->readStatus = 2;
		return (int)0x8104002F;
	}
	songData->offset = songData->nextOffset;
	songData->message[0] = *curOffset;
	songData->message[1] = curOffset[1];
	songData->message[2] = curOffset[2];
	if ( songData->message[0] == 0xA0 && songData->message[1] == 0x7F && songData->message[2] == 0x7F )
		songData->readStatus = 1;
	else
		songData->nextOffset += 3;
	return 0;
}

int sceSdSqGetMaxCompTableIndex(void *addr, u32 midiNumber)
{
	int result;
	sceSeqMidiDataBlock *dblk;

	result = (int)do_get_midi_data_block(addr, midiNumber, &dblk);
	if ( result )
		return result;
	if ( dblk->sequenceDataOffset == 6 )
		return (int)0x81049028;
	return dblk->compBlock[0].compTableSize >> 1;
}

int sceSdSqGetCompTableOffset(void *addr, u32 midiNumber, u32 *offset)
{
	int result;
	sceSeqMidiDataBlock *dblk;

	result = (int)do_get_midi_data_block(addr, midiNumber, &dblk);
	if ( result )
		return result;
	if ( dblk->sequenceDataOffset == 6 )
		return (int)0x81049028;
	*offset = (u32)((char *)dblk - (char *)addr) - 10;
	return 0;
}

int sceSdSqGetCompTableDataByIndex(void *addr, u32 midiNumber, u32 compTableIndex, SceSdSqCompTableData *data)
{
	int result;
	sceSeqMidiDataBlock *dblk;

	result = (int)do_get_midi_data_block(addr, midiNumber, &dblk);
	if ( result )
		return result;
	if ( dblk->sequenceDataOffset == 6 )
		return (int)0x81049028;
	if ( dblk->compBlock[0].compTableSize >> 1 < compTableIndex )
		return (int)0x81049029;
	data->status = *((u8 *)&dblk->compBlock[1].compOption + (compTableIndex << 1));
	data->data = *((u8 *)&dblk->compBlock[1].compOption + (compTableIndex << 1) + 1);
	return 0;
}

int sceSdSqGetNoteOnEventByPolyKeyPress(
	void *addr, u32 midiNumber, const SceSdSqPolyKeyData *pData, SceSdSqCompTableNoteOnEvent *kData)
{
	u32 compTableIndex;
	int result;
	SceSdSqCompTableData data;

	if ( (pData->status & 0xF0) != 160 )
		return (int)0x8104902A;
	compTableIndex = (pData->status & 0xF) + (pData->data & 0xF0);
	if ( compTableIndex >= 0x80 )
		return (int)0x8104902A;
	result = sceSdSqGetCompTableDataByIndex(addr, midiNumber, compTableIndex, &data);
	if ( result )
		return result;
	kData->status = data.status;
	kData->note = data.data;
	kData->velocity = 8 * (pData->data & 0xF);
	return 0;
}

int sceSdSqCopyMidiData(SceSdSqMidiData *to, const SceSdSqMidiData *from)
{
	// The following structure copy was inlined
	*to = *from;
	return 0;
}

int sceSdSqCopySongData(SceSdSqSongData *to, const SceSdSqSongData *from)
{
	// The following structure copy was inlined
	*to = *from;
	return 0;
}

#ifdef _IOP
int _start(int ac, char *av[], void *startaddr, ModuleInfo_t *mi)
{
	int regres;
	int state;

	(void)av;
	(void)startaddr;
	if ( ac < 0 )
	{
		CpuSuspendIntr(&state);
		regres = ReleaseLibraryEntries(&_exp_sdsq);
		CpuResumeIntr(state);
		return (!regres) ? MODULE_NO_RESIDENT_END : MODULE_REMOVABLE_END;
	}
	CpuSuspendIntr(&state);
	regres = RegisterLibraryEntries(&_exp_sdsq);
	CpuResumeIntr(state);
	if ( regres )
		return MODULE_NO_RESIDENT_END;
#if 0
  return MODULE_REMOVABLE_END;
#else
	if ( mi && ((mi->newflags & 2) != 0) )
		mi->newflags |= 0x10;
	return MODULE_RESIDENT_END;
#endif
}
#endif
