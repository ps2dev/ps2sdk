#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include "AHX.h"

// if this is defined the irx will be compiled without PRINTF output
#define COMPACT_CODE

#define malloc(size) \
(AllocSysMemory(0,size,NULL))

#define free(ptr) \
(FreeSysMemory(ptr))

#define Period2Freq(period) (3579545 / (period)) // accurate enough ;)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#define AHXOF_BOOST        0
#define AHXOI_OVERSAMPLING 1

int VibratoTable[] = { 
	0,24,49,74,97,120,141,161,180,197,212,224,235,244,250,253,255,
	253,250,244,235,224,212,197,180,161,141,120,97,74,49,24,
	0,-24,-49,-74,-97,-120,-141,-161,-180,-197,-212,-224,-235,-244,-250,-253,-255,
	-253,-250,-244,-235,-224,-212,-197,-180,-161,-141,-120,-97,-74,-49,-24
};

int PeriodTable[] = {
	0x0000, 0x0D60, 0x0CA0, 0x0BE8, 0x0B40, 0x0A98, 0x0A00, 0x0970,
	0x08E8, 0x0868, 0x07F0, 0x0780, 0x0714, 0x06B0, 0x0650, 0x05F4,
	0x05A0, 0x054C, 0x0500, 0x04B8, 0x0474, 0x0434, 0x03F8, 0x03C0,
	0x038A, 0x0358, 0x0328, 0x02FA, 0x02D0, 0x02A6, 0x0280, 0x025C,
	0x023A, 0x021A, 0x01FC, 0x01E0, 0x01C5, 0x01AC, 0x0194, 0x017D,
	0x0168, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D, 0x00FE, 0x00F0,
	0x00E2, 0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00AA, 0x00A0, 0x0097,
	0x008F, 0x0087, 0x007F, 0x0078, 0x0071
};

int Offsets[] = {0x00,0x04,0x04+0x08,0x04+0x08+0x10,0x04+0x08+0x10+0x20,0x04+0x08+0x10+0x20+0x40};

int* MixingBuffer;
int BlockLen;
int VolumeTable[65][256];
char VoiceBuffer[0x281]; // for oversampling optimization!
char* WaveformTab[4];
struct AHXVoice Voices[4];
struct AHXSong Song;
struct AHXWaves* Waves;
struct AHXPosition* Positions;
struct AHXStep** Tracks;
struct AHXInstrument* Instruments;

short insrument_count=0;
short position_count=0;
short step_count=0;

int pos[4] = { 0, 0, 0, 0 }; // global to retain 'static' properties

// Player vars
int StepWaitFrames, GetNewPosition, SongEndReached, TimingValue;
int PatternBreak;
int MainVolume;
int Playing, Tempo;
int PosNr, PosJump;
int NoteNr, PosJumpNote;	
int WNRandom;
int PlayingTime;

// Output vars
int Oversampling = 0;
int Boost = 0;
#define Bits 16
#define Frequency 48000
#define MixLen 2
#define Hz 50
#define BlockLen 3840
int Paused;

// AHXWaves ////////////////////////////////////////////////////////////////////////////////////////////////

void GenerateTriangle(char* Buffer, int Len)
{
	int d2 = Len;
	int d5 = d2 >> 2;
	int d1 = 128/d5;
	int d4 = -(d2 >> 1);
	char* edi = Buffer;
	char* esi;
	int eax = 0;
    int ecx;
	for(ecx = 0; ecx < d5; ecx++) {
		*edi++ = eax;
		eax += d1;
	}
	*edi++ = 0x7f;
	if(d5 != 1) {
		eax = 128;
		for(ecx = 0; ecx < d5-1; ecx++) {
			eax -= d1;
			*edi++ = eax;
		}
	}
	esi = edi + d4;
	for(ecx = 0; ecx < d5*2; ecx++) {
		*edi++ = *esi++;
		if(edi[-1] == 0x7f) edi[-1] = 0x80;
		               else edi[-1] = -edi[-1];
	}
}

void GenerateSquare(char* Buffer)
{
	char* edi = Buffer;
    int ebx, ecx;
	for(ebx = 1; ebx <= 0x20; ebx++) {
		for(ecx = 0; ecx < (0x40-ebx)*2; ecx++) *edi++ = 0x80;
		for(ecx = 0; ecx <       ebx *2; ecx++) *edi++ = 0x7f;
	}
}

void GenerateSawtooth(char* Buffer, int Len)
{
	char* edi = Buffer;
    int ecx;
	int ebx = 256/(Len-1), eax = -128;
	for(ecx = 0; ecx < Len; ecx++) {
		*edi++ = eax;
		eax += ebx;
	}
}

void GenerateWhiteNoise(char* Buffer, int Len)
{
	unsigned int tmp, tmp2;
	unsigned int Seed = 0x41595321;

	while(Len--) {
		if(Seed&0x100) *Buffer = (char)(Seed&0xff);
		else if(!(Seed&0xffff)) *Buffer = 0x80;
		else *Buffer = 0x7f;
		
		Buffer++;
		tmp = Seed;		// ror 5
		Seed >>= 5;
		tmp <<= 31-5;
		Seed |= tmp;
		Seed = Seed ^ 0x9a;
		tmp2 = tmp = Seed;		// rol 2
		Seed <<= 2;
		tmp >>= 31-2;
		Seed |= tmp;
		Seed += tmp2;
		tmp = Seed ^ tmp2;		// ror 5
		Seed >>= 3;
		tmp <<= 31-3;
		Seed |= tmp;
	}
}

// AHXVoice ////////////////////////////////////////////////////////////////////////////////////////////////

void AHXVoice_Init(struct AHXVoice* pVoice)
{
	memset(pVoice, 0, sizeof(struct AHXVoice));
	memset(pVoice->VoiceBuffer, 0, 0x281);
	pVoice->TrackOn = 1;
	pVoice->TrackMasterVolume = 0x40;
}

