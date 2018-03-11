#include <errno.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <sysclib.h>
#include <sysmem.h>
#include <sifman.h>
#include <thevent.h>
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

IRX_ID("Network_Manager", 2, 1);

extern struct irx_export_table _exp_netman;

int _start(int argc, char *argv[]){
	iop_sema_t sema;

	if(RegisterLibraryEntries(&_exp_netman) == 0){
		sema.attr = 0;
		sema.option = 0;
		sema.initial = 1;
		sema.max = 1;
		NetManIOSemaID = CreateSema(&sema);

		NetmanInitRPCServer();

		return MODULE_RESIDENT_END;
	}

	return MODULE_NO_RESIDENT_END;
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

void NetManNetIFXmit(void){
	if(MainNetIF != NULL)
		MainNetIF->xmit();
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

void *NetManNetProtStackAllocRxPacket(unsigned int length, void **payload){
	return IsInitialized?MainNetProtStack.AllocRxPacket(length, payload):NULL;
}

void NetManNetProtStackFreeRxPacket(void *packet){
	if(IsInitialized)
		MainNetProtStack.FreeRxPacket(packet);
}

void NetManNetProtStackEnQRxPacket(void *packet){
	if(IsInitialized)
		MainNetProtStack.EnQRxPacket(packet);
}

int NetManTxPacketNext(void **payload){
	return IsInitialized?MainNetProtStack.NextTxPacket(payload):0;
}

void NetManTxPacketDeQ(void){
	if(IsInitialized)
		MainNetProtStack.DeQTxPacket();
}

int NetManRegisterNetIF(struct NetManNetIF *NetIF){
	unsigned int i;
	int result;
	iop_event_t EventFlag;

	WaitSema(NetManIOSemaID);

	for(i=0, result=-ENOMEM; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(!(NetIFs[i].flags&NETMAN_NETIF_IN_USE)){
			if(NetIF->init()==0){
				EventFlag.attr = 0;
				EventFlag.option = 0;
				EventFlag.bits = 0;
				if((result = NetIF->EventFlagID = CreateEventFlag(&EventFlag)) >= 0){
					memcpy(&NetIFs[i], NetIF, sizeof(NetIFs[i]));
					NetIFs[i].flags|=NETMAN_NETIF_IN_USE;
					result=NetIFs[i].id=((unsigned int)NextNetIFID)<<8|i;

					UpdateNetIFStatus();
					NextNetIFID++;
				}
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
			DeleteEventFlag(NetIFs[i].EventFlagID);
			UpdateNetIFStatus();
			break;
		}
	}

	SignalSema(NetManIOSemaID);
}

void NetManToggleNetIFLinkState(int NetIFID, unsigned char state){
	struct NetManNetIF *pNetIF;

	WaitSema(NetManIOSemaID);

	if((NetIFID&0xFF)<NETMAN_MAX_NETIF_COUNT && NetIFID==NetIFs[NetIFID&0xFF].id && (NetIFs[NetIFID&0xFF].flags&NETMAN_NETIF_IN_USE)){
		pNetIF = &NetIFs[NetIFID&0xFF];

		ClearEventFlag(pNetIF->EventFlagID, ~(NETMAN_NETIF_EVF_UP|NETMAN_NETIF_EVF_DOWN));

		if(state){
			pNetIF->flags|=NETMAN_NETIF_LINK_UP;
			SetEventFlag(pNetIF->EventFlagID, NETMAN_NETIF_EVF_UP);
		}
		else{
			pNetIF->flags&=~NETMAN_NETIF_LINK_UP;
			SetEventFlag(pNetIF->EventFlagID, NETMAN_NETIF_EVF_DOWN);
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

int NetManSetLinkMode(int mode){
	int result, evfid;

	WaitSema(NetManIOSemaID);

	if(MainNetIF!=NULL){
		evfid = MainNetIF->EventFlagID;

		ClearEventFlag(evfid, ~NETMAN_NETIF_EVF_DOWN);
		result = MainNetIF->ioctl(NETMAN_NETIF_IOCTL_ETH_SET_LINK_MODE, &mode, sizeof(mode), NULL, 0);
	}
	else{
		evfid = -1;
		result = -1;
	}

	SignalSema(NetManIOSemaID);

	if(result == 0  && evfid >= 0)
		WaitEventFlag(evfid, NETMAN_NETIF_EVF_DOWN, WEF_OR, NULL);	//Wait for the IF to go down.

	return result;
}
