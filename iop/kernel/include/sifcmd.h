/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SIF CMD and RPC.
 */

#ifndef __SIFCMD_H__
#define __SIFCMD_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SIF command.  */
typedef struct t_SifCmdHeader {
	/** Packet size. Min: 1x16 (header only), max: 7*16 */
	u32	psize:8;
	/** Extra data size */
	u32	dsize:24;
	/** Extra data destination address. May be NULL if there's no extra data. */
	void	*dest;
	/** Function number of the function to call. */
	int	cid;
	/** Can be freely used. */
	u32	opt;
} SifCmdHeader_t;

/* System functions */
#define	SIF_CMD_ID_SYSTEM	0x80000000

#define SIF_CMD_CHANGE_SADDR (SIF_CMD_ID_SYSTEM | 0)
#define SIF_CMD_SET_SREG	 (SIF_CMD_ID_SYSTEM | 1)
#define SIF_CMD_INIT_CMD	 (SIF_CMD_ID_SYSTEM | 2)
#define SIF_CMD_RESET_CMD	 (SIF_CMD_ID_SYSTEM | 3)
#define SIF_CMD_RPC_END		 (SIF_CMD_ID_SYSTEM | 8)
#define SIF_CMD_RPC_BIND	 (SIF_CMD_ID_SYSTEM | 9)
#define SIF_CMD_RPC_CALL	 (SIF_CMD_ID_SYSTEM | 10)
#define SIF_CMD_RPC_RDATA	 (SIF_CMD_ID_SYSTEM | 12)

//System SREG
#define SIF_SREG_RPCINIT	0

/** Structure for remotely (over the SIF) changing the value of a software register (SREG).
 * There are 32 software registers (0 - 31). Registers 0-7 are used by the system.
 */
typedef struct t_SifCmdSRegData {
	SifCmdHeader_t	header;
	int		index;
	unsigned int	value;
} SifCmdSRegData_t;

typedef void (*SifCmdHandler_t)(void *data, void *harg);

typedef struct t_SifCmdHandlerData {
	SifCmdHandler_t handler;
	void		*harg;
} SifCmdHandlerData_t;

typedef struct t_SifCmdSysHandlerData
{
    SifCmdHandler_t handler;
    void *harg;
    void *unknown08;
} SifCmdSysHandlerData_t;

void sceSifInitCmd(void);
void sceSifExitCmd(void);
unsigned int sceSifGetSreg(int index);
void sceSifSetSreg(int index, unsigned int value);
void sceSifSetCmdBuffer(SifCmdHandlerData_t *cmdBuffer, int size);
void sceSifSetSysCmdBuffer(SifCmdSysHandlerData_t *sysCmdBuffer, int size);
void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg);

/* SIF RPC.  */

/* Modes for bind() and call() */
/** Don't wait for end function */
#define SIF_RPC_M_NOWAIT	0x01	
/** Don't write back the D cache */
#define SIF_RPC_M_NOWBDC	0x02	

typedef void * (*SifRpcFunc_t)(int fno, void *buffer, int length);
typedef void (*SifRpcEndFunc_t)(void *end_param);

typedef struct t_SifRpcPktHeader
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;
    void *pkt_addr;
    int rpc_id;
} SifRpcPktHeader_t;

typedef struct t_SifRpcRendPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;     /* 04 */
    void *pkt_addr; /* 05 */
    int rpc_id;     /* 06 */

    struct t_SifRpcClientData *client; /* 7 */
    u32 cid;                           /* 8 */
    struct t_SifRpcServerData *server; /* 9 */
    void *buff,                        /* 10 */
        *cbuff;                        /* 11 */
} SifRpcRendPkt_t;

typedef struct t_SifRpcOtherDataPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;     /* 04 */
    void *pkt_addr; /* 05 */
    int rpc_id;     /* 06 */

    struct t_SifRpcReceiveData *receive; /* 07 */
    void *src;                           /* 08 */
    void *dest;                          /* 09 */
    int size;                            /* 10 */
} SifRpcOtherDataPkt_t;

typedef struct t_SifRpcBindPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;                        /* 04 */
    void *pkt_addr;                    /* 05 */
    int rpc_id;                        /* 06 */
    struct t_SifRpcClientData *client; /* 07 */
    int sid;                           /* 08 */
} SifRpcBindPkt_t;

typedef struct t_SifRpcCallPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;                        /* 04 */
    void *pkt_addr;                    /* 05 */
    int rpc_id;                        /* 06 */
    struct t_SifRpcClientData *client; /* 07 */
    int rpc_number;                    /* 08 */
    int send_size;                     /* 09 */
    void *receive;                     /* 10 */
    int recv_size;                     /* 11 */
    int rmode;                         /* 12 */
    struct t_SifRpcServerData *server; /* 13 */
} SifRpcCallPkt_t;

typedef struct t_SifRpcServerData {
	int	sid;

	SifRpcFunc_t func;
	void	*buff;
	int	size;

	SifRpcFunc_t cfunc;
	void	*cbuff;
	int	size2;

	struct t_SifRpcClientData *client;
	void	*pkt_addr;
	int	rpc_number;

	void	*receive;
	int	rsize;
	int	rmode;
	int	rid;

	struct t_SifRpcServerData *link;
	struct t_SifRpcServerData *next;
	struct t_SifRpcDataQueue *base;
} SifRpcServerData_t;

typedef struct t_SifRpcHeader {
	void	*pkt_addr;
	u32	rpc_id;
	int	sema_id;
	u32	mode;
} SifRpcHeader_t;

typedef struct t_SifRpcClientData {
	struct t_SifRpcHeader hdr;
	u32	command;
	void	*buff;
	void	*cbuff;
	SifRpcEndFunc_t end_function;
	void	*end_param;
	struct t_SifRpcServerData *server;
} SifRpcClientData_t;