void AHXVoice_CalcADSR(struct AHXVoice* pVoice)
{
	pVoice->ADSR.aFrames = pVoice->Instrument->Envelope.aFrames;
	pVoice->ADSR.aVolume = pVoice->Instrument->Envelope.aVolume*256/pVoice->ADSR.aFrames;
	pVoice->ADSR.dFrames = pVoice->Instrument->Envelope.dFrames;
	pVoice->ADSR.dVolume = (pVoice->Instrument->Envelope.dVolume-pVoice->Instrument->Envelope.aVolume)*256/pVoice->ADSR.dFrames;
	pVoice->ADSR.sFrames = pVoice->Instrument->Envelope.sFrames;
	pVoice->ADSR.rFrames = pVoice->Instrument->Envelope.rFrames;
	pVoice->ADSR.rVolume = (pVoice->Instrument->Envelope.rVolume-pVoice->Instrument->Envelope.dVolume)*256/pVoice->ADSR.rFrames;
}

// AHXPlayer ///////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
    #pragma warning(disable:4309 4305)
#endif

#define FIXEDPOINT 16 // 16 = 32-FIXEDPOINT.FIXEDPOINT

void clipint(int* x)
{
	if(*x > 127<<FIXEDPOINT) { *x = 127<<FIXEDPOINT; return; }
	if(*x < -128<<FIXEDPOINT) { *x = -128<<FIXEDPOINT; return; }
}

void GenerateFilterWaveforms(char* Buffer, char* Low, char* High)
{
	char *a0;
	int LengthTable[] = { 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 3, 7, 0xf, 0x1f, 0x3f, 0x7f,
		0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,
		0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,
		(0x280*3)-1
		};
    int temp, freq, waves, i, freint, highint, midint, lowint;

    for(temp = 0, freq = 8; temp < 31; temp++, freq += 3 ) 
	{
		int v125;
		a0 = Buffer;
		//fre = ((freq*1.25f)/100);
		v125 = (1<<FIXEDPOINT)+((1<<FIXEDPOINT)/4);
		freint = ((freq*v125)/100); // 0x14000 ~ 1.25 
		for(waves = 0; waves < 6+6+0x20+1; waves++) 
		{ // 0 - 44
			char low_,high_;
			//mid = 0.f;
				//low = 0.f;
			midint = 0;
			lowint = 0;

			for(i = 0; i <= LengthTable[waves]; i++) 
			{
				//high = a0[i] - mid - low; clip(&high);
				//mid += high*fre; clip(&mid);
				//low += mid*fre; clip(&low);

				highint = (((int)(a0[i]))<<FIXEDPOINT) - midint - lowint; clipint(&highint);
				midint += (highint*(freint>>(FIXEDPOINT/2))>>(FIXEDPOINT/2)); clipint(&midint);
				lowint += (midint*(freint>>(FIXEDPOINT/2))>>(FIXEDPOINT/2)); clipint(&lowint);
			}
			for(i = 0; i <= LengthTable[waves]; i++) 
			{
				//high = a0[i] - mid - low; clip(&high);
				//mid += high*fre; clip(&mid);
				//low += mid*fre; clip(&low);
				//*Low++ = (char)low;
				//*High++ = (char)high;

				highint = (((int)(a0[i]))<<FIXEDPOINT) - midint - lowint; clipint(&highint);
				midint += (highint*(freint>>(FIXEDPOINT/2))>>(FIXEDPOINT/2)); clipint(&midint);
				lowint += (midint*(freint>>(FIXEDPOINT/2))>>(FIXEDPOINT/2)); clipint(&lowint);

				low_ = (lowint>>FIXEDPOINT);
				high_ = (highint>>FIXEDPOINT);

				*Low++ = low_;
				*High++ = high_;
			}
			a0 += LengthTable[waves]+1;
			//printf("start = %d, a0 = %d\n", start, a0); 
		} 
	}
}

int AHXOutput_SetOption(int Option, int Value)
{
    int i, j;
	switch(Option) {
		case AHXOF_BOOST: {
			// Initialize volume table
			for (i = 0; i < 65; i++)
				for (j = -128; j < 128; j++)
					VolumeTable[i][j+128] = (int)(i * j * Value) / 64;
			Boost = Value;
		} return 1;
		default: return 0;
	}
}

void AHXPlayer_Init()
{
	Waves = (struct AHXWaves*)malloc(sizeof(struct AHXWaves));
	WaveformTab[0] = Waves->Triangle04;
	WaveformTab[1] = Waves->Sawtooth04;
	WaveformTab[3] = Waves->WhiteNoiseBig;

	GenerateSawtooth(Waves->Sawtooth04, 0x04);
	GenerateSawtooth(Waves->Sawtooth08, 0x08);
	GenerateSawtooth(Waves->Sawtooth10, 0x10);
	GenerateSawtooth(Waves->Sawtooth20, 0x20);
	GenerateSawtooth(Waves->Sawtooth40, 0x40);
	GenerateSawtooth(Waves->Sawtooth80, 0x80);
	GenerateTriangle(Waves->Triangle04, 0x04);
	GenerateTriangle(Waves->Triangle08, 0x08);
	GenerateTriangle(Waves->Triangle10, 0x10);
	GenerateTriangle(Waves->Triangle20, 0x20);
	GenerateTriangle(Waves->Triangle40, 0x40);
	GenerateTriangle(Waves->Triangle80, 0x80);
	GenerateSquare(Waves->Squares);
	GenerateWhiteNoise(Waves->WhiteNoiseBig, 0x280*3);
	GenerateFilterWaveforms(Waves->Triangle04, Waves->LowPasses, Waves->HighPasses);

	MixingBuffer = (int*) malloc(sizeof(int)*(MixLen*Frequency/Hz));

	AHXOutput_SetOption(AHXOF_BOOST, Boost+1);
}

void AHXPlayer_FreeMem()
{
	int i;
	// free old mem in reverse order
	//printf("1 Memory Allocated. %d bytes left.\n",QueryTotalFreeMemSize());
	for(i = 1; i < Song.InstrumentNr+1; i++) {
		free(Instruments[i].PList.Entries);
		free(Instruments[i].Name);
	}
	free(Instruments);

	for(i = 0; i < Song.TrackNr+1; i++) {
		free(Tracks[i]);
	}
	free(Tracks);
	free(Positions);
	free(Song.Subsongs);
	free(Song.Name);
	//printf("2 Memory Allocated. %d bytes left.\n",QueryTotalFreeMemSize());
}

