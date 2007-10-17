/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SIF CMD and RPC.
*/

#ifndef IOP_SIFCMD_H
#define IOP_SIFCMD_H

#include "types.h"
#include "irx.h"

#define sifcmd_IMPORTS_start DECLARE_IMPORT_TABLE(sifcmd, 1, 1)
#define sifcmd_IMPORTS_end END_IMPORT_TABLE

/* SIF command.  */

#define SYSTEM_CMD	0x80000000

typedef struct t_SifCmdHeader {
	u32	size;
	void	*dest;
	int	cid;
	u32	unknown;
} SifCmdHeader_t;

typedef void (*SifCmdHandler_t)(void *, void *);

typedef struct t_SifCmdHandlerData {
	SifCmdHandler_t handler;
	void	*harg;
} SifCmdHandlerData_t;

void sceSifInitCmd(void);
#define I_sceSifInitCmd DECLARE_IMPORT(4, sceSifInitCmd)

void sceSifExitCmd(void);
#define I_sceSifExitCmd DECLARE_IMPORT(5, sceSifExitCmd)

void sceSifGetSreg(int index);
#define I_sceSifGetSreg DECLARE_IMPORT(6, sceSifGetSreg)

void sceSifSetSreg(int index, unsigned int value);
#define I_sceSifSetSreg DECLARE_IMPORT(7, sceSifSetSreg)

void sceSifSetCmdBuffer(SifCmdHandlerData_t *cmdBuffer, int size);
#define I_sceSifSetCmdBuffer DECLARE_IMPORT(8, sceSifSetCmdBuffer)

void sceSifSetSysCmdBuffer(SifCmdHandlerData_t *sysCmdBuffer, int size);
#define I_sceSifSetSysCmdBuffer DECLARE_IMPORT(9, sceSifSetSysCmdBuffer)

void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg);
#define I_sceSifAddCmdHandler DECLARE_IMPORT(10, sceSifAddCmdHandler)


/* SIF RPC.  */

/* Modes for bind() and call() */
#define SIF_RPC_M_NOWAIT	0x01	/* Don't wait for end function */
#define SIF_RPC_M_NOWBDC	0x02	/* Don't write back the D cache */

typedef void * (*SifRpcFunc_t)(int, void *, int);
typedef void (*SifRpcEndFunc_t)(void *);

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
#define I_sceSifRemoveCmdHandler DECLARE_IMPORT(11, sceSifRemoveCmdHandler)

unsigned int sceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra, 
	void *dest_extra, int size_extra);
#define I_sceSifSendCmd DECLARE_IMPORT(12, sceSifSendCmd)

unsigned int isceSifSendCmd(int cmd, void *packet, int packet_size, void *src_extra, 
	void *dest_extra, int size_extra);
#define I_isceSifSendCmd DECLARE_IMPORT(13, isceSifSendCmd)

void sceSifInitRpc(int mode);
#define I_sceSifInitRpc DECLARE_IMPORT(14, sceSifInitRpc)

int sceSifBindRpc(SifRpcClientData_t * client, int rpc_number, int mode);
#define I_sceSifBindRpc DECLARE_IMPORT(15, sceSifBindRpc)

int sceSifCallRpc(SifRpcClientData_t * client, int rpc_number, int mode, void *send,
	int ssize, void *receive, int rsize, SifRpcEndFunc_t end_function, void *end_param);
#define I_sceSifCallRpc DECLARE_IMPORT(16, sceSifCallRpc)

void sceSifRegisterRpc(SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf,
	SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd);
#define I_sceSifRegisterRpc DECLARE_IMPORT(17, sceSifRegisterRpc)

int sceSifCheckStatRpc(SifRpcClientData_t * cd);
#define I_sceSifCheckStatRpc DECLARE_IMPORT(18, sceSifCheckStatRpc)

SifRpcDataQueue_t * sceSifSetRpcQueue(SifRpcDataQueue_t *q, int thread_id);
#define I_sceSifSetRpcQueue DECLARE_IMPORT(19, sceSifSetRpcQueue)

SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t * qd);
#define I_sceSifGetNextRequest DECLARE_IMPORT(20, sceSifGetNextRequest)

void sceSifExecRequest(SifRpcServerData_t * srv);
#define I_sceSifExecRequest DECLARE_IMPORT(21, sceSifExecRequest)

void sceSifRpcLoop(SifRpcDataQueue_t *qd);
#define I_sceSifRpcLoop DECLARE_IMPORT(22, sceSifRpcLoop)

int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode);
#define I_sceSifGetOtherData DECLARE_IMPORT(23, sceSifGetOtherData)

SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd);
#define I_sceSifRemoveRpc DECLARE_IMPORT(24, sceSifRemoveRpc)

SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd);
#define I_sceSifRemoveRpcQueue DECLARE_IMPORT(25, sceSifRemoveRpcQueue)

void sceSifSetSif1CB(void *func, int param);
#define I_sceSifSetSif1CB DECLARE_IMPORT(26, sceSifSetSif1CB)

void sceSifClearSif1CB(void);
#define I_sceSifClearSif1CB DECLARE_IMPORT(27, sceSifClearSif1CB)


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

#endif	/* IOP_SIFCMD_H */
