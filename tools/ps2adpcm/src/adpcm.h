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

#ifndef _ADPCM_H_
#define _ADPCM_H_

typedef int (*AdpcmGetPCMfunc)  (void *priv, double *pcm, int len); /* Length in samples */
typedef int (*AdpcmPutADPCMfunc)(void *priv, void  *data, int len); /* Length in bytes */

typedef struct
{
	AdpcmGetPCMfunc   GetPCM;
	AdpcmPutADPCMfunc PutADPCM;
	double s_1, s_2;
	double ps_1, ps_2;
	int curblock;  /* Current block */
	int loopstart; /* Loop start position, in ADPCM blocks (28 PCM samples) */
	void *getpriv;
	void *putpriv;
	int pad;
} AdpcmSetup;

AdpcmSetup *AdpcmCreate(AdpcmGetPCMfunc get, void *getpriv, AdpcmPutADPCMfunc put, void *putpriv, int loopstart);
int AdpcmDestroy(AdpcmSetup *set);
int AdpcmEncode(AdpcmSetup *set, int blocks);


#endif
