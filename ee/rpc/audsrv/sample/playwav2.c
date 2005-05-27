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
# audsrv sample using callbacks
*/

#include <stdio.h>
#include <string.h>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>

#include <audsrv.h>

static int fillbuffer(void *arg)
{
	iSignalSema((int)arg);
	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	int played;
	int err;
	int bytes;
	char chunk[2048];
	FILE *wav;
	ee_sema_t sema;
	int fillbuffer_sema;
	struct audsrv_fmt_t format;

	SifInitRpc(0); 

	printf("sample: kicking IRXs\n");
	ret = SifLoadModule("rom0:LIBSD", 0, NULL);
	printf("libsd loadmodule %d\n", ret);

	printf("sample: loading audsrv\n");
	ret = SifLoadModule("host:audsrv.irx", 0, NULL);
	printf("audsrv loadmodule %d\n", ret);

	ret = audsrv_init();
	if (ret != 0)
	{
		printf("sample: failed to initialize audsrv\n");
		printf("audsrv returned error string: %s\n", audsrv_get_error_string());
		return 1;
	}

	format.bits = 16;
	format.freq = 22050;
	format.channels = 2;
	err = audsrv_set_format(&format);
	printf("set format returned %d\n", err);
	printf("audsrv returned error string: %s\n", audsrv_get_error_string());

	audsrv_set_volume(MAX_VOLUME);

	sema.init_count = 0;
	sema.max_count = 1;
	sema.option = 0;
	fillbuffer_sema = CreateSema(&sema);

	err = audsrv_on_fillbuf(sizeof(chunk), fillbuffer, (void *)fillbuffer_sema);
	if (err != AUDSRV_ERR_NOERROR)
	{
		printf("audsrv_on_fillbuf failed with err=%d\n", err);
		goto loser;
	}

	wav = fopen("host:song_22k.wav", "rb");
	if (wav == NULL)
	{
		printf("failed to open wav file\n");
		audsrv_quit();
		return 1;
	}

	fseek(wav, 0x30, SEEK_SET);

	printf("starting play loop\n");
	played = 0;
	bytes = 0;
	while (1)
	{
		ret = fread(chunk, 1, sizeof(chunk), wav);
		if (ret > 0)
		{
			WaitSema(fillbuffer_sema);
			audsrv_play_audio(chunk, ret);
		}

		if (ret < sizeof(chunk))
		{
			/* no more data */
			break;
		}

		played++;
		bytes = bytes + ret;

		if (played % 8 == 0)
		{
			printf("\r%d bytes sent..", bytes);
		}

		if (played == 512) break;
	}

	fclose(wav);

loser:
	printf("sample: stopping audsrv\n");
	audsrv_quit();

	printf("sample: ended\n");
	return 0;
}
