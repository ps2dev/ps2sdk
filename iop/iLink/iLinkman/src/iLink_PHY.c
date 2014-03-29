/*	iLink_PHY.c
 *	Purpose:	Contains the functions that are used for writing/reading values from the PHY.
 *
 *	Last Updated:	2012/02/20
 *	Programmer:	SP193
 */

#include <thbase.h>

#include "iLinkman.h"
#include "iLink_internal.h"

extern struct ILINKMemMap *ILINKRegisterBase;

#define RdPhy 0x80000000
#define WrPhy 0x40000000

static void iLinkPhySync(void){
	while(ILINKRegisterBase->PHYAccess&(RdPhy|WrPhy)){};
}

unsigned char iLinkReadPhy(unsigned char address){
	ILINKRegisterBase->PHYAccess=(((unsigned int)address)<<24)|RdPhy;
	iLinkPhySync();

	while(!(ILINKRegisterBase->intr0&iLink_INTR0_PhyRRx)){};
	ILINKRegisterBase->intr0=iLink_INTR0_PhyRRx;

	return(ILINKRegisterBase->PHYAccess&0xFF);
}

void iLinkWritePhy(unsigned char address, unsigned char data){
	ILINKRegisterBase->PHYAccess=(((unsigned int)address)<<24)|(((unsigned int)data)<<16)|WrPhy;
	iLinkPhySync();
}

void iLinkPHY_SetRootBit(int isRoot){
	unsigned short int Register01value;

	Register01value=iLinkReadPhy(1);
	if(isRoot) Register01value|=REG01_RHB;
	else Register01value&=(~REG01_RHB);

	iLinkWritePhy(1, Register01value);
}

void iLinkPHY_SetGapCount(unsigned char GapCount){
	iLinkWritePhy(1, (iLinkReadPhy(1)&0xC0)|GapCount);
}

void iLinkPHY_SetLCTRL(int LCTRL_status){
	unsigned short int Register04value;

	Register04value=iLinkReadPhy(4);
	if(LCTRL_status) Register04value|=REG04_LCTRL;
	else Register04value&=(~REG04_LCTRL);

	iLinkWritePhy(4, Register04value);
}

void iLinkPHYBusReset(void){
	iLinkWritePhy(5, iLinkReadPhy(5)|REG05_ISBR);
}