int AHXPlayer_LoadSongBuffer(void* Buffer, int Len)
{
	int SongLength = Len;
    int i, j, MaxTrack;
	unsigned char* SongBuffer = (unsigned char*)Buffer;
	unsigned char* SBPtr = &SongBuffer[14];
    char* NamePtr;

	AHXPlayer_FreeMem();
    
	if(SongLength < 14 || SongLength == 65536) return 0;

	if(SongBuffer[0] != 'T' && SongBuffer[1] != 'H' && SongBuffer[2] != 'X') return 0;
	Song.Revision = SongBuffer[3];
	if(Song.Revision > 1) return 0;

	// Header ////////////////////////////////////////////
	// Songname
	NamePtr = (char*)&SongBuffer[(SongBuffer[4]<<8) | SongBuffer[5]];
	Song.Name =  (char*) malloc(strlen(NamePtr)+1);
	strcpy(Song.Name, NamePtr); NamePtr += strlen(NamePtr)+1;
	Song.SpeedMultiplier = ((SongBuffer[6]>>5)&3)+1;

	Song.PositionNr = ((SongBuffer[6]&0xf)<<8) | SongBuffer[7];
	Song.Restart = (SongBuffer[8]<<8) | SongBuffer[9];
	Song.TrackLength = SongBuffer[10];
	Song.TrackNr = SongBuffer[11];
	Song.InstrumentNr = SongBuffer[12];
	Song.SubsongNr = SongBuffer[13];

	// Subsongs //////////////////////////////////////////
	Song.Subsongs = (int*) malloc(sizeof(int)*Song.SubsongNr);
	#ifndef COMPACT_CODE
		printf("name length = %d\n",strlen(NamePtr)+1);
		printf("playing: %s\n", NamePtr);
		printf("subsongs = %d\n",Song.SubsongNr);
		printf("Positions = %d\n",Song.PositionNr);
		printf("Tracks = %d\n",(MaxTrack+1));
	#endif	
	for(i = 0; i < Song.SubsongNr; i++) {
		if(SBPtr - SongBuffer > SongLength) return 0;
		Song.Subsongs[i] = (SBPtr[0]<<8)|SBPtr[1];
		SBPtr += 2;
	}

	// Position List /////////////////////////////////////
	Positions = (struct AHXPosition*) malloc(sizeof(struct AHXPosition)*Song.PositionNr);	
	for(i = 0; i < Song.PositionNr; i++) {
		for(j = 0; j < 4; j++) {
			if(SBPtr - SongBuffer > SongLength) return 0;
			Positions[i].Track[j] = *SBPtr++;
			Positions[i].Transpose[j] = *(signed char*)SBPtr++;
		}
	}

	// Tracks ////////////////////////////////////////////
	MaxTrack = Song.TrackNr;
	Tracks = (struct AHXStep**) malloc(sizeof(struct AHXStep)*(MaxTrack+1));	
	for(i = 0; i < MaxTrack+1; i++) {
		Tracks[i] = (struct AHXStep*) malloc(sizeof(struct AHXStep)*Song.TrackLength);
		if((SongBuffer[6]&0x80)==0x80 && i==0) {
			memset(Tracks[i], 0, Song.TrackLength*sizeof(struct AHXStep));
			continue;
		}
		for(j = 0; j < Song.TrackLength; j++) {
			if(SBPtr - SongBuffer > SongLength) return 0;
			Tracks[i][j].Note = (SBPtr[0]>>2)&0x3f;
			Tracks[i][j].Instrument = ((SBPtr[0]&0x3)<<4) | (SBPtr[1]>>4);
			Tracks[i][j].FX = SBPtr[1]&0xf;
			Tracks[i][j].FXParam = SBPtr[2];
			SBPtr += 3;
		}
	}

	// Instruments ///////////////////////////////////////
	Instruments = (struct AHXInstrument*) malloc(sizeof(struct AHXInstrument)*(Song.InstrumentNr+1));
	for(i = 1; i < Song.InstrumentNr+1; i++) {
		Instruments[i].Name = (char*) malloc(strlen(NamePtr)+1);
		strcpy(Instruments[i].Name, NamePtr); NamePtr += strlen(NamePtr)+1;
		if(SBPtr - SongBuffer > SongLength) return 0;
		Instruments[i].Volume = SBPtr[0];
		Instruments[i].FilterSpeed = ((SBPtr[1]>>3)&0x1f) | ((SBPtr[12]>>2)&0x20);
		Instruments[i].WaveLength = SBPtr[1]&0x7;
		Instruments[i].Envelope.aFrames = SBPtr[2];
		Instruments[i].Envelope.aVolume = SBPtr[3];
		Instruments[i].Envelope.dFrames = SBPtr[4]; //4
		Instruments[i].Envelope.dVolume = SBPtr[5];
		Instruments[i].Envelope.sFrames = SBPtr[6];
		Instruments[i].Envelope.rFrames = SBPtr[7]; //7
		Instruments[i].Envelope.rVolume = SBPtr[8];
		Instruments[i].FilterLowerLimit = SBPtr[12]&0x7f;
		Instruments[i].VibratoDelay = SBPtr[13]; //13
		Instruments[i].HardCutReleaseFrames = (SBPtr[14]>>4)&7;
		Instruments[i].HardCutRelease = SBPtr[14]&0x80?1:0;
		Instruments[i].VibratoDepth = SBPtr[14]&0xf; //14
		Instruments[i].VibratoSpeed = SBPtr[15];
		Instruments[i].SquareLowerLimit = SBPtr[16];
		Instruments[i].SquareUpperLimit = SBPtr[17]; //17
		Instruments[i].SquareSpeed = SBPtr[18];
		Instruments[i].FilterUpperLimit = SBPtr[19]&0x3f; //19
		Instruments[i].PList.Speed = SBPtr[20];
		Instruments[i].PList.Length= SBPtr[21];
		SBPtr += 22;
		Instruments[i].PList.Entries = (struct AHXPListEntry*) malloc(sizeof(struct AHXPListEntry)*Instruments[i].PList.Length);
		for(j = 0; j < Instruments[i].PList.Length; j++) {
			if(SBPtr - SongBuffer > SongLength) return 0;
			Instruments[i].PList.Entries[j].FX[1] = (SBPtr[0]>>5)&7;
			Instruments[i].PList.Entries[j].FX[0] = (SBPtr[0]>>2)&7;
			Instruments[i].PList.Entries[j].Waveform = ((SBPtr[0]<<1)&6) | (SBPtr[1]>>7);
			Instruments[i].PList.Entries[j].Fixed = (SBPtr[1]>>6)&1;
			Instruments[i].PList.Entries[j].Note = SBPtr[1]&0x3f;
			Instruments[i].PList.Entries[j].FXParam[0] = SBPtr[2];
			Instruments[i].PList.Entries[j].FXParam[1] = SBPtr[3];
			SBPtr += 4;
		}
	}
	return Song.SubsongNr;
}

