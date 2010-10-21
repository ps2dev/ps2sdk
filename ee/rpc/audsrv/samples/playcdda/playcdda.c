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
# audsrv cdda sample
*/

#include <stdio.h>
#include <string.h>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <tamtypes.h>

#include <audsrv.h>

static volatile int track_ended = 0;

static void cdda_stopped(void *arg)
{
	track_ended = 1;
}

int main(int argc, char **argv)
{
	int ret;
	int track;
	int lastpos;

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

	printf("setting callback\n");
	audsrv_on_cdda_stop((void *)cdda_stopped, 0);

	track = 4;
	printf("playing track %d\n", track);
	audsrv_play_cd(track);

	//printf("starting loop\n");

	lastpos = 0;
	while (track_ended == 0)
	{
		int pos;

		if (audsrv_get_cdpos() == 0)
		{
			printf("-- track ended before semaphore --\n");
			break;
		}

		pos = audsrv_get_trackpos();
		nopdelay();

		if (lastpos != pos)
		{
			printf("\rTrack %02d: %02d:%02d:%02d", 
			track, pos / (75*60), (pos / 75) % 60, pos % 75);
			lastpos = pos;
		}
	}

	printf("\n");
	printf("track ended\n");

	audsrv_quit();
	printf("sample: ended\n");
	return 0;
}
