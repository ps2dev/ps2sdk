/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
#
# $Id$
# cdsrv IOP server.
*/

#ifndef __CDROM_C_H__
#define __CDROM_C_H__

/**
 * \file cdrom.c.h
 * \author gawd (Gil Megidish)
 * \date 05-09-05
 */

#define SECTOR_SIZE                2352

#define SD_CORE_0               0

typedef struct cdda_toc
{
	int num_tracks;
	cd_location_t tracks[100];
} cdda_toc;

#endif
