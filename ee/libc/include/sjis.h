/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * sjis function prototypes
 */

#ifndef __SJIS_H__
#define __SJIS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** copies ascii string to sjis string
 * @param sjis_buff dest sjis string buffer
 * @param ascii_buff source ascii string buffer
 * @return length of ascii string copied
 */
int strcpy_sjis(short* sjis_buff, const char* ascii_buff);

/** copies sjis string to ascii string
 *
 * @param ascii_buff dest ascii string buffer
 * @param sjis_buff source sjis string buffer
 * @return length of sjis string copied
 */
int strcpy_ascii(char* ascii_buff, const short* sjis_buff);

#ifdef __cplusplus
}
#endif

#endif /* __SJIS_H__ */
