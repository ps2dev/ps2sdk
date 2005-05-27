/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
#
# $Id$
# audsrv IOP-side upsamplers
*/

#ifndef __UPSAMPLERS_INCLUDED__
#define __UPSAMPLERS_INCLUDED__

/**
 * \file upsamplers.h
 * \author gawd (Gil Megidish)
 * \date 04-24-05
 */

typedef struct upsample_t
{
	short *left;
	short *right;
	const unsigned char *src;
} upsample_t;

typedef int (*upsampler_t)(struct upsample_t *);

upsampler_t find_upsampler(int freq, int bits, int channels);

#endif
