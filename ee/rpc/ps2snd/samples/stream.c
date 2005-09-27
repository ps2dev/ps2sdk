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

#include <kernel.h>
#include <loadfile.h>
#include <stdio.h>
#include <ps2snd.h>

int main(void)
{
	int ret;
	/* Load LibSD (freesd will work too one day, I promise ;) */
	ret = SifLoadModule("host:LIBSD.IRX", 0, NULL);
	if (ret<0)
	{
		printf("XXXXX failed to load host:LIBSD.IRX (%d)\n", ret);
		SleepThread();
	}

	/* Load ps2snd */
	ret = SifLoadModule("host:ps2snd.irx", 0, NULL);
	if (ret<0)
	{
		printf("XXXXX failed to load host:ps2snd.irx (%d)\n", ret);
		SleepThread();
	}

	/* Start LibSD */
	if (SdInit(0)<0)
	{
		printf("Failed to start LibSD\n");
		SleepThread();
	}

	/* Setup master volumes for both cores */
	SdSetParam(0 | SD_PARAM_MVOLL, 0x3fff);
	SdSetParam(0 | SD_PARAM_MVOLR, 0x3fff);
	SdSetParam(1 | SD_PARAM_MVOLL, 0x3fff);
	SdSetParam(1 | SD_PARAM_MVOLR, 0x3fff);

	/*
		Open a stream, using voices 0:22 and 0:23
		It's stereo and it should be closed when the end is hit.
		The SPU buffers are at 0x6000 in spu2 ram.
		The chunksize is 1024 blocks (16kbyte)
	*/
	if (sndStreamOpen("host:stream.adpcm", SD_VOICE(0,22) | (SD_VOICE(0,23)<<16), STREAM_STEREO | STREAM_END_CLOSE, 0x6000, 1024)<0)
	{
		printf("Failed to open stream\n");
		SleepThread();
	}


	sndStreamPlay();

	SleepThread();

	return(0);
}

