#include <errno.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <sysclib.h>
#include <sysmem.h>
#include <sifman.h>
#include <thsemap.h>
#include <netman.h>
#include <netman_rpc.h>

#include "rpc_client.h"
#include "rpc_server.h"
#include "internal.h"

static struct NetManNetProtStack MainNetProtStack;
static struct NetManNetIF NetIFs[NETMAN_MAX_NETIF_COUNT];
static struct NetManNetIF *MainNetIF=NULL;
static unsigned char IsInitialized=0;
static char NIFLinkState=0;
static unsigned char NextNetIFID=0;
static int NetManIOSemaID;

IRX_ID("Network_Manager", 1, 1);

extern struct irx_export_table _exp_netman;

int _start(int argc, char *argv[]){
	iop_sema_t sema;

	sema.attr = 0;
	sema.option = 0;
	sema.initial = 1;
	sema.max = 1;
	NetManIOSemaID = CreateSema(&sema);

	NetmanInitRPCServer();
	return(RegisterLibraryEntries(&_exp_netman)!=0)?MODULE_NO_RESIDENT_END:MODULE_RESIDENT_END;
}

void *malloc(int size){
	int OldState;
	void *result;

	CpuSuspendIntr(&OldState);
	result = AllocSysMemory(ALLOC_FIRST, size, NULL);
	CpuResumeIntr(OldState);

	return result;
}

void free(void *buffer){
	int OldState;

	CpuSuspendIntr(&OldState);
	FreeSysMemory(buffer);
	CpuResumeIntr(OldState);
}

void NetManToggleGlobalNetIFLinkState(unsigned char state){
	NIFLinkState = state;

	NetManUpdateStackNIFLinkState();
}

int NetManGetGlobalNetIFLinkState(void){
	return NIFLinkState;
}

void NetManUpdateStackNIFLinkState(void){
	if(IsInitialized){
		if(NIFLinkState){
			MainNetProtStack.LinkStateUp();
		}
		else{
			MainNetProtStack.LinkStateDown();
		}
	}
}

static void UpdateNetIFStatus(void){
	unsigned char i;

	if(!(MainNetIF->flags&NETMAN_NETIF_IN_USE)) MainNetIF = NULL;

	if(MainNetIF == NULL){
		for(i=0; i<NETMAN_MAX_NETIF_COUNT; i++){
			if(NetIFs[i].flags&NETMAN_NETIF_IN_USE){
				MainNetIF=&NetIFs[i];
				break;
			}
		}
	}

	NetManToggleGlobalNetIFLinkState((MainNetIF!=NULL && (MainNetIF->flags&NETMAN_NETIF_LINK_UP)) ? 1 : 0);
}

int NetManRegisterNetworkStack(const struct NetManNetProtStack *stack){
	WaitSema(NetManIOSemaID);

	if(!IsInitialized){
		memcpy(&MainNetProtStack, stack, sizeof(MainNetProtStack));
		IsInitialized=1;

		NetManUpdateStackNIFLinkState();
	}

	SignalSema(NetManIOSemaID);

	return 0;
}

void NetManUnregisterNetworkStack(void){
	unsigned int i;

	WaitSema(NetManIOSemaID);

	for(i=0; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(NetIFs[i].flags&NETMAN_NETIF_IN_USE){
			NetIFs[i].flags=0;
			NetIFs[i].deinit();
		}
	}

	UpdateNetIFStatus();
	memset(&MainNetProtStack, 0, sizeof(MainNetProtStack));
	IsInitialized=0;

	SignalSema(NetManIOSemaID);
}

int NetManNetIFSendPacket(const void *packet, unsigned int length){
	int result;

	WaitSema(NetManIOSemaID);

	if(MainNetIF!=NULL){
		result=MainNetIF->xmit(packet, length);
	}
	else result=-1;

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length){
	int result;

	WaitSema(NetManIOSemaID);

	if(MainNetIF!=NULL){
		result=MainNetIF->ioctl(command, args, args_len, output, length);
	}
	else result=-1;

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManNetIFSetLinkMode(int mode){
	return NetManIoctl(NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE, &mode, sizeof(mode), NULL, 0);
}

struct NetManPacketBuffer *NetManNetProtStackAllocRxPacket(unsigned int length){
	return IsInitialized?MainNetProtStack.AllocRxPacket(length):NULL;
}

void NetManNetProtStackFreeRxPacket(struct NetManPacketBuffer *packet){
	if(IsInitialized) MainNetProtStack.FreeRxPacket(packet);
}

int NetManNetProtStackEnQRxPacket(struct NetManPacketBuffer *packet){
	return IsInitialized?MainNetProtStack.EnQRxPacket(packet):-1;
}

int NetManNetProtStackFlushInputQueue(void){
	return IsInitialized?MainNetProtStack.FlushInputQueue():-1;
}

int NetManRegisterNetIF(const struct NetManNetIF *NetIF){
	unsigned int i;
	int result;

	WaitSema(NetManIOSemaID);

	for(i=0, result=-ENOMEM; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(!(NetIFs[i].flags&NETMAN_NETIF_IN_USE)){
			if(NetIF->init()==0){
				memcpy(&NetIFs[i], NetIF, sizeof(NetIFs[i]));
				NetIFs[i].flags|=NETMAN_NETIF_IN_USE;
				result=NetIFs[i].id=((unsigned int)NextNetIFID)<<8|i;

				UpdateNetIFStatus();
				NextNetIFID++;
			}
			else result=-EIO;
			break;
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

void NetManUnregisterNetIF(const char *name){
	unsigned int i;

	WaitSema(NetManIOSemaID);

	for(i=0; i<NETMAN_MAX_NETIF_COUNT; i++){
		if((NetIFs[i].flags&NETMAN_NETIF_IN_USE) && strcmp(name, NetIFs[i].name)==0){
			NetIFs[i].flags=0;
			NetIFs[i].deinit();
			UpdateNetIFStatus();
			break;
		}
	}

	SignalSema(NetManIOSemaID);
}

void NetManToggleNetIFLinkState(int NetIFID, unsigned char state){
	WaitSema(NetManIOSemaID);

	if((NetIFID&0xFF)<NETMAN_MAX_NETIF_COUNT && NetIFID==NetIFs[NetIFID&0xFF].id && NetIFs[NetIFID&0xFF].flags&NETMAN_NETIF_IN_USE){
		if(state){
			NetIFs[NetIFID&0xFF].flags|=NETMAN_NETIF_LINK_UP;
		}
		else{
			NetIFs[NetIFID&0xFF].flags&=~NETMAN_NETIF_LINK_UP;
		}

		UpdateNetIFStatus();
	}

	SignalSema(NetManIOSemaID);
}

int NetManSetMainIF(const char *name){
	int result, i;

	WaitSema(NetManIOSemaID);

	for(i=0,result=-ENXIO; i<NETMAN_MAX_NETIF_COUNT; i++){
		if((NetIFs[i].flags&NETMAN_NETIF_IN_USE) && strcmp(name, NetIFs[i].name)==0){
			MainNetIF=&NetIFs[i];
			result = 0;
			break;
		}
	}

	SignalSema(NetManIOSemaID);

	return result;
}

int NetManQueryMainIF(char *name){
	int result;

	WaitSema(NetManIOSemaID);

	if(MainNetIF!=NULL && (MainNetIF->flags&NETMAN_NETIF_IN_USE)){
		strcpy(name, MainNetIF->name);
		result = 0;
	}else result = -ENXIO;

	SignalSema(NetManIOSemaID);

	return result;
}
