/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Modem interface.
 * Intended for network stacks (PPP) to implement.
 */

#ifndef __MODEM_H__
#define __MODEM_H__

#include <types.h>
#include <irx.h>

typedef struct sceModemOps
{
    struct sceModemOps *forw;
    struct sceModemOps *back;
    char *module_name;
    char *vendor_name;
    char *device_name;
    u8 bus_type;
    u8 bus_loc[31];
    u16 prot_ver;
    u16 impl_ver;
    void *priv;
    int evfid;
    int rcv_len;
    int snd_len;
    int (*start)(void *priv, int flags);
    int (*stop)(void *priv, int flags);
    int (*recv)(void *priv, void *ptr, int len);
    int (*send)(void *priv, void *ptr, int len);
    int (*control)(void *priv, int code, void *ptr, int len);
    void *reserved[4];
} sceModemOps_t;

extern int sceModemRegisterDevice(sceModemOps_t *ops);
extern int sceModemUnregisterDevice(sceModemOps_t *ops);

#define modem_IMPORTS_start DECLARE_IMPORT_TABLE(modem, 1, 1)
#define modem_IMPORTS_end END_IMPORT_TABLE

#define I_sceModemRegisterDevice DECLARE_IMPORT(4, sceModemRegisterDevice)
#define I_sceModemUnregisterDevice DECLARE_IMPORT(5, sceModemUnregisterDevice)

#endif
