/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: testcd.c 1116 2005-05-27 16:49:33Z gawd $
# audsrv cdda toc sample
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
	int n, track;
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

	n = audsrv_get_numtracks();
	printf("There are %d tracks on this disc\n", n);

	lastpos = 0;
	for (track=0; track<=n; track++)
	{
		int pos = audsrv_get_track_offset(track);

		if (track > 0)
		{
			int length = pos - lastpos;

			printf("Track %02d: sector 0x%x, length: %02d:%02d:%02d\n",
			track, pos, length / (75*60), (length / 75) % 60, length % 75);
		}

		lastpos = pos;
	}

	audsrv_quit();
	printf("sample: ended\n");
	return 0;
}
