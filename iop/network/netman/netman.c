#include <errno.h>
#include <intrman.h>
#include <irx.h>
#include <loadcore.h>
#include <sysclib.h>
#include <sifman.h>
#include <netman.h>
#include <netman_rpc.h>

#include "rpc_server.h"
#include "internal.h"

static struct NetManNetProtStack MainNetProtStack;
static struct NetManNetIF NetIFs[NETMAN_MAX_NETIF_COUNT];
static struct NetManNetIF *MainNetIF=NULL;
static unsigned char IsInitialized=0;
static unsigned char NextNetIFID=0;

IRX_ID("Network Manager", 1, 0);

extern struct irx_export_table _exp_netman;

int _start(int argc, char *argv){
	NetmanInitRPCServer();
	return(RegisterLibraryEntries(&_exp_netman)!=0)?MODULE_NO_RESIDENT_END:MODULE_RESIDENT_END;
}

void NetManToggleGlobalNetIFLinkState(unsigned char state){
	if(IsInitialized){
		if(state){
			MainNetProtStack.LinkStateUp();
		}
		else{
			MainNetProtStack.LinkStateDown();
		}
	}
}

static void UpdateNetIFStatus(void){
	unsigned char i, HasAvailableConn;

	/* Determine whether there is a valid connection or not. */
	for(i=0, HasAvailableConn=0; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(NetIFs[i].flags&NETMAN_NETIF_IN_USE && NetIFs[i].flags&NETMAN_NETIF_LINK_UP){
			MainNetIF=&NetIFs[i];
			HasAvailableConn=1;
			break;
		}
	}

	NetManToggleGlobalNetIFLinkState(HasAvailableConn);
}

int NetManInit(const struct NetManNetProtStack *stack){
	if(!IsInitialized){
		memcpy(&MainNetProtStack, stack, sizeof(MainNetProtStack));
		IsInitialized=1;
	}

	return 0;
}

void NetManDeinit(void){
	unsigned int i;

	for(i=0; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(NetIFs[i].flags&NETMAN_NETIF_IN_USE){
			NetIFs[i].flags=0;
			NetIFs[i].deinit();
		}
	}

	UpdateNetIFStatus();
	memset(&MainNetProtStack, 0, sizeof(MainNetProtStack));
	IsInitialized=0;
}

int NetManNetIFSendPacket(const void *packet, unsigned int length){
	int result;

	if(MainNetIF!=NULL){
		result=MainNetIF->xmit(packet, length);
	}
	else result=-1;

	return result;
}

int NetManIoctl(unsigned int command, void *args, unsigned int args_len, void *output, unsigned int length){
	int result;

	if(MainNetIF!=NULL){
		result=MainNetIF->ioctl(command, args, args_len, output, length);
	}
	else result=-1;

	return result;
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

	return result;
}

void NetManUnregisterNetIF(const char *name){
	unsigned int i;

	for(i=0; i<NETMAN_MAX_NETIF_COUNT; i++){
		if(NetIFs[i].flags&NETMAN_NETIF_IN_USE && strncmp(name, NetIFs[i].name, sizeof(NetIFs[i].name))==0){
			NetIFs[i].flags=0;
			NetIFs[i].deinit();
			UpdateNetIFStatus();
			break;
		}
	}
}

void NetManToggleNetIFLinkState(int NetIFID, unsigned char state){
	if((NetIFID&0xFF)<NETMAN_MAX_NETIF_COUNT && NetIFID==NetIFs[NetIFID&0xFF].id && NetIFs[NetIFID&0xFF].flags&NETMAN_NETIF_IN_USE){
		if(state){
			NetIFs[NetIFID&0xFF].flags|=NETMAN_NETIF_LINK_UP;
		}
		else{
			NetIFs[NetIFID&0xFF].flags&=~NETMAN_NETIF_LINK_UP;
		}

		UpdateNetIFStatus();
	}
}

