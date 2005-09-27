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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdint.h>
#include "adpcm.h"

#define dprintf(format, args...) \
	fprintf(stderr, "ps2adpcm(%s): " format, __FUNCTION__, ## args)

typedef struct
{
	int Position;
	int Channel;
	int ChannelCount;
	short *Sample;
	int    SampleCount;
} PcmBuffer;

int GetPCM(void *priv, double *out, int len)
{
	int i;
	PcmBuffer *pcm = priv;

	for (i=0;i<len;i++)
	{
		if (pcm->Position+i>=pcm->SampleCount)
			break;
		out[i] = pcm->Sample[((pcm->Position+i)*pcm->ChannelCount)+pcm->Channel];
	}
	pcm->Position += i;

	return(i);
}


int PutADPCM(void *priv, void *data, int len)
{
	FILE *f = priv;
	int r;
	r = fwrite(data, 1, len, f);
	if (r<0)
	{
		dprintf("write error (%s)\n", strerror(errno));
		return(-1);
	}

	return(r);
}

int main(int argc, char *argv[])
{
	char *infile, *outfile;
	FILE *fi, *fo;
	int bpc = 1024; /* ADPCM (16byte, 28sample) blocks per chunk */
	int loopstart = -1;
	AdpcmSetup *set[2];
	PcmBuffer pcm;

	pcm.ChannelCount = 1; 

	if (argc<3)
	{
		dprintf("usage:   %s <PCM Input> <ADPCM Output> -s(tereo) -c[chunksize] -l[loopstart]\n", argv[0]);
		dprintf("example: %s - output.adpcm -s -c1024 -s1000\n", argv[0]);

		return(1);
	}

	infile = argv[1];
	outfile = argv[2];

	for (int i=3;i<argc;i++)
	{
		long int num;
		if (argv[i][0] != '-' || argv[i][1] == '\0')
		{
			dprintf("%s: invalid option '%s'\n", argv[0], argv[i]);
			return(1);
		}
		num = strtol(&argv[i][2], NULL, 0);
		switch(argv[i][1])
		{
		case 's': pcm.ChannelCount = 2; break;
		case 'c':
			if (num<=0 || num >= 65536)
			{
				dprintf("%s: invalid block size (%ld, '%s')\n", argv[0], num, &argv[i][2]);
				return(1);
			}
			bpc = num;
			break;
		case 'l':
			if (num<0)
			{
				dprintf("%s: invalid loop start address (%ld, '%s')\n", argv[0], num, &argv[i][2]);
				return(1);
			}
			loopstart = num;
			break;
		default:
			dprintf("%s: unkown option '%s'\n", argv[0], argv[i]);
			return(1);
			break;
		}		
	}


	if (!strcmp(infile, "-"))
		fi = stdin;
	else
	{
		fi = fopen(infile, "rb");
		if (fi==NULL)
		{
			dprintf("Failed to open input file '%s' (%s)\n", infile, strerror(errno));
			return(1);
		}
	}

	fo = fopen(outfile, "wb");
	if (fo==NULL)
	{
		dprintf("Failed to open output file '%s' (%s)\n", outfile, strerror(errno));
		return(1);
	}

	for (int i=0;i<pcm.ChannelCount;i++)
	{
		set[i] = AdpcmCreate(GetPCM, &pcm, PutADPCM, fo, loopstart);
		if (set[i]==NULL)
		{
			dprintf("Failed to create ADPCM setup\n");
			return(1);
		}
	}

	if (pcm.ChannelCount>1)
	{
		set[0]->pad = 1;
		set[1]->pad = 1;
	}

	pcm.Sample = malloc((bpc*28)*pcm.ChannelCount*2);
	do
	{
		int i, r;
		r = fread(pcm.Sample, 2*pcm.ChannelCount, (bpc*28), fi);
		if (r<0)
			break;
		pcm.SampleCount = r;

		for (i=0;i<pcm.ChannelCount;i++)
		{
			pcm.Position = 0;
			pcm.Channel = i;
			if (AdpcmEncode(set[i], bpc)!=bpc)
				break;
		}
		if (i!=pcm.ChannelCount)
			break;
	} while(1);	

	return(0);
}