#ifdef WIN32
    int AHXPlayer_LoadSong(char* Filename)
    {
        int SongLength;
        FILE* f;
	    unsigned char SongBuffer[65536];
	    f = fopen(Filename, "rb"); if(!f) return 0;
	    SongLength = fread(SongBuffer, 1, 65536, f);
	    fclose(f);
	    return AHXPlayer_LoadSongBuffer(SongBuffer, SongLength);
    }
#endif

int AHXPlayer_InitSubsong(int Nr)
{
    int v;

	if(Nr > Song.SubsongNr) return 0;

	if(Nr == 0) PosNr = 0;
	       else PosNr = Song.Subsongs[Nr-1];

	PosJump = 0;
	PatternBreak = 0;
	MainVolume = 0x40;
	Playing = 1;
	NoteNr = PosJumpNote = 0;
	Tempo = 6;
	StepWaitFrames = 0;
	GetNewPosition = 1;
	SongEndReached = 0;
	TimingValue = PlayingTime = 0;

	for(v = 0; v < 4; v++) 
    {
        AHXVoice_Init(&Voices[v]);
    }

	return 1;
}

void AHXPlayer_NextPosition()
{
	PosNr++;
	if(PosNr == Song.PositionNr) PosNr = 0;
	StepWaitFrames = 0;
	GetNewPosition = 1;
}

void AHXPlayer_PrevPosition()
{
	PosNr--;
	if(PosNr < 0) PosNr = 0;
	StepWaitFrames = 0;
	GetNewPosition = 1;
}

