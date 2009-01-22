/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: playadpcm.c 1221 2005-10-01 16:19:49Z gawd $
# audsrv adpcm sample
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
	int i, ret;
	FILE* adpcm;
	audsrv_adpcm_t sample;
	int size;
	u8* buffer;

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

	adpcm = fopen("host:evillaugh.adp", "rb");

	if (adpcm == NULL)
	{
		printf("failed to open adpcm file\n");
		audsrv_quit();
		return 1;
	}

	fseek(adpcm, 0, SEEK_END);
	size = ftell(adpcm);
	fseek(adpcm, 0, SEEK_SET);

	buffer = malloc(size);

	fread(buffer, 1, size, adpcm);
	fclose(adpcm);

	printf("playing sample..\n");

	audsrv_adpcm_init();
	audsrv_set_volume(MAX_VOLUME);
	audsrv_load_adpcm(&sample, buffer, size);
	audsrv_play_adpcm(&sample);

	/* Uncomment to hear two samples played simultaenously
	for (i=0; i<100; i++)
	{
		nopdelay();
	}

	audsrv_play_adpcm(&sample);
	*/

	printf("sample played..\n");

	free(buffer);

	while (1);
	return 0;
}