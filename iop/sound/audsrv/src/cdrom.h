/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
*/

/**
 * @file
 * cdsrv IOP server.
 */

#ifndef __CDROM_C_H__
#define __CDROM_C_H__

#define SECTOR_SIZE                2352

typedef struct cdda_toc
{
	int num_tracks;
	sceCdlLOCCD tracks[100];
} cdda_toc;

#endif
