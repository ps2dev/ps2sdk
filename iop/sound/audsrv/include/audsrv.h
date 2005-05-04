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
# audsrv IOP server
*/

#ifndef __AUDSRV_H__
#define __AUDSRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AUDSRV_INIT          0x0000
#define AUDSRV_QUIT          0x0001

#define AUDSRV_FORMAT_OK     0x0002
#define AUDSRV_SET_FORMAT    0x0003
#define AUDSRV_PLAY_AUDIO    0x0004
#define AUDSRV_WAIT_AUDIO    0x0005
#define AUDSRV_STOP_AUDIO    0x0006
#define AUDSRV_SET_VOLUME    0x0007

/* cdrom functions */
#define AUDSRV_PLAY_CD       0x0008
#define AUDSRV_STOP_CD       0x0009
#define AUDSRV_SET_CDVOL     0x000a

/* error codes */
#define AUDSRV_ERR_NOERROR                 0x0000
#define AUDSRV_ERR_NOT_INITIALIZED         0x0001
#define AUDSRV_ERR_RPC_FAILED              0x0002
#define AUDSRV_ERR_FORMAT_NOT_SUPPORTED    0x0003
#define AUDSRV_ERR_OUT_OF_MEMORY           0x0004
#define AUDSRV_ERR_ARGS                    0x0005

int audsrv_init();
#define I_audsrv_init DECLARE_IMPORT(audsrv_init, 4)

int audsrv_format_ok(int freq, int bits, int channels);
#define I_audsrv_format_ok DECLARE_IMPORT(audsrv_format_ok, 5)

int audsrv_set_format(int freq, int bits, int channels);
#define I_audsrv_set_format DECLARE_IMPORT(audsrv_set_format, 6)

int audsrv_wait_audio(int buflen);
#define I_audsrv_wait_audio DECLARE_IMPORT(audsrv_wait_audio, 7)

int audsrv_play_audio(const char *buf, int buflen);
#define I_audsrv_play_audio DECLARE_IMPORT(audsrv_play_audio, 8)

int audsrv_stop_audio();
#define I_audsrv_stop_audio DECLARE_IMPORT(audsrv_stop_audio, 9)

int audsrv_set_volume(int vol);
#define I_audsrv_set_volume DECLARE_IMPORT(audsrv_set_volume, 10)

int audsrv_quit();
#define I_audsrv_quit DECLARE_IMPORT(audsrv_quit, 11)

int audsrv_play_cd(int track);
#define I_audsrv_play_cd DECLARE_IMPORT(audsrv_play_cd, 12)

int audsrv_set_cdvol(int vol);
#define I_audsrv_set_cdvol DECLARE_IMPORT(audsrv_set_cdvol, 13)

int audsrv_stop_cd();
#define I_audsrv_stop_cd DECLARE_IMPORT(audsrv_stop_cd, 14)

#ifdef __cplusplus
}
#endif

#endif
