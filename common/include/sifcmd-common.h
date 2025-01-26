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
 * Common definitions for SIF CMD.
 */

#ifndef __SIFCMD_COMMON_H__
#define __SIFCMD_COMMON_H__

#include <tamtypes.h>

/** SIF command.  */
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

typedef struct t_SifCmdSysHandlerData
{
    SifCmdHandler_t handler;
    void *harg;
    void *unknown08;
} SifCmdSysHandlerData_t;

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

extern void sceSifInitCmd(void);
extern void sceSifExitCmd(void);
extern unsigned int sceSifGetSreg(int sreg);
extern void sceSifSetSreg(int sreg, unsigned int value);
extern void sceSifSetCmdBuffer(SifCmdHandlerData_t *db, int size);
extern void sceSifSetSysCmdBuffer(SifCmdSysHandlerData_t *db, int size);
extern void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg);
extern void sceSifRemoveCmdHandler(int cid);
extern unsigned int sceSifSendCmd(int cid, void *packet, int packet_size, void *src_extra,
    void *dest_extra, int size_extra);
extern unsigned int isceSifSendCmd(int cid, void *packet, int packet_size, void *src_extra,
    void *dest_extra, int size_extra);

#ifdef _EE
extern void sceSifWriteBackDCache(void *ptr, int size); // EE only
#endif
#ifdef _IOP
extern unsigned int sceSifSendCmdIntr(int cid, void *packet, int packet_size, void *src_extra,
    void *dest_extra, int size_extra, void (*completioncb)(void *userdata), void *userdata);
extern unsigned int isceSifSendCmdIntr(int cid, void *packet, int packet_size, void *src_extra,
    void *dest_extra, int size_extra, void (*completioncb)(void *userdata), void *userdata);
#endif

#ifdef __cplusplus
}
#endif

// For backwards compatibility purposes
#define SifInitCmd(...) sceSifInitCmd(__VA_ARGS__)
#define SifExitCmd(...) sceSifExitCmd(__VA_ARGS__)
#define SifGetSreg(...) sceSifGetSreg(__VA_ARGS__)
#define SifSetCmdBuffer(...) sceSifSetCmdBuffer(__VA_ARGS__)
#define SifAddCmdHandler(...) sceSifAddCmdHandler(__VA_ARGS__)
#define SifRemoveCmdHandler(...) sceSifRemoveCmdHandler(__VA_ARGS__)
#define SifSendCmd(...) sceSifSendCmd(__VA_ARGS__)
#define iSifSendCmd(...) isceSifSendCmd(__VA_ARGS__)
#define SifWriteBackDCache(...) sceSifWriteBackDCache(__VA_ARGS__)

// Send mode bits
/** Called within an interrupt context */
#define SIF_CMD_M_INTR 0x01
/** Write back D-cache for extended data */
#define SIF_CMD_M_WBDC 0x04

#endif	/* __SIFCMD_COMMON_H__ */