void AHXPlayer_ProcessStep(int v)
{
	int Note, Instrument, FX, FXParam, SquareLower, SquareUpper, d3, d4, d6, Neue, Alte, i;

	if(!Voices[v].TrackOn) return;
	Voices[v].VolumeSlideUp = Voices[v].VolumeSlideDown = 0;

	Note = Tracks[Positions[PosNr].Track[v]][NoteNr].Note;
	Instrument = Tracks[Positions[PosNr].Track[v]][NoteNr].Instrument;
	FX = Tracks[Positions[PosNr].Track[v]][NoteNr].FX;
	FXParam = Tracks[Positions[PosNr].Track[v]][NoteNr].FXParam;

	switch(FX) {
		case 0x0: // Position Jump HI
			if((FXParam & 0xf) > 0 && (FXParam & 0xf) <= 9)
				PosJump = FXParam & 0xf;
			break;
		case 0x5: // Volume Slide + Tone Portamento
		case 0xa: // Volume Slide
			Voices[v].VolumeSlideDown = FXParam & 0x0f;
			Voices[v].VolumeSlideUp   = FXParam >> 4;
			break;
		case 0xb: // Position Jump
			PosJump = PosJump*100 + (FXParam & 0x0f) + (FXParam >> 4)*10;
			PatternBreak = 1;
			break;
		case 0xd: // Patternbreak
			PosJump = PosNr + 1;
			PosJumpNote = (FXParam & 0x0f) + (FXParam >> 4)*10;
			if(PosJumpNote > Song.TrackLength) PosJumpNote = 0;
			PatternBreak = 1;
			break;
		case 0xe: // Enhanced commands
			switch(FXParam >> 4) {
				case 0xc: // Note Cut
					if((FXParam & 0x0f) < Tempo) {
						Voices[v].NoteCutWait = FXParam & 0x0f;
						if(Voices[v].NoteCutWait) {
							Voices[v].NoteCutOn = 1;
							Voices[v].HardCutRelease = 0;
						}
					}
					break;
				case 0xd: // Note Delay
					if(Voices[v].NoteDelayOn) {
						Voices[v].NoteDelayOn = 0;
					} else {
						if((FXParam & 0x0f) < Tempo) {
							Voices[v].NoteDelayWait = FXParam & 0x0f;
							if(Voices[v].NoteDelayWait) {
								Voices[v].NoteDelayOn = 1;
								return;
							}
						}
					}
					break;
			}
			break;
		case 0xf: // Speed
			Tempo = FXParam;
			break;
	}
	if(Instrument) {
		Voices[v].PerfSubVolume = 0x40;
		Voices[v].PeriodSlideSpeed = Voices[v].PeriodSlidePeriod = Voices[v].PeriodSlideLimit = 0;
		Voices[v].ADSRVolume = 0;
		Voices[v].Instrument = &Instruments[Instrument];
		AHXVoice_CalcADSR(&Voices[v]);
		//InitOnInstrument
		Voices[v].WaveLength = Voices[v].Instrument->WaveLength;
		Voices[v].NoteMaxVolume = Voices[v].Instrument->Volume;
		//InitVibrato
		Voices[v].VibratoCurrent = 0;
		Voices[v].VibratoDelay = Voices[v].Instrument->VibratoDelay;
		Voices[v].VibratoDepth = Voices[v].Instrument->VibratoDepth;
		Voices[v].VibratoSpeed = Voices[v].Instrument->VibratoSpeed;
		Voices[v].VibratoPeriod = 0;
		//InitHardCut
		Voices[v].HardCutRelease = Voices[v].Instrument->HardCutRelease;
		Voices[v].HardCut = Voices[v].Instrument->HardCutReleaseFrames;
		//InitSquare
		Voices[v].IgnoreSquare = Voices[v].SquareSlidingIn = 0;
		Voices[v].SquareWait = Voices[v].SquareOn = 0;
		SquareLower = Voices[v].Instrument->SquareLowerLimit >> (5-Voices[v].WaveLength);
		SquareUpper = Voices[v].Instrument->SquareUpperLimit >> (5-Voices[v].WaveLength);
		if(SquareUpper < SquareLower) { int t = SquareUpper; SquareUpper = SquareLower; SquareLower = t; }
		Voices[v].SquareUpperLimit = SquareUpper;
		Voices[v].SquareLowerLimit = SquareLower;
		//InitFilter
		Voices[v].IgnoreFilter = Voices[v].FilterWait = Voices[v].FilterOn = 0;
		Voices[v].FilterSlidingIn = 0;
		d6 = Voices[v].Instrument->FilterSpeed;
		d3 = Voices[v].Instrument->FilterLowerLimit;
		d4 = Voices[v].Instrument->FilterUpperLimit;
		if(d3 & 0x80) d6 |= 0x20;
		if(d4 & 0x80) d6 |= 0x40;
		Voices[v].FilterSpeed = d6;
		d3 &= ~0x80;
		d4 &= ~0x80;
		if(d3 > d4) { int t = d3; d3 = d4; d4 = t; }
		Voices[v].FilterUpperLimit = d4;
		Voices[v].FilterLowerLimit = d3;
		Voices[v].FilterPos = 32;
		//Init PerfList
		Voices[v].PerfWait  =  Voices[v].PerfCurrent = 0;
		Voices[v].PerfSpeed =  Voices[v].Instrument->PList.Speed;
		Voices[v].PerfList  = &Voices[v].Instrument->PList;
	}
	//NoInstrument
	Voices[v].PeriodSlideOn = 0;

	switch(FX) {
		case 0x4: // Override filter
			break;
		case 0x9: // Set Squarewave-Offset
			Voices[v].SquarePos = FXParam >> (5 - Voices[v].WaveLength);
			Voices[v].PlantSquare = 1;
			Voices[v].IgnoreSquare = 1;
			break;
		case 0x5: // Tone Portamento + Volume Slide
		case 0x3: // Tone Portamento (Period Slide Up/Down w/ Limit)
			if(FXParam != 0) Voices[v].PeriodSlideSpeed = FXParam;
			if(Note) {
				Neue = PeriodTable[Note];
				Alte = PeriodTable[Voices[v].TrackPeriod];
				Alte -= Neue;
				Neue = Alte + Voices[v].PeriodSlidePeriod;
				if(Neue) Voices[v].PeriodSlideLimit = -Alte;
			}
			Voices[v].PeriodSlideOn = 1;
			Voices[v].PeriodSlideWithLimit = 1;
			goto NoNote;
	}

	// Note anschlagen
	if(Note) {
		Voices[v].TrackPeriod = Note;
		Voices[v].PlantPeriod = 1;
	}
NoNote:
	switch(FX) {
		case 0x1: // Portamento up (Period slide down)
			Voices[v].PeriodSlideSpeed = -FXParam;
			Voices[v].PeriodSlideOn = 1;
			Voices[v].PeriodSlideWithLimit = 0;
			break;
		case 0x2: // Portamento down (Period slide up)
			Voices[v].PeriodSlideSpeed = FXParam;
			Voices[v].PeriodSlideOn = 1;
			Voices[v].PeriodSlideWithLimit = 0;
			break;
		case 0xc: // Volume
			if(FXParam <= 0x40) 
				Voices[v].NoteMaxVolume = FXParam;
			else {
				FXParam -= 0x50;
				if(FXParam <= 0x40)
					for(i = 0; i < 4; i++) Voices[i].TrackMasterVolume = FXParam;
				else {
					FXParam -= 0xa0 - 0x50;
					if(FXParam <= 0x40)
						Voices[v].TrackMasterVolume = FXParam;
				}
			}
			break;
		case 0xe: // Enhanced commands
			switch(FXParam >> 4) {
				case 0x1: // Fineslide up (Period fineslide down)
					Voices[v].PeriodSlidePeriod = -(FXParam & 0x0f);
					Voices[v].PlantPeriod = 1;
					break;
				case 0x2: // Fineslide down (Period fineslide up)
					Voices[v].PeriodSlidePeriod = FXParam & 0x0f;
					Voices[v].PlantPeriod = 1;
					break;
				case 0x4: // Vibrato control
					Voices[v].VibratoDepth = FXParam & 0x0f;
					break;
				case 0xa: // Finevolume up
					Voices[v].NoteMaxVolume += FXParam & 0x0f;
					if(Voices[v].NoteMaxVolume > 0x40) Voices[v].NoteMaxVolume = 0x40;
					break;
				case 0xb: // Finevolume down
					Voices[v].NoteMaxVolume -= FXParam & 0x0f;
					if(Voices[v].NoteMaxVolume < 0) Voices[v].NoteMaxVolume = 0;
					break;
			}
			break;
	}
}

void AHXPlayer_PListCommandParse(int v, int FX, int FXParam)
{
	switch(FX) {
		case 0: 
			if(Song.Revision > 0 && FXParam != 0) {
				if(Voices[v].IgnoreFilter) {
					Voices[v].FilterPos = Voices[v].IgnoreFilter;
					Voices[v].IgnoreFilter = 0;
				} else Voices[v].FilterPos = FXParam;
				Voices[v].NewWaveform = 1;
			}
			break;
		case 1:
			Voices[v].PeriodPerfSlideSpeed = FXParam;
			Voices[v].PeriodPerfSlideOn = 1;
			break;
		case 2:
			Voices[v].PeriodPerfSlideSpeed = -FXParam;
			Voices[v].PeriodPerfSlideOn = 1;
			break;
		case 3: // Init Square Modulation
			if(!Voices[v].IgnoreSquare) {
				Voices[v].SquarePos = FXParam >> (5-Voices[v].WaveLength);
			} else Voices[v].IgnoreSquare = 0;
			break;
		case 4: // Start/Stop Modulation
			if(Song.Revision == 0 || FXParam == 0) {
				Voices[v].SquareInit = (Voices[v].SquareOn ^= 1);
				Voices[v].SquareSign = 1;
			} else {
				if(FXParam & 0x0f) {
					Voices[v].SquareInit = (Voices[v].SquareOn ^= 1);
					Voices[v].SquareSign = 1;
					if((FXParam & 0x0f) == 0x0f) Voices[v].SquareSign = -1;
				}
				if(FXParam & 0xf0) {
					Voices[v].FilterInit = (Voices[v].FilterOn ^= 1);
					Voices[v].FilterSign = 1;
					if((FXParam & 0xf0) == 0xf0) Voices[v].FilterSign = -1;
				}
			}
			break;
		case 5: // Jump to Step [xx]
			Voices[v].PerfCurrent = FXParam;
			break;
		case 6: // Set Volume
			if(FXParam > 0x40) {
				if((FXParam -= 0x50) >= 0) {
					if(FXParam <= 0x40) Voices[v].PerfSubVolume = FXParam;
					else 
						if((FXParam -= 0xa0-0x50) >= 0) 
							if(FXParam <= 0x40) Voices[v].TrackMasterVolume = FXParam;
				}
			} else Voices[v].NoteMaxVolume = FXParam;
			break;
		case 7: // set speed
			Voices[v].PerfSpeed = Voices[v].PerfWait = FXParam;
			break;
	}
}

