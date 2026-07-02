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

#define sceModemBus_Unknown 0
#define sceModemBus_USB 1
#define sceModemBus_1394 2
#define sceModemBus_PCMCIA 3
#define sceModemBus_PSEUDO 4
// The following definition is unofficial.
#define sceModemBus_NIC 5

#define sceModemProtVer 0
#define sceModemProtVerMask 0x00FF
#define sceModemProtSkipDLL 0x0100
#define sceModemProtOverEther 0x0100
#define sceModemProtOverATM 0x0300

#define sceModemEFP_StartDone 0x00000001
#define sceModemEFP_PlugOut 0x00000002
#define sceModemEFP_Connect 0x00000010
#define sceModemEFP_Disconnect 0x00000020
#define sceModemEFP_Ring 0x00000040
#define sceModemEFP_Recv 0x00000100
#define sceModemEFP_Send 0x00000200
#define sceModemEFP_UpperUse 0xFFFF0000

#define sceModemCC_GET_THPRI 0xC0000000
#define sceModemCC_SET_THPRI 0xC1000000
#define sceModemCC_GET_IF_TYPE 0xC0000100

#define sceModemIFT_GENERIC 0x00000000
#define sceModemIFT_SERIAL 0x00000001

#define sceModemCC_FLUSH_RXBUF 0xC0000110
#define sceModemCC_FLUSH_TXBUF 0xC0000111
#define sceModemCC_GET_DIALCONF 0xC0000200
#define sceModemCC_GET_RX_COUNT 0xC0010000
#define sceModemCC_GET_TX_COUNT 0xC0010001
#define sceModemCC_GET_OE_COUNT 0xC0010002
#define sceModemCC_GET_PE_COUNT 0xC0010003
#define sceModemCC_GET_FE_COUNT 0xC0010004
#define sceModemCC_GET_BO_COUNT 0xC0010005
#define sceModemCC_GET_PARAM 0xC0020000
#define sceModemCC_SET_PARAM 0xC1020000

#define sceModemPARAM_SPEED 0x003FFFFF
#define sceModemPARAM_RESERVED 0x00400000
#define sceModemPARAM_XON 0x00800000
#define sceModemPARAM_XOFF 0x01000000
#define sceModemPARAM_RTSCTS 0x02000000
#define sceModemPARAM_STOPS 0x0C000000
#define sceModemPARAM_STOP0 0x00000000
#define sceModemPARAM_STOP1 0x04000000
#define sceModemPARAM_STOP1H 0x08000000
#define sceModemPARAM_STOP2 0x0C000000
#define sceModemPARAM_CSIZE 0x30000000
#define sceModemPARAM_CS5 0x00000000
#define sceModemPARAM_CS6 0x10000000
#define sceModemPARAM_CS7 0x20000000
#define sceModemPARAM_CS8 0x30000000
#define sceModemPARAM_PARODD 0x40000000
#define sceModemPARAM_PARENB 0x80000000

#define sceModemCC_GET_LINE 0xC0030000
#define sceModemCC_SET_LINE 0xC1030000

#define sceModemLINE_CTS 0x00000001
#define sceModemLINE_DSR 0x00000002
#define sceModemLINE_RI 0x00000004
#define sceModemLINE_DCD 0x00000008
#define sceModemLINE_DTR 0x00000010
#define sceModemLINE_RTS 0x00000020

#define sceModemCC_SET_BREAK 0xC1040000

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
