/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
*/

#ifndef __AHX_H
#define __AHX_H

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

struct  AHXPListEntry {
	int Note;
	int Fixed;
	int Waveform;
	int FX[2], FXParam[2];
};

struct  AHXPList {
	int Speed, Length;
	struct AHXPListEntry* Entries;
};

struct  AHXEnvelope {
	int aFrames, aVolume;
	int dFrames, dVolume;
	int sFrames;
	int rFrames, rVolume;
};

struct  AHXInstrument {
	char* Name;
	int Volume;			// 0..64
	int WaveLength;		// 0..5 (shifts)
	struct AHXEnvelope Envelope;
	int FilterLowerLimit, FilterUpperLimit, FilterSpeed;
	int SquareLowerLimit, SquareUpperLimit, SquareSpeed;
	int VibratoDelay, VibratoDepth, VibratoSpeed;
	int HardCutRelease, HardCutReleaseFrames;
	struct AHXPList PList;
};

struct  AHXPosition {
	int Track[4], Transpose[4];
};

struct  AHXStep {
	int Note, Instrument, FX, FXParam;
};

struct  AHXSong {
	char* Name;
	int Restart, PositionNr, TrackLength, TrackNr, InstrumentNr, SubsongNr;
	int Revision, SpeedMultiplier;
	int* Subsongs;
};

struct  AHXVoice {
	// Read those variables for mixing!
	int VoiceVolume, VoicePeriod;
	char VoiceBuffer[0x281]; // for oversampling optimization!
	int Track, Transpose;
	int NextTrack, NextTranspose;
	int ADSRVolume; // fixed point 8:8
	struct AHXEnvelope ADSR; // frames/delta fixed 8:8
	struct AHXInstrument* Instrument; // current instrument
	int InstrPeriod, TrackPeriod, VibratoPeriod;
	int NoteMaxVolume, PerfSubVolume, TrackMasterVolume;
	int NewWaveform, Waveform, PlantSquare, PlantPeriod, IgnoreSquare;
	int TrackOn, FixedNote;
	int VolumeSlideUp, VolumeSlideDown;
	int HardCut, HardCutRelease, HardCutReleaseF;
	int PeriodSlideSpeed, PeriodSlidePeriod, PeriodSlideLimit, PeriodSlideOn, PeriodSlideWithLimit;
	int PeriodPerfSlideSpeed, PeriodPerfSlidePeriod, PeriodPerfSlideOn;
	int VibratoDelay, VibratoCurrent, VibratoDepth, VibratoSpeed;
	int SquareOn, SquareInit, SquareWait, SquareLowerLimit, SquareUpperLimit, SquarePos, SquareSign, SquareSlidingIn, SquareReverse;
	int FilterOn, FilterInit, FilterWait, FilterLowerLimit, FilterUpperLimit, FilterPos, FilterSign, FilterSpeed, FilterSlidingIn, IgnoreFilter;
	int PerfCurrent, PerfSpeed, PerfWait;
	int WaveLength;
	struct AHXPList* PerfList;
	int NoteDelayWait, NoteDelayOn, NoteCutWait, NoteCutOn;
	char* AudioSource;
	int AudioPeriod, AudioVolume;
	char SquareTempBuffer[0x80];
};

struct  AHXWaves {
	char LowPasses[0x31588];
	char Triangle04[0x04], Triangle08[0x08], Triangle10[0x10], Triangle20[0x20], Triangle40[0x40], Triangle80[0x80];
	char Sawtooth04[0x04], Sawtooth08[0x08], Sawtooth10[0x10], Sawtooth20[0x20], Sawtooth40[0x40], Sawtooth80[0x80];
	char Squares[0x1000];
	char WhiteNoiseBig[0x780];
	char HighPasses[0x31588];
};

void AHXPlayer_Init();
int  AHXPlayer_LoadSongBuffer(void* Buffer, int Len);
#ifdef WIN32
	int  AHXPlayer_LoadSong(char* Filename);
#endif
int  AHXPlayer_InitSubsong(int Nr);
void AHXPlayer_NextPosition();
void AHXPlayer_PrevPosition();
void AHXPlayer_VoiceOnOff(int Voice, int OnOff);
void AHXPlayer_SetAudio(int v);
void AHXPlayer_PListCommandParse(int v, int FX, int FXParam);
void AHXPlayer_PlayIRQ();
void AHXPlayer_ProcessStep(int v);
void AHXPlayer_ProcessFrame(int v);
void AHXPlayer_SetBoost(int boostval);
void AHXPlayer_SetOversampling(int enable);

void AHXOutput_MixBuffer(short* target);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif //__AHX_H
