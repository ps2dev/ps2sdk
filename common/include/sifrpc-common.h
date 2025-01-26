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
 * Common defintions for SIF RPC.
 */

#ifndef __SIFRPC_COMMON_H__
#define __SIFRPC_COMMON_H__

#include <sifcmd-common.h>

/* Modes for bind() and call() */
/** Don't wait for end function */
#define SIF_RPC_M_NOWAIT 0x01
/** Don't write back the D cache */
#define SIF_RPC_M_NOWBDC 0x02

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*SifRpcFunc_t)(int fno, void *buffer, int length);
typedef void (*SifRpcEndFunc_t)(void *end_param);

#ifdef __cplusplus
}
#endif

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

    struct t_SifRpcClientData *cd;     /* 7 */
    u32 cid;                           /* 8 */
    struct t_SifRpcServerData *sd;     /* 9 */
    void *buf,                         /* 10 */
        *cbuf;                         /* 11 */
} SifRpcRendPkt_t;

typedef struct t_SifRpcOtherDataPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;     /* 04 */
    void *pkt_addr; /* 05 */
    int rpc_id;     /* 06 */

    struct t_SifRpcReceiveData *recvbuf; /* 07 */
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
    struct t_SifRpcClientData *cd;     /* 07 */
    int sid;                           /* 08 */
} SifRpcBindPkt_t;

typedef struct t_SifRpcCallPkt
{
    struct t_SifCmdHeader sifcmd;
    int rec_id;                        /* 04 */
    void *pkt_addr;                    /* 05 */
    int rpc_id;                        /* 06 */
    struct t_SifRpcClientData *cd;     /* 07 */
    int rpc_number;                    /* 08 */
    int send_size;                     /* 09 */
    void *recvbuf;                     /* 10 */
    int recv_size;                     /* 11 */
    int rmode;                         /* 12 */
    struct t_SifRpcServerData *sd;     /* 13 */
} SifRpcCallPkt_t;

typedef struct t_SifRpcServerData
{
    int sid; /* 04	00 */

    SifRpcFunc_t func; /* 05	01 */
    void *buf;         /* 06	02 */
    int size;          /* 07	03 */

    SifRpcFunc_t cfunc; /* 08	04 */
    void *cbuf;         /* 09	05 */
    int size2;          /* 10	06 */

    struct t_SifRpcClientData *client; /* 11	07 */
    void *pkt_addr;                    /* 12	08 */
    int rpc_number;                    /* 13	09 */

    void *recvbuf; /* 14	10 */
    int rsize;     /* 15	11 */
    int rmode;     /* 16	12 */
    int rid;       /* 17	13 */

    struct t_SifRpcServerData *link; /* 18	14 */
    struct t_SifRpcServerData *next; /* 19	15 */
    struct t_SifRpcDataQueue *base;  /* 20	16 */
} SifRpcServerData_t;


typedef struct t_SifRpcHeader
{
    void *pkt_addr; /* 04	00 */
    u32 rpc_id;     /* 05	01 */
    int sema_id;    /* 06	02 */
    u32 mode;       /* 07	03 */
} SifRpcHeader_t;


typedef struct t_SifRpcClientData
{
    struct t_SifRpcHeader hdr;
    u32 command;                       /* 04 	08 */
    void *buf,                         /* 05 	09 */
        *cbuf;                         /* 06 	10 */
    SifRpcEndFunc_t end_function;      /* 07 	11 */
    void *end_param;                   /* 08 	12 */
    struct t_SifRpcServerData *server; /* 09 	13 */
} SifRpcClientData_t;

typedef struct t_SifRpcReceiveData
{
    struct t_SifRpcHeader hdr;
    void *src, /* 04 */
        *dest; /* 05 */
    int size;  /* 06 */
} SifRpcReceiveData_t;

typedef struct t_SifRpcDataQueue
{
    int thread_id,                   /* 00 */
        active;                      /* 01 */
    struct t_SifRpcServerData *link, /* 02 */
        *start,                      /* 03 */
        *end;                        /* 04 */
    struct t_SifRpcDataQueue *next;  /* 05 */
} SifRpcDataQueue_t;

#ifdef __cplusplus
extern "C" {
#endif

extern void sceSifInitRpc(int mode);
#ifdef _EE
extern void sceSifExitRpc(void);
#endif

/* SIF RPC client API */
extern int sceSifBindRpc(SifRpcClientData_t *cd, int sid, int mode);
extern int sceSifCallRpc(SifRpcClientData_t *cd, int rpc_number, int mode, void *sendbuf,
    int ssize, void *recvbuf, int rsize, SifRpcEndFunc_t end_function, void *end_param);

extern void sceSifRegisterRpc(SifRpcServerData_t *sd, int sid, SifRpcFunc_t func, void *buf,
    SifRpcFunc_t cfunc, void *cbuf, SifRpcDataQueue_t *qd);

extern int sceSifCheckStatRpc(SifRpcClientData_t *cd);

/* SIF RPC server API */
extern void sceSifSetRpcQueue(SifRpcDataQueue_t *qd, int thread_id);
extern SifRpcServerData_t *sceSifGetNextRequest(SifRpcDataQueue_t *qd);
extern void sceSifExecRequest(SifRpcServerData_t *sd);
extern void sceSifRpcLoop(SifRpcDataQueue_t *qd);

extern int sceSifGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest, int size, int mode);

extern SifRpcServerData_t *sceSifRemoveRpc(SifRpcServerData_t *sd, SifRpcDataQueue_t *qd);
extern SifRpcDataQueue_t *sceSifRemoveRpcQueue(SifRpcDataQueue_t *qd);
#ifdef _IOP
extern void sceSifSetSif1CB(void (*func)(void *userdata), void *userdata);
extern void sceSifClearSif1CB(void);
#endif

#ifdef __cplusplus
}
#endif

// For backwards compatibility purposes
#define SifInitRpc(...) sceSifInitRpc(__VA_ARGS__)
#define SifExitRpc(...) sceSifExitRpc(__VA_ARGS__)
#define SifBindRpc(...) sceSifBindRpc(__VA_ARGS__)
#define SifCallRpc(...) sceSifCallRpc(__VA_ARGS__)
#define SifRegisterRpc(...) sceSifRegisterRpc(__VA_ARGS__)
#define SifCheckStatRpc(...) sceSifCheckStatRpc(__VA_ARGS__)
#define SifSetRpcQueue(...) sceSifSetRpcQueue(__VA_ARGS__)
#define SifGetNextRequest(...) sceSifGetNextRequest(__VA_ARGS__)
#define SifExecRequest(...) sceSifExecRequest(__VA_ARGS__)
#define SifRpcLoop(...) sceSifRpcLoop(__VA_ARGS__)
#define SifRpcGetOtherData(...) sceSifGetOtherData(__VA_ARGS__)
#define SifRemoveRpc(...) sceSifRemoveRpc(__VA_ARGS__)
#define SifRemoveRpcQueue(...) sceSifRemoveRpcQueue(__VA_ARGS__)

#endif	/* __SIFRPC_COMMON_H__ */