void AHXPlayer_ProcessFrame(int v)
{
    int d0, d1, d2, d3, Cur, FMax, i, X, Delta;
    char* SquarePtr, *AudioSource;

	if(!Voices[v].TrackOn) return;

	if(Voices[v].NoteDelayOn) {
		if(Voices[v].NoteDelayWait <= 0) AHXPlayer_ProcessStep(v);
		                            else Voices[v].NoteDelayWait--;
	}
	if(Voices[v].HardCut) {
		int NextInstrument;
		if(NoteNr+1 < Song.TrackLength) NextInstrument = Tracks[Voices[v].Track][NoteNr+1].Instrument;
						           else NextInstrument = Tracks[Voices[v].NextTrack][0].Instrument;
		if(NextInstrument) {
			d1 = Tempo - Voices[v].HardCut;
			if(d1 < 0) d1 = 0;
			if(!Voices[v].NoteCutOn) {
				Voices[v].NoteCutOn = 1;
				Voices[v].NoteCutWait = d1;
				Voices[v].HardCutReleaseF = -(d1 - Tempo);
			} else Voices[v].HardCut = 0;
		}
	}
	if(Voices[v].NoteCutOn) {
		if(Voices[v].NoteCutWait <= 0) {
			Voices[v].NoteCutOn = 0;
			if(Voices[v].HardCutRelease) {
				Voices[v].ADSR.rVolume = -(Voices[v].ADSRVolume - (Voices[v].Instrument->Envelope.rVolume << 8))/Voices[v].HardCutReleaseF;
				Voices[v].ADSR.rFrames = Voices[v].HardCutReleaseF;
				Voices[v].ADSR.aFrames = Voices[v].ADSR.dFrames = Voices[v].ADSR.sFrames = 0;
			} else Voices[v].NoteMaxVolume = 0;
		} else Voices[v].NoteCutWait--;
	}
	//adsrEnvelope
	if(Voices[v].ADSR.aFrames) {
		Voices[v].ADSRVolume += Voices[v].ADSR.aVolume; // Delta
		if(--Voices[v].ADSR.aFrames <= 0) Voices[v].ADSRVolume = Voices[v].Instrument->Envelope.aVolume << 8;
	} else if(Voices[v].ADSR.dFrames) {
		Voices[v].ADSRVolume += Voices[v].ADSR.dVolume; // Delta
		if(--Voices[v].ADSR.dFrames <= 0) Voices[v].ADSRVolume = Voices[v].Instrument->Envelope.dVolume << 8;
	} else if(Voices[v].ADSR.sFrames) {
		Voices[v].ADSR.sFrames--;
	} else if(Voices[v].ADSR.rFrames) {
		Voices[v].ADSRVolume += Voices[v].ADSR.rVolume; // Delta
		if(--Voices[v].ADSR.rFrames <= 0) Voices[v].ADSRVolume = Voices[v].Instrument->Envelope.rVolume << 8;
	}
	//VolumeSlide
	Voices[v].NoteMaxVolume = Voices[v].NoteMaxVolume + Voices[v].VolumeSlideUp - Voices[v].VolumeSlideDown;
	if(Voices[v].NoteMaxVolume < 0) Voices[v].NoteMaxVolume = 0;
	if(Voices[v].NoteMaxVolume > 0x40) Voices[v].NoteMaxVolume = 0x40;
	//Portamento
	if(Voices[v].PeriodSlideOn) {
		if(Voices[v].PeriodSlideWithLimit) {
			d0 = Voices[v].PeriodSlidePeriod - Voices[v].PeriodSlideLimit;
			d2 = Voices[v].PeriodSlideSpeed;
			if(d0 > 0) d2 = -d2;
			if(d0) {
				d3 = (d0 + d2) ^ d0;
				if(d3 >= 0) d0 = Voices[v].PeriodSlidePeriod + d2;
				       else d0 = Voices[v].PeriodSlideLimit;
				Voices[v].PeriodSlidePeriod = d0;
				Voices[v].PlantPeriod = 1;
			}
		} else {
			Voices[v].PeriodSlidePeriod += Voices[v].PeriodSlideSpeed;
			Voices[v].PlantPeriod = 1;
		}
	}
	//Vibrato
	if(Voices[v].VibratoDepth) {
		if(Voices[v].VibratoDelay <= 0) {
			Voices[v].VibratoPeriod = (VibratoTable[Voices[v].VibratoCurrent] * Voices[v].VibratoDepth) >> 7;
			Voices[v].PlantPeriod = 1;
			Voices[v].VibratoCurrent = (Voices[v].VibratoCurrent + Voices[v].VibratoSpeed) & 0x3f;
		} else Voices[v].VibratoDelay--;
	}
	//PList
	if(Voices[v].Instrument && Voices[v].PerfCurrent < Voices[v].Instrument->PList.Length) {
		if(--Voices[v].PerfWait <= 0) {
			Cur = Voices[v].PerfCurrent++;
			Voices[v].PerfWait = Voices[v].PerfSpeed;
			if(Voices[v].PerfList->Entries[Cur].Waveform) {
				Voices[v].Waveform = Voices[v].PerfList->Entries[Cur].Waveform-1;
				Voices[v].NewWaveform = 1;
				Voices[v].PeriodPerfSlideSpeed = Voices[v].PeriodPerfSlidePeriod = 0;
			}
			//Holdwave
			Voices[v].PeriodPerfSlideOn = 0;
			for(i = 0; i < 2; i++) AHXPlayer_PListCommandParse(v, Voices[v].PerfList->Entries[Cur].FX[i], Voices[v].PerfList->Entries[Cur].FXParam[i]);
			//GetNote
			if(Voices[v].PerfList->Entries[Cur].Note) {
				Voices[v].InstrPeriod = Voices[v].PerfList->Entries[Cur].Note;
				Voices[v].PlantPeriod = 1;
				Voices[v].FixedNote = Voices[v].PerfList->Entries[Cur].Fixed;
			}
		}
	} else {
		if(Voices[v].PerfWait) Voices[v].PerfWait--;
		                  else Voices[v].PeriodPerfSlideSpeed = 0;
	}
	//PerfPortamento
	if(Voices[v].PeriodPerfSlideOn) {
		Voices[v].PeriodPerfSlidePeriod -= Voices[v].PeriodPerfSlideSpeed;
		if(Voices[v].PeriodPerfSlidePeriod) Voices[v].PlantPeriod = 1;
	}
	if(Voices[v].Waveform == 3-1 && Voices[v].SquareOn) {
		if(--Voices[v].SquareWait <= 0) {
			int d1 = Voices[v].SquareLowerLimit;
			int	d2 = Voices[v].SquareUpperLimit;
			int d3 = Voices[v].SquarePos;
			if(Voices[v].SquareInit) {
				Voices[v].SquareInit = 0;
				if(d3 <= d1) { 
					Voices[v].SquareSlidingIn = 1;
					Voices[v].SquareSign = 1;
				} else if(d3 >= d2) {
					Voices[v].SquareSlidingIn = 1;
					Voices[v].SquareSign = -1;
				}
			}
			//NoSquareInit
			if(d1 == d3 || d2 == d3) {
				if(Voices[v].SquareSlidingIn) {
					Voices[v].SquareSlidingIn = 0;
				} else {
					Voices[v].SquareSign = -Voices[v].SquareSign;
				}
			}
			d3 += Voices[v].SquareSign;
			Voices[v].SquarePos = d3;
			Voices[v].PlantSquare = 1;
			Voices[v].SquareWait = Voices[v].Instrument->SquareSpeed;
		}
	}
	if(Voices[v].FilterOn && --Voices[v].FilterWait <= 0) {
		d1 = Voices[v].FilterLowerLimit;
		d2 = Voices[v].FilterUpperLimit;
		d3 = Voices[v].FilterPos;
		if(Voices[v].FilterInit) {
			Voices[v].FilterInit = 0;
			if(d3 <= d1) {
				Voices[v].FilterSlidingIn = 1;
				Voices[v].FilterSign = 1;
			} else if(d3 >= d2) {
				Voices[v].FilterSlidingIn = 1;
				Voices[v].FilterSign = -1;
			}
		}
		//NoFilterInit
		FMax = (Voices[v].FilterSpeed < 3)?(5-Voices[v].FilterSpeed):1;
		for(i = 0; i < FMax; i++) {
			if(d1 == d3 || d2 == d3) {
				if(Voices[v].FilterSlidingIn) {
					Voices[v].FilterSlidingIn = 0;
				} else {
					Voices[v].FilterSign = -Voices[v].FilterSign;
				}
			}
			d3 += Voices[v].FilterSign;
		}
		Voices[v].FilterPos = d3;
		Voices[v].NewWaveform = 1;
		Voices[v].FilterWait = Voices[v].FilterSpeed - 3;
		if(Voices[v].FilterWait < 1) Voices[v].FilterWait = 1;
	}
	if(Voices[v].Waveform == 3-1 || Voices[v].PlantSquare) {
		//CalcSquare
		SquarePtr = &Waves->Squares[(Voices[v].FilterPos-0x20)*(0xfc+0xfc+0x80*0x1f+0x80+0x280*3)];
		X = Voices[v].SquarePos << (5 - Voices[v].WaveLength);
		if(X > 0x20) {
			X = 0x40 - X;
			Voices[v].SquareReverse = 1;
		}
		//OkDownSquare
		if(--X) SquarePtr += X << 7;
		Delta = 32 >> Voices[v].WaveLength;
		WaveformTab[2] = Voices[v].SquareTempBuffer;
		for(i = 0; i < (1 << Voices[v].WaveLength)*4; i++) {
			Voices[v].SquareTempBuffer[i] = *SquarePtr;
			SquarePtr += Delta;
		}
		Voices[v].NewWaveform = 1;
		Voices[v].Waveform = 3-1;
		Voices[v].PlantSquare = 0;
	}
	if(Voices[v].Waveform == 4-1) Voices[v].NewWaveform = 1;

	if(Voices[v].NewWaveform) {
		AudioSource = WaveformTab[Voices[v].Waveform];
		if(Voices[v].Waveform != 3-1) {
			AudioSource += (Voices[v].FilterPos-0x20)*(0xfc+0xfc+0x80*0x1f+0x80+0x280*3);
		}
		if(Voices[v].Waveform < 3-1) {
			AudioSource += Offsets[Voices[v].WaveLength];
		}
		if(Voices[v].Waveform == 4-1) {
			//AddRandomMoving
			AudioSource += (WNRandom & (2*0x280-1)) & ~1;
			//GoOnRandom
			WNRandom += 2239384;
			WNRandom = ((((WNRandom >> 8) | (WNRandom << 24)) + 782323) ^ 75) - 6735;
		}
		Voices[v].AudioSource = AudioSource;
	}
	//StillHoldWaveform
	//AudioInitPeriod
	Voices[v].AudioPeriod = Voices[v].InstrPeriod;
	if(!Voices[v].FixedNote) Voices[v].AudioPeriod += Voices[v].Transpose + Voices[v].TrackPeriod-1;
	if(Voices[v].AudioPeriod > 5*12) Voices[v].AudioPeriod = 5*12;
	if(Voices[v].AudioPeriod < 0)    Voices[v].AudioPeriod = 0;
	Voices[v].AudioPeriod = PeriodTable[Voices[v].AudioPeriod];
	if(!Voices[v].FixedNote) Voices[v].AudioPeriod += Voices[v].PeriodSlidePeriod;
	Voices[v].AudioPeriod += Voices[v].PeriodPerfSlidePeriod + Voices[v].VibratoPeriod;
	if(Voices[v].AudioPeriod > 0x0d60) Voices[v].AudioPeriod = 0x0d60;
	if(Voices[v].AudioPeriod < 0x0071) Voices[v].AudioPeriod = 0x0071;
	//AudioInitVolume
	Voices[v].AudioVolume = ((((((((Voices[v].ADSRVolume >> 8) * Voices[v].NoteMaxVolume) >> 6) * Voices[v].PerfSubVolume) >> 6) * Voices[v].TrackMasterVolume) >> 6) * MainVolume) >> 6;
}

