/*      
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (C)2001, Gustavo Scotti (gustavo@scotti.com)
                           (c) Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  sifrpc.h
			EE SIF RPC commands prototypes
                        These are my findings based on debug-info elf files.
*/

#ifndef _SIFRPC_H
#define _SIFRPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sifcmd.h>

/* Modes for bind() and call() */
#define SIF_RPC_M_NOWAIT	0x01 /* Don't wait for end function */
#define SIF_RPC_M_NOWBDC	0x02 /* Don't write back the D cache */

typedef void * (*SifRpcFunc_t)(int, void *, int);
typedef void (*SifRpcEndFunc_t)(void *);

typedef struct t_SifRpcPktHeader {
	struct t_SifCmdHeader	sifcmd;
	int			rec_id;
	void			*pkt_addr;
	int			rpc_id;
} SifRpcPktHeader_t;

typedef struct t_SifRpcRendPkt
{
   struct t_SifCmdHeader	sifcmd;
   int				rec_id;		/* 04 */
   void				*pkt_addr;	/* 05 */
   int				rpc_id;		/* 06 */
                                
   struct t_SifRpcClientData	*client;	/* 7 */
   u32                          cid;		/* 8 */
   struct t_SifRpcServerData	*server;	/* 9 */
   void				*buff,		/* 10 */
      				*cbuff;		/* 11 */
} SifRpcRendPkt_t;

typedef struct t_SifRpcOtherDataPkt
{
   struct t_SifCmdHeader	sifcmd;
   int				rec_id;		/* 04 */
   void				*pkt_addr;	/* 05 */
   int				rpc_id;		/* 06 */
                                
   struct t_SifRpcReceiveData	*receive;	/* 07 */
   void				*src;		/* 08 */
   void				*dest;		/* 09 */
   int				size;		/* 10 */
} SifRpcOtherDataPkt_t;

typedef struct t_SifRpcBindPkt
{
   struct t_SifCmdHeader	sifcmd;
   int				rec_id;		/* 04 */
   void				*pkt_addr;	/* 05 */
   int				rpc_id;		/* 06 */
   struct t_SifRpcClientData	*client;	/* 07 */
   int				sid;		/* 08 */
} SifRpcBindPkt_t;

typedef struct t_SifRpcCallPkt
{
   struct t_SifCmdHeader	sifcmd;
   int				rec_id;		/* 04 */
   void				*pkt_addr;	/* 05 */
   int				rpc_id;		/* 06 */
   struct t_SifRpcClientData	*client;	/* 07 */
   int				rpc_number;	/* 08 */
   int				send_size;	/* 09 */
   void				*receive;	/* 10 */
   int				recv_size;	/* 11 */
   int				rmode;		/* 12 */
   struct t_SifRpcServerData	*server;	/* 13 */
} SifRpcCallPkt_t;

typedef struct t_SifRpcServerData
{
   int				sid;		/* 04	00 */

   SifRpcFunc_t			func;		/* 05	01 */
   void				*buff;		/* 06	02 */
   int				size;		/* 07	03 */

   SifRpcFunc_t			cfunc;		/* 08	04 */
   void				*cbuff;		/* 09	05 */
   int				size2;		/* 10	06 */

   struct t_SifRpcClientData	*client;	/* 11	07 */
   void				*pkt_addr;	/* 12	08 */
   int				rpc_number;	/* 13	09 */

   void				*receive;	/* 14	10 */
   int				rsize;		/* 15	11 */
   int				rmode;		/* 16	12 */
   int				rid;		/* 17	13 */

   struct t_SifRpcServerData	*link;		/* 18	14 */
   struct t_SifRpcServerData	*next;		/* 19	15 */
   struct t_SifRpcDataQueue	*base;		/* 20	16 */
} SifRpcServerData_t;


typedef struct t_SifRpcHeader
{
   void				*pkt_addr;	/* 04	00 */
   u32				rpc_id;		/* 05	01 */
   int				sema_id;	/* 06	02 */
   u32			 	mode;		/* 07	03 */
} SifRpcHeader_t;


typedef struct t_SifRpcClientData
{
   struct t_SifRpcHeader	hdr;
   u32				command;	/* 04 	08 */
   void				*buff, 		/* 05 	09 */
      				*cbuff;		/* 06 	10 */
   SifRpcEndFunc_t 		end_function;	/* 07 	11 */
   void				*end_param;	/* 08 	12*/
   struct t_SifRpcServerData	*server;	/* 09 	13 */
} SifRpcClientData_t;

typedef struct t_SifRpcReceiveData
{
   struct t_SifRpcHeader	hdr;
   void				*src,		/* 04 */
      				*dest;		/* 05 */
   int	                        size;		/* 06 */
} SifRpcReceiveData_t;

typedef struct t_SifRpcDataQueue
{
   int				thread_id,	/* 00 */
      				active;		/* 01 */
   struct t_SifRpcServerData	*link,		/* 02 */
      				*start,		/* 03 */
                                *end;		/* 04 */
   struct t_SifRpcDataQueue	*next;  	/* 05 */
} SifRpcDataQueue_t;

void SifInitRpc(int mode);
void SifExitRpc(void);

/* SIF RPC client API */
int SifBindRpc(SifRpcClientData_t *client, int rpc_number, int mode);
int SifCallRpc(SifRpcClientData_t *client, int rpc_number, int mode,
		void *send, int ssize, void *receive, int rsize,
		SifRpcEndFunc_t end_function, void *end_param);
int SifRpcGetOtherData(SifRpcReceiveData_t *rd, void *src, void *dest,
		int size, int mode);

int SifCheckStatRpc(SifRpcClientData_t *cd);

/* SIF RPC server API */
SifRpcDataQueue_t *
SifSetRpcQueue(SifRpcDataQueue_t *q, int thread_id);

SifRpcServerData_t *
SifRegisterRpc(SifRpcServerData_t *srv,
		int sid, SifRpcFunc_t func, void *buff, SifRpcFunc_t cfunc,
		void *cbuff, SifRpcDataQueue_t *qd);

SifRpcServerData_t *
SifGetNextRequest(SifRpcDataQueue_t *qd);

void SifExecRequest(SifRpcServerData_t *srv);
void SifRpcLoop(SifRpcDataQueue_t *q);

#ifdef __cplusplus
}
#endif

#endif
