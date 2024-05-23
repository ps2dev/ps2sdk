/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACATA_H
#define _ACATA_H

#include <accore.h>

typedef struct ac_ata acAtaData;
typedef acAtaData *acAtaT;

typedef void (*acAtaDone)(acAtaT ata, void *arg, int ret);

typedef struct ac_ata_ops *acAtaOpsT;

struct ac_ata_h
{
	acQueueChainData a_chain;
	acAtaOpsT a_ops;
	void *a_arg;
	void *a_buf;
	acUint32 a_size;
	acUint32 a_tmout;
	acUint16 a_state;
	acUint16 a_flag;
};

struct ac_ata_ops
{
	int (*ao_command)(struct ac_ata_h *atah, int cmdpri, int pri);
	void (*ao_done)(struct ac_ata_h *atah, int result);
	int (*ao_error)(struct ac_ata_h *atah, int ret);
};

typedef acUint16 acAtaCommandData;
typedef acAtaCommandData *acAtaCommandT;

struct ac_ata
{
	struct ac_ata_h ac_h;
	acAtaDone ac_done;
	acAtaCommandData ac_command[6];
};

typedef struct ac_atapi acAtapiData;
typedef acAtapiData *acAtapiT;

typedef void (*acAtapiDone)(acAtapiT atapi, void *arg, int ret);

union ac_atapi_pkt
{
	acUint8 u_b[12];
	acUint16 u_h[6];
	acUint32 u_w[3];
};

typedef union ac_atapi_pkt acAtapiPacketData;
typedef acAtapiPacketData *acAtapiPacketT;

struct ac_atapi
{
	struct ac_ata_h ap_h;
	acAtapiDone ap_done;
	acAtapiPacketData ap_packet;
};

typedef volatile acUint16 *acAtaReg;

extern int acAtaModuleRestart(int argc, char **argv);
extern int acAtaModuleStart(int argc, char **argv);
extern int acAtaModuleStatus();
extern int acAtaModuleStop();
extern acAtaT acAtaSetup(acAtaData *ata, acAtaDone done, void *arg, unsigned int tmout);
extern int acAtaRequest(acAtaT ata, int flag, acAtaCommandData *cmd, int item, void *buf, int size);
extern int acAtaRequestI(acAtaT ata, int flag, acAtaCommandData *cmd, int item, void *buf, int size);
extern acAtaCommandData *acAtaReply(acAtaT ata);
extern int acAtaStatus(acAtaT ata);
extern acAtapiT acAtapiSetup(acAtapiData *atapi, acAtapiDone done, void *arg, unsigned int tmout);
extern int acAtapiRequest(acAtapiT atapi, int flag, acAtapiPacketData *pkt, void *buf, int size);
extern int acAtapiRequestI(acAtapiT atapi, int flag, acAtapiPacketData *pkt, void *buf, int size);
extern int acAtapiStatus(acAtapiT atapi);
extern int ata_probe(acAtaReg atareg);

#define acata_IMPORTS_start DECLARE_IMPORT_TABLE(acata, 1, 1)
#define acata_IMPORTS_end END_IMPORT_TABLE

#define I_acAtaModuleRestart DECLARE_IMPORT(4, acAtaModuleRestart)
#define I_acAtaModuleStart DECLARE_IMPORT(5, acAtaModuleStart)
#define I_acAtaModuleStatus DECLARE_IMPORT(6, acAtaModuleStatus)
#define I_acAtaModuleStop DECLARE_IMPORT(7, acAtaModuleStop)
#define I_acAtaSetup DECLARE_IMPORT(8, acAtaSetup)
#define I_acAtaRequest DECLARE_IMPORT(9, acAtaRequest)
#define I_acAtaRequestI DECLARE_IMPORT(10, acAtaRequestI)
#define I_acAtaReply DECLARE_IMPORT(11, acAtaReply)
#define I_acAtaStatus DECLARE_IMPORT(12, acAtaStatus)
#define I_acAtapiSetup DECLARE_IMPORT(13, acAtapiSetup)
#define I_acAtapiRequest DECLARE_IMPORT(14, acAtapiRequest)
#define I_acAtapiRequestI DECLARE_IMPORT(15, acAtapiRequestI)
#define I_acAtapiStatus DECLARE_IMPORT(16, acAtapiStatus)
#define I_ata_probe DECLARE_IMPORT(17, ata_probe)

#endif
