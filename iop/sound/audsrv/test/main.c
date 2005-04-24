/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# audsrv sample
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <tamtypes.h>
#include <string.h>

#include <audsrv.h>

#define	AUDSRV_IRX        0x870884d

#define MIN(a,b) ((a) <= (b)) ? (a) : (b)

static struct t_SifRpcClientData cd0;

static unsigned int sbuff[4096] __attribute__((aligned (64)));

int audsrv_quit()
{
	SifCallRpc(&cd0, AUDSRV_QUIT, 0, sbuff, 1*4, sbuff, 4, 0, 0);
	return 0;
}

int audsrv_set_format(struct audsrv_fmt_t *fmt)
{
	sbuff[0] = fmt->freq;
	sbuff[1] = fmt->bits;
	sbuff[2] = fmt->channels;
	SifCallRpc(&cd0, AUDSRV_SET_FORMAT, 0, sbuff, 3*4, sbuff, 4, 0, 0);
	FlushCache(0);
	return sbuff[0];
}

int audsrv_wait_audio(int samples)
{
	int bytes;

	bytes = 4 * samples;
	sbuff[0] = bytes;
	SifCallRpc(&cd0, AUDSRV_WAIT_AUDIO, 0, sbuff, 1*4, sbuff, 4, 0, 0);
	FlushCache(0);
	return sbuff[0];
}

int audsrv_play_audio(const char *chunk, int samples)
{
	int copy, maxcopy;
	int packet_size;
	int bytes;

	maxcopy = sizeof(sbuff) - sizeof(int);
	while (samples > 0)
	{
		copy = MIN(samples, maxcopy);
		sbuff[0] = copy;
		memcpy(&sbuff[1], chunk, copy);
		packet_size = copy + sizeof(int);
		SifCallRpc(&cd0, AUDSRV_PLAY_AUDIO, 0, sbuff, packet_size, sbuff, 4, 0, 0);
		FlushCache(0);

		chunk = chunk + copy;
		samples = samples - copy;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	int played;
	int err;
	char chunk[2048];
	FILE *wav;
	struct audsrv_fmt_t format;

	SifInitRpc(0); 

	printf("kicking irx\n");

	ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	printf("libsd loadmodule %d\n", ret);

	printf("load audsrv\n");
	ret = SifLoadModule("host:audsrv.irx", 0, NULL);
	printf("audsrv loadmodule %d\n", ret);

	memset(&cd0, '\0', sizeof(cd0));

	printf("rpc binding");
	while (1)
	{
		printf(".");
		if (SifBindRpc(&cd0, AUDSRV_IRX, 0) < 0)
		{
			printf("bind rpc failed\n");
			return -1;
		}

 		if (cd0.server != 0) 
		{
			break;
		}

		nopdelay();
	}

	printf("\n");

	SifCallRpc(&cd0, AUDSRV_INIT, 0, sbuff, 64, sbuff, 64, 0, 0);
	FlushCache(0);
	printf("init returned %08x\n", sbuff[0]);

	format.bits = 16;
	format.freq = 48000;
	format.channels = 2;
	err = audsrv_set_format(&format);
	printf("set format returned %d\n", err);

	wav = fopen("host:song_48k.wav", "rb");
	fseek(wav, 0x30, SEEK_SET);

	printf("sizeof(int) = %d\n", sizeof(int));

	printf("starting play loop\n");
	played = 0;
	while (1)
	{
		ret = fread(chunk, 1, sizeof(chunk), wav);
		if (ret > 0)
		{
			audsrv_wait_audio(ret);
			audsrv_play_audio(chunk, ret);
		}

		if (ret < sizeof(chunk))
		{
			/* no more data */
			break;
		}

		played++;
		if (played % 8 == 0)
		{
			printf(".");
		}

		//if (played == 1024) break;
	}

	fclose(wav);

	printf("going back home\n");
	audsrv_quit();
	printf("audsrv dead\n");

	return 0;
}