void AHXPlayer_SetAudio(int v)
{
    int i, WaveLoops;

	if(!Voices[v].TrackOn) {
		Voices[v].VoiceVolume = 0;
		return;
	}

	Voices[v].VoiceVolume = Voices[v].AudioVolume;
	if(Voices[v].PlantPeriod) {
		Voices[v].PlantPeriod = 0;
		Voices[v].VoicePeriod = Voices[v].AudioPeriod;
	}
	if(Voices[v].NewWaveform) {
		if(Voices[v].Waveform == 4-1) {
			memcpy(Voices[v].VoiceBuffer, Voices[v].AudioSource, 0x280);
		} else {
			WaveLoops = (1 << (5-Voices[v].WaveLength))*5;
			for(i = 0; i < WaveLoops; i++) memcpy(&Voices[v].VoiceBuffer[i*4*(1 << Voices[v].WaveLength)], Voices[v].AudioSource, 4*(1 << Voices[v].WaveLength));
		}
		Voices[v].VoiceBuffer[0x280] = Voices[v].VoiceBuffer[0];
	}
}

void AHXPlayer_VoiceOnOff(int Voice, int OnOff)
{
	if(Voice < 0 || Voice > 3) return;
	Voices[Voice].TrackOn = OnOff;
}

// AHXOutput ///////////////////////////////////////////////////////////////////////////////////////////////


