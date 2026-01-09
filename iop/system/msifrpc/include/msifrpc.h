/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _MSIFRPC_H
#define _MSIFRPC_H

typedef struct _sifm_rpc_data
{
	void *paddr;
	unsigned int pid;
	int tid;
	unsigned int mode;
} sceSifMRpcData;

typedef void (*sceSifMEndFunc)(void *end_param);

typedef struct _sifm_client_data
{
	sceSifMRpcData rpcd;
	int command;
	void *buff;
	void *cbuff;
	sceSifMEndFunc func;
	void *para;
	struct _sifm_serve_data *serve;
} sceSifMClientData;

typedef void *(*sceSifMRpcFunc)(unsigned int fno, void *buffer, int length);

typedef struct _sifm_serve_data
{
	void *func_buff;
	int size;
	void *cfunc_buff;
	int csize;
	sceSifMClientData *client;
	void *paddr;
	unsigned int fno;
	void *receive;
	int rsize;
	int rmode;
	unsigned int rid;
	struct _sifm_serve_data *link;
	struct _sifm_serve_data *next;
	struct _sifm_queue_data *base;
	struct _sifm_serve_entry *sentry;
} sceSifMServeData;

typedef struct _sifm_serve_entry
{
	unsigned int mbxid;
	int command;
	sceSifMRpcFunc func;
	sceSifMRpcFunc cfunc;
	sceSifMServeData *serve_list;
	struct _sifm_serve_entry *next;
} sceSifMServeEntry;

extern void sceSifMInitRpc(unsigned int mode);
extern int sceSifMTermRpc(int request, int flags);
extern void sceSifMEntryLoop(sceSifMServeEntry *se, int request, sceSifMRpcFunc func, sceSifMRpcFunc cfunc);

#define msifrpc_IMPORTS_start DECLARE_IMPORT_TABLE(msifrpc, 1, 2)
#define msifrpc_IMPORTS_end END_IMPORT_TABLE

#define I_sceSifMInitRpc DECLARE_IMPORT(4, sceSifMInitRpc)
#define I_sceSifMTermRpc DECLARE_IMPORT(16, sceSifMTermRpc)
#define I_sceSifMEntryLoop DECLARE_IMPORT(17, sceSifMEntryLoop)

#endif
