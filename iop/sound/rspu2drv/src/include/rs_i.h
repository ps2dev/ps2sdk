
#ifndef _RS_I_H
#define _RS_I_H

#include <intrman.h>
#include <loadcore.h>
#include <sifman.h>
#include <sifrpc.h>
#include <string.h>
#include <tamtypes.h>
#include <thbase.h>

#include <libsnd2.h>
#include <libspu2.h>

typedef struct SpuEECBData_
{
	int mode;
	int voice_bit;
	int status;
	int opt;
} SpuEECBData;

#define sce_SPU_DEV 0x80000601
#define sce_SPUST_DEV 0x80000602
#define sce_SPUST_CB 0x80000603

extern SpuStEnv *gStPtr;
extern int gStThid;
extern SpuStEnv gStBuff;

extern void create_th(void *userdata);
extern void sce_spu2_loop(void *userdata);
extern void *spuFunc(unsigned int command, void *data, int size);
extern void sceSifCmdLoop2();
extern void DMA0CallBackProc(void);
extern void DMA1CallBackProc(void);
extern void IRQCallBackProc(void);
extern void spustCB_preparation_finished(unsigned int voice_bit, int p_status);
extern void spustCB_transfer_finished(unsigned int voice_bit, int t_status);
extern void spustCB_stream_finished(unsigned int voice_bit, int s_status);
extern void sce_spust_loop(void *userdata);

#endif