void AHXPlayer_PlayIRQ()
{
    int NextPos, i, a;
	if(StepWaitFrames <= 0) {
		if(GetNewPosition) {
			NextPos = (PosNr+1==Song.PositionNr)?0:(PosNr+1);
			for(i = 0; i < 4; i++) {
				Voices[i].Track = Positions[PosNr].Track[i];
				Voices[i].Transpose = Positions[PosNr].Transpose[i];
				Voices[i].NextTrack = Positions[NextPos].Track[i];
				Voices[i].NextTranspose = Positions[NextPos].Transpose[i];
			}
			GetNewPosition = 0;
		}
		for(i = 0; i < 4; i++) AHXPlayer_ProcessStep(i);
		StepWaitFrames = Tempo;
	}
	//DoFrameStuff
	for(i = 0; i < 4; i++) AHXPlayer_ProcessFrame(i);
	PlayingTime++;
	if(Tempo > 0 && --StepWaitFrames <= 0) {
		if(!PatternBreak) {
			NoteNr++;
			if(NoteNr >= Song.TrackLength) {
				PosJump = PosNr+1;
				PosJumpNote = 0;
				PatternBreak = 1;
			}
		}
		if(PatternBreak) {
			PatternBreak = 0;
			NoteNr = PosJumpNote;
			PosJumpNote = 0;
			PosNr = PosJump;
			PosJump = 0;
			if(PosNr == Song.PositionNr) {
				SongEndReached = 1;
				PosNr = Song.Restart;
			}
			GetNewPosition = 1;
		}
	}
	//RemainPosition
	for(a = 0; a < 4; a++) AHXPlayer_SetAudio(a);
}

void AHXPlayer_SetBoost(int boostval)
{
	Boost = boostval;
	AHXOutput_SetOption(AHXOF_BOOST, Boost+1);
}

void AHXPlayer_SetOversampling(int enable)
{
	Oversampling = enable;
}

void AHXOutput_MixChunk(int NrSamples, int** mb)
{
    int v, delta, samples_to_mix, mixpos, *VolTab, i, offset, sample1, sample2, frac1, frac2, thiscount;
	long freq;
    
    for(v = 0; v < 4; v++) {
		if(Voices[v].VoiceVolume == 0) continue;
		 freq = Period2Freq(Voices[v].VoicePeriod);
		 delta = (int)(freq * (1 << 16) / Frequency);
		 samples_to_mix = NrSamples;
		 mixpos = 0;
		while(samples_to_mix) {
			if(pos[v] > (0x280 << 16)) pos[v] -= 0x280 << 16;
			thiscount = min(samples_to_mix, ((0x280 << 16)-pos[v]-1) / delta + 1);
			samples_to_mix -= thiscount;
			VolTab = &VolumeTable[Voices[v].VoiceVolume][128];
			//INNER LOOP
			if(Oversampling) {
				for(i = 0; i < thiscount; i++) {
					offset = pos[v] >> 16;
					sample1 = VolTab[Voices[v].VoiceBuffer[offset]];
					sample2 = VolTab[Voices[v].VoiceBuffer[offset+1]];
					frac1 = pos[v] & ((1 << 16) - 1);
					frac2 = (1 << 16) - frac1;
					(*mb)[mixpos++] += ((sample1 * frac2) + (sample2 * frac1)) >> 16;
					pos[v] += delta;
				}
			} else {
				for(i = 0; i < thiscount; i++) {
					(*mb)[mixpos++] += VolTab[Voices[v].VoiceBuffer[pos[v] >> 16]];
					pos[v] += delta;
				}
			}
		} // while
	} // v = 0-3
	*mb += NrSamples;
}

void AHXOutput_MixBuffer(short* target)
{
	#define LOW_CLIP16     -0x8000
	#define HI_CLIP16       0x7FFF
	#define LOW_CLIP8      -0x80
	#define HI_CLIP8        0x7F
    int f;
	int NrSamples = Frequency / Hz / Song.SpeedMultiplier;
	int* mb = MixingBuffer;
	int thissample;
    int s;
	
	memset(MixingBuffer, 0, MixLen*Frequency/Hz*sizeof(int));
	for(f = 0; f < MixLen*Song.SpeedMultiplier /* MixLen = # frames */; f++) {
		AHXPlayer_PlayIRQ();
		AHXOutput_MixChunk(NrSamples, &mb);
	} // frames

	for(s = 0; s < BlockLen /(16/8); s++) 
	{
		thissample = *(MixingBuffer+s) << 6; // 16 bit		
        target[s] = thissample < LOW_CLIP16 ? LOW_CLIP16 : thissample > HI_CLIP16 ? HI_CLIP16 : thissample;
    }
}
