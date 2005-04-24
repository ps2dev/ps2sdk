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
#include <string.h>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>

#include <audsrv.h>

int main(int argc, char **argv)
{
	int ret;
	int played;
	int err;
	char chunk[2048];
	FILE *wav;
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

		if (played == 512) break;
	}

	fclose(wav);

	printf("sample: stopping audsrv\n");
	audsrv_quit();

	printf("sample: ended\n");
	return 0;
}