typedef struct t_SifRpcReceiveData {
	struct t_SifRpcHeader hdr;
	void	*src;
	void	*dest;
	int	size;
} SifRpcReceiveData_t;

typedef struct t_SifRpcDataQueue {
	int	thread_id;
	int	active;
	struct t_SifRpcServerData *link;
	struct t_SifRpcServerData *start;
	struct t_SifRpcServerData *end;
	struct t_SifRpcDataQueue *next;
} SifRpcDataQueue_t;


void sceSifRemoveCmdHandler(int cid);
unsigned int sceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra,
	void *dest_extra, int size_extra);
unsigned int isceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra,
	void *dest_extra, int size_extra);

void sceSifInitRpc(int mode);
int sceSifBindRpc(SifRpcClientData_t * client, int rpc_number, int mode);
int sceSifCallRpc(SifRpcClientData_t * client, int rpc_number, int mode, void *send,
	int ssize, void *receive, int rsize, SifRpcEndFunc_t end_function, void *end_param);

void sceSifRegisterRpc(SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf,
	SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd);

int sceSifCheckStatRpc(SifRpcClientData_t * cd);

SifRpcDataQueue_t * sceSifSetRpcQueue(SifRpcDataQueue_t *q, int thread_id);
SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t * qd);
void sceSifExecRequest(SifRpcServerData_t * srv);
void sceSifRpcLoop(SifRpcDataQueue_t *qd);

int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode);

SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd);
SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd);
void sceSifSetSif1CB(void (*func)(void *userdata), void *userdata);
void sceSifClearSif1CB(void);
unsigned int sceSifSendCmdIntr(int cmd, void *packet, int packet_size, void *src_extra,
	void *dest_extra, int size_extra, void (*completioncb)(void *userdata), void *userdata);
unsigned int isceSifSendCmdIntr(int cmd, void *packet, int packet_size, void *src_extra,
	void *dest_extra, int size_extra, void (*completioncb)(void *userdata), void *userdata);

/* Compatibility names for use with ps2lib.  */
#define SifInitRpc sceSifInitRpc
#define SifBindRpc sceSifBindRpc
#define SifCallRpc sceSifCallRpc

#define SifRegisterRpc sceSifRegisterRpc

#define SifCheckStatRpc sceSifCheckStatRpc

#define SifSetRpcQueue sceSifSetRpcQueue
#define SifGetNextRequest sceSifGetNextRequest
#define SifExecRequest sceSifExecRequest
#define SifRpcLoop sceSifRpcLoop

#define SifRpcGetOtherData sceSifGetOtherData

#define sifcmd_IMPORTS_start DECLARE_IMPORT_TABLE(sifcmd, 1, 1)
#define sifcmd_IMPORTS_end END_IMPORT_TABLE

#define I_sceSifInitCmd DECLARE_IMPORT(4, sceSifInitCmd)
#define I_sceSifExitCmd DECLARE_IMPORT(5, sceSifExitCmd)
#define I_sceSifGetSreg DECLARE_IMPORT(6, sceSifGetSreg)
#define I_sceSifSetSreg DECLARE_IMPORT(7, sceSifSetSreg)
#define I_sceSifSetCmdBuffer DECLARE_IMPORT(8, sceSifSetCmdBuffer)
#define I_sceSifSetSysCmdBuffer DECLARE_IMPORT(9, sceSifSetSysCmdBuffer)
#define I_sceSifAddCmdHandler DECLARE_IMPORT(10, sceSifAddCmdHandler)
#define I_sceSifRemoveCmdHandler DECLARE_IMPORT(11, sceSifRemoveCmdHandler)
#define I_sceSifSendCmd DECLARE_IMPORT(12, sceSifSendCmd)
#define I_isceSifSendCmd DECLARE_IMPORT(13, isceSifSendCmd)
#define I_sceSifInitRpc DECLARE_IMPORT(14, sceSifInitRpc)
#define I_sceSifBindRpc DECLARE_IMPORT(15, sceSifBindRpc)
#define I_sceSifCallRpc DECLARE_IMPORT(16, sceSifCallRpc)
#define I_sceSifRegisterRpc DECLARE_IMPORT(17, sceSifRegisterRpc)
#define I_sceSifCheckStatRpc DECLARE_IMPORT(18, sceSifCheckStatRpc)
#define I_sceSifSetRpcQueue DECLARE_IMPORT(19, sceSifSetRpcQueue)
#define I_sceSifGetNextRequest DECLARE_IMPORT(20, sceSifGetNextRequest)
#define I_sceSifExecRequest DECLARE_IMPORT(21, sceSifExecRequest)
#define I_sceSifRpcLoop DECLARE_IMPORT(22, sceSifRpcLoop)
#define I_sceSifGetOtherData DECLARE_IMPORT(23, sceSifGetOtherData)
#define I_sceSifRemoveRpc DECLARE_IMPORT(24, sceSifRemoveRpc)
#define I_sceSifRemoveRpcQueue DECLARE_IMPORT(25, sceSifRemoveRpcQueue)
#define I_sceSifSetSif1CB DECLARE_IMPORT(26, sceSifSetSif1CB)
#define I_sceSifClearSif1CB DECLARE_IMPORT(27, sceSifClearSif1CB)
#define I_sceSifSendCmdIntr DECLARE_IMPORT(28, sceSifSendCmdIntr)
#define I_isceSifSendCmdIntr DECLARE_IMPORT(29, isceSifSendCmdIntr)

#ifdef __cplusplus
}
#endif

#endif	/* __SIFCMD_H__ */
