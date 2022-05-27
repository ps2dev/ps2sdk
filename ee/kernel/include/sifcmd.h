/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE SIF commands prototypes
 */

#ifndef __SIFCMD_H__
#define __SIFCMD_H__

#include <tamtypes.h>

typedef struct t_SifCmdHeader
{
    /** Packet size. Min: 1x16 (header only), max: 7*16 */
    u32 psize : 8;
    /** Payload size */
    u32 dsize : 24;
    /** Destination address for payload. Can be NULL if there is no payload. */
    void *dest;
    /** Function number of function to call. */
    int cid;
    /** Can be freely used. */
    u32 opt;
} SifCmdHeader_t;

/** System functions */
#define SIF_CMD_ID_SYSTEM 0x80000000

#define SIF_CMD_CHANGE_SADDR (SIF_CMD_ID_SYSTEM | 0)
#define SIF_CMD_SET_SREG     (SIF_CMD_ID_SYSTEM | 1)
#define SIF_CMD_INIT_CMD     (SIF_CMD_ID_SYSTEM | 2)
#define SIF_CMD_RESET_CMD    (SIF_CMD_ID_SYSTEM | 3)
#define SIF_CMD_RPC_END      (SIF_CMD_ID_SYSTEM | 8)
#define SIF_CMD_RPC_BIND     (SIF_CMD_ID_SYSTEM | 9)
#define SIF_CMD_RPC_CALL     (SIF_CMD_ID_SYSTEM | 10)
#define SIF_CMD_RPC_RDATA    (SIF_CMD_ID_SYSTEM | 12)

/** System SREG */
#define SIF_SREG_RPCINIT 0

/** Structure for remotely (over the SIF) changing the value of a software register (SREG).
 * There are 32 software registers (0 - 31). Registers 0-7 are used by the system.
 */
typedef struct t_SifCmdSRegData
{
    SifCmdHeader_t header;
    int index;
    unsigned int value;
} SifCmdSRegData_t;

#ifdef __cplusplus
extern "C"
#endif
    typedef void (*SifCmdHandler_t)(void *data, void *harg);

typedef struct t_SifCmdHandlerData
{
    SifCmdHandler_t handler;
    void *harg;
} SifCmdHandlerData_t;

/** Triggers an IOP reboot */
#define RESET_ARG_MAX 79

typedef struct _iop_reset_pkt
{
    struct t_SifCmdHeader header;
    int arglen;
    int mode;
    char arg[RESET_ARG_MAX + 1];
} SifCmdResetData_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned int SifSendCmd(int cmd, void *packet, int packet_size, void *src_extra,
                        void *dest_extra, int size_extra);
unsigned int iSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra,
                         void *dest_extra, int size_extra);
void SifAddCmdHandler(int pos, SifCmdHandler_t handler, void *harg);
void SifRemoveCmdHandler(int pos);
void SifInitCmd(void);
void SifExitCmd(void);
SifCmdHandlerData_t *SifSetCmdBuffer(SifCmdHandlerData_t *db, int size);
int SifGetSreg(int index);

void SifWriteBackDCache(void *ptr, int size); // EE only

// Send mode bits
/** Called within an interrupt context */
#define SIF_CMD_M_INTR 0x01
/** Write back D-cache for extended data */
#define SIF_CMD_M_WBDC 0x04

#ifdef __cplusplus
}
#endif

#endif /* __SIFCMD_H__ */
