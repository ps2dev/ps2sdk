/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, James Lee (jbit<at>jbit<dot>net)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: $
*/



/*
	Based on:
	PSX VAG-Packer, hacked by bITmASTER@bigfoot.com, hacked by jbit :)
	jbit's note: 
*/

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "adpcm.h"

#define ADPCM_LOOP_START    4  /* Set on first block of looped data */
#define ADPCM_LOOP          2  /* Set on all blocks (?that are inside the loop?) */
#define ADPCM_LOOP_END      1  /* Set on last block to loop */

#define PACKED __attribute__((packed))

typedef struct 
{
	unsigned shift:      4  PACKED;
	unsigned predict:    4  PACKED;
	uint8_t  flags;
	uint8_t  sample[14]; /* 4bits each */
} AdpcmBlock;

static void find_predict(AdpcmSetup *set, AdpcmBlock *adpcm, double *samples);
static void pack(AdpcmSetup *set, AdpcmBlock *adpcm, double *samples);


AdpcmSetup *AdpcmCreate(AdpcmGetPCMfunc get, void *getpriv, AdpcmPutADPCMfunc put, void *putpriv, int loopstart)
{
	AdpcmSetup *set;

	set = malloc(sizeof(AdpcmSetup));
	if (set==NULL)
		return(NULL);

	set->s_1 = set->s_2 = 0.0;
	set->ps_1 = set->ps_2 = 0.0;

	set->curblock = 0;

	if (loopstart<0) /* disable looping (single shot) */
		set->loopstart = -1;
	else
		set->loopstart = loopstart;

	set->GetPCM = get;
	set->getpriv = getpriv;

	set->PutADPCM = put;
	set->putpriv = putpriv;
	set->pad = 0;
	return(set);
}

int AdpcmDestroy(AdpcmSetup *set)
{
	free(set);
	return(0);
}

int AdpcmEncode(AdpcmSetup *set, int blocks)
{
	AdpcmBlock adpcm;
	double samples[28];
	int procblocks;
 
	for (procblocks=0;procblocks<blocks;procblocks++)
	{
		int ret;
		adpcm.flags = 0;

		for (int j=0;j<28;j++)
			samples[j] = 0.0;

		ret = set->GetPCM(set->getpriv, samples, 28);
		if (ret<0)
			return(-1);
		if (ret<28)
		{
			printf("loop end!\n");
			adpcm.flags = ADPCM_LOOP_END;
		}

		if (set->loopstart>=0)
		{
			adpcm.flags |= ADPCM_LOOP;
			if (set->curblock == set->loopstart)
			{
				printf("loop start!\n");
				adpcm.flags |= ADPCM_LOOP_START;
			}
		}


		find_predict(set, &adpcm, samples);
		pack(set, &adpcm, samples);

		if (set->PutADPCM(set->putpriv, &adpcm, 16)<0)
			return(-1);

		set->curblock++;
		if (ret<28)
			break;
	}
    
	if (set->loopstart<0 && procblocks<blocks)
	{
		/* this block essentialy loops to itself and contains no data */
		adpcm.predict = 0;
		adpcm.shift = 0;
		adpcm.flags = ADPCM_LOOP_START | ADPCM_LOOP | ADPCM_LOOP_END;

		for (int i=0;i<14;i++)
			adpcm.sample[i] = 0;
		if (set->PutADPCM(set->putpriv, &adpcm, 16)<0)
			return(-1);
		set->curblock++;
	}

	if (procblocks<blocks && set->pad)
	{
		int padblocks = blocks-(set->curblock%blocks);

		adpcm.predict = 0;
		adpcm.shift = 0;
		adpcm.flags = ADPCM_LOOP_START | ADPCM_LOOP | ADPCM_LOOP_END;
		for (int i=0;i<14;i++)
			adpcm.sample[i] = 0;


		for (int i=0;i<padblocks;i++)
		{
			if (set->PutADPCM(set->putpriv, &adpcm, 16)<0)
			return(-1);
			set->curblock++;
		}
	}	
	
	return(procblocks);
}

static double f[5][2] =
{
	{           0.0, 0.0},
   	{  -60.0 / 64.0, 0.0},
   	{ -115.0 / 64.0, 52.0 / 64.0},
   	{  -98.0 / 64.0, 55.0 / 64.0},
   	{ -122.0 / 64.0, 60.0 / 64.0}
};
#define MAX(_a,_b)   ((_a)>(_b) ? (_a):(_b))
#define MIN(_a,_b)   ((_a)<(_b) ? (_a):(_b))
#define CLAMP(_v,_min,_max) MIN(MAX(_v,_min),_max);

static void find_predict(AdpcmSetup *set, AdpcmBlock *adpcm, double *samples)
{
	double buffer[28][5];
	double min = 1e10;
	double max[5];
	double ds;
	int min2;
	int shift_mask;
	double s_0, s_1, s_2;

	for (int i=0;i<5;i++) 
	{
		max[i] = 0.0;
		s_1 = set->s_1;
		s_2 = set->s_2;

		for (int j=0;j<28;j++) 
		{
			s_0 = CLAMP(samples[j],-30720.0,30719.0);

			ds = s_0 + s_1 * f[i][0] + s_2 * f[i][1];
			buffer[j][i] = ds;
			if (fabs(ds) > max[i])
				max[i] = fabs(ds);

			s_2 = s_1;
			s_1 = s_0;
		}
    
		if ( max[i] < min ) 
		{
			min = max[i];
			adpcm->predict = i;
		}
		if (min <= 7) 
		{
			adpcm->predict = 0;
			break;
		}
	}

	set->s_1 = s_1;
	set->s_2 = s_2;
    
	for (int  i=0;i<28;i++ )
		samples[i] = buffer[i][adpcm->predict];
      
	min2 = (int)min;
	shift_mask = 0x4000;
	adpcm->shift = 0;
    
	while(adpcm->shift < 12) 
	{
		if (shift_mask & ( min2 + (shift_mask>>3) ))
			break;
		adpcm->shift++;
		shift_mask = shift_mask >> 1;
	}
}

static void pack(AdpcmSetup *set, AdpcmBlock *adpcm, double *samples)
{
	double ds;
	int di;
	double s_0, s_1, s_2;
	short four_bit[28];

	s_1 = set->ps_1;
	s_2 = set->ps_2;

	for (int i=0;i<28;i++) 
	{
		s_0 = samples[i] + s_1 * f[adpcm->predict][0] + s_2 * f[adpcm->predict][1];
		ds = s_0 * (double) (1<<adpcm->shift);

		di = ((int)ds+0x800) & 0xfffff000;
		di = CLAMP(di,-32768,32767);

		four_bit[i] = (short)di;

		di = di >> adpcm->shift;
		s_2 = s_1;
		s_1 = (double) di - s_0;
	}

	for (int i=0;i<14;i++) 
		adpcm->sample[i] = ( ( four_bit[(i*2)+1] >> 8 ) & 0xf0 ) | ( ( four_bit[i*2] >> 12 ) & 0xf );

	set->ps_1 = s_1;
	set->ps_2 = s_2;
}
