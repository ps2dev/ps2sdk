/*	iLink_intr.c
 *	Purpose:	Contains the functions related to interrupt event management/response, and functions that manage IEEE1394 read/write requests and responses.
 *
 *	Last Updated:	2012/02/24
 *	Programmer:	SP193
 */

#include <dmacman.h>
#include <sysmem.h>
#include <stdio.h>
#include <thevent.h>

#include "iLinkman.h"
#include "iLink_internal.h"

extern int IntrEventFlag;
extern unsigned short int LocalNodeID;
extern int NodeCapabilities;
extern int nNodes;
extern int GenerationNumber;
extern struct ILINKMemMap *ILINKRegisterBase;
extern struct DMAChannelRegBlock *iLinkDMACRegs;
extern void (*CallBackFunction)(int reason, unsigned int offset, unsigned int size);

extern unsigned int *ConfigurationROM;
extern unsigned int ConfigurationROMSize;

unsigned int *TargetBuffer, NodeData[63];
unsigned int LocalCachedIntr0Register;

unsigned char IsBusRoot=1;	/* Assume that the console is the root, in case this variable is polled before it was set correctly. */

static void NoResponseHandler(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads){
	unsigned int i;
#ifdef DEBUG_TTY_FEEDBACK
	unsigned int data;
#endif

	DEBUG_PRINTF("NULL response handler called.\n");

	for(i=0; i<nQuads; i++){	/* Flush the buffer. */
#ifdef DEBUG_TTY_FEEDBACK
		data=*buffer;
#endif
		DEBUG_PRINTF("Unhandled data: 0x%08x.\n", data);
	}
}

static void PHYPacketHandler(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads){
	unsigned int data;

	DEBUG_PRINTF("Received PHY packet.\n");

	if(header==PHY_SELF_ID_PACKET){
		data=header;
		nQuads++;
		while(nQuads-->0){
			if(data==PHY_SELF_ID_PACKET){
				DEBUG_PRINTF("Receiving SELFID packets...\n");
				nNodes=0;
			}
			else if((data!=1)&&(data!=0xE)){
				if((data>>30)==2){	/* SELFID entry. */
					DEBUG_PRINTF("SELFID received from Node 0x%04x. Speed: 0x%02x. Power: 0x%02x.\n", SELF_ID_NODEID(data), SELF_ID_SPEED(data), SELF_ID_POWER(data));
					NodeData[nNodes]=data;
					nNodes++;
				}
				else DEBUG_PRINTF("Unknown quad received in SELF-ID phase: 0x%08x.\n", data);
			}
			else{	/* End of SELF ID packet receival. */
				/* The local node ID should be already valid at this point in execution. */
				LocalNodeID=ILINKRegisterBase->NodeID>>16;

				IsBusRoot=(ILINKRegisterBase->ctrl0&iLink_CTRL0_Root)?1:0;

				DEBUG_PRINTF("Local Node ID: 0x%04x.\n", LocalNodeID);
				SetEventFlag(IntrEventFlag, iLinkEventGotSELFIDs);
				if(CallBackFunction!=NULL) CallBackFunction(iLink_CB_BUS_RESET, 0, 0);
			}

			data=*buffer;
		}
	}
	else
	if(header==PHY_CONFIG_PACKET){
		data = *buffer;
		/* For monitoring only. It seems like the PHY will automatically configure itself with the data from the PHY packet. */
/*		if(PHY_CONFIG_R(data)) iLinkPHY_SetRootBit(PHY_CONFIG_ROOT_ID(data)==(LocalNodeID&0x3F));
		if(PHY_CONFIG_T(data)) iLinkPHY_SetGapCount(PHY_CONFIG_GAP_CNT(data)); */
		if(PHY_CONFIG_T(data)) DEBUG_PRINTF("gap count: %d.\n", PHY_CONFIG_GAP_CNT(data));
	}
	else{
		DEBUG_PRINTF("DEBUG: Unknown PHY packet type: 0x%8x.\n", header);
		NoResponseHandler(header, buffer, nQuads);
	}
}

static void ResponseHandler(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads){
	unsigned int i, HeaderLength;
	struct ieee1394_TrResponsePacketHdr ResponseData;
	unsigned char tCode;
	unsigned int data __attribute__((unused));

	tCode=(header>>4)&0xF;

	HeaderLength=(tCode!=IEEE1394_TCODE_WRITE_RESPONSE)?4:3;	/* Write response packets are shorter by other responses by one quadlet! */

	ResponseData.header=header;
	for(i=1; i<HeaderLength; i++){
		((unsigned int *)&ResponseData)[i]=*buffer;
	}

	switch(tCode){
		case IEEE1394_TCODE_WRITE_RESPONSE:
			DEBUG_PRINTF("Data write result: ");
			break;
		case IEEE1394_TCODE_READQ_RESPONSE:
			DEBUG_PRINTF("Writing to buffer %p.\n", TargetBuffer);
			DEBUG_PRINTF("Quadlet read result: ");
			*TargetBuffer=BSWAP32(ResponseData.LastField);
			break;
		case IEEE1394_TCODE_READB_RESPONSE:
			DEBUG_PRINTF("Block read result: ");
			break;
		default:
			DEBUG_PRINTF("Unknown IEE1394 transaction type 0x%02x result: ", (ResponseData.header>>10)&0x3F);
	}

	DEBUG_PRINTF("%u.\n", (ResponseData.header2>>12)&0xF);
	if(tCode==IEEE1394_TCODE_READB_RESPONSE){
	    DEBUG_PRINTF("data length: 0x%08x.\n", ResponseData.LastField>>16);	/* This refers to the data_legnth field of a block read response packet. */
	    for(i=0; i<(ResponseData.LastField>>16); i+=4){
		*TargetBuffer=*buffer;
		TargetBuffer++;
	    }
	}

	data=*buffer;	/* Read the last quadlet of the read response that contains only the speed. */
	SetEventFlag(IntrEventFlag, iLinkEventDataReceived);
}

static void WriteRequestHandler(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads){
	struct ieee1394_TrPacketHdr WriteReqHeader;
	unsigned int QuadsToRead, data;
	unsigned int i;
	unsigned char rCode, tLabel;

	DEBUG_PRINTF("Incoming write request. Header: 0x%08x; Buffer: 0x%p; nQuads: 0x%08x.\n", header, buffer, nQuads);
	rCode=0;

	/* Read in the header. */
	WriteReqHeader.header=header;
	QuadsToRead=sizeof(struct ieee1394_TrPacketHdr)>>2;
	if(((header>>4)&0xF)==IEEE1394_TCODE_WRITEQ){
		QuadsToRead--;		/* Quadlet write requests do not have the data length field. */
		WriteReqHeader.misc=4;	/* nBytes=4 */
		DEBUG_PRINTF("Quadlet");
	}
	else DEBUG_PRINTF("Block");

	for(i=1; i<QuadsToRead; i++){
		data=*buffer;
		((unsigned int *)(&WriteReqHeader))[i]=data;
	}

	WriteReqHeader.misc>>=16;

	DEBUG_PRINTF(" write request to 0x%08x %08x with length %u.\n", WriteReqHeader.offset_high, WriteReqHeader.offset_low, WriteReqHeader.misc);

	/* Using the information in the header, write the data to the destination. */
#ifdef REQ_CHECK_MEM_BOUNDARIES
	if(WriteReqHeader.offset_low<0x00200000){	/* Make sure that the write request falls within IOP memory. */
#endif
		for(i=0; i<(WriteReqHeader.misc>>2); i++){
			data=*buffer;
			((unsigned int *)WriteReqHeader.offset_low)[i]=data;
		}
#ifdef REQ_CHECK_MEM_BOUNDARIES
	}
	else{
		DEBUG_PRINTF("Invalid write request.");
		rCode=6;
	}
#endif

	data=*buffer;	/* Read in the last field that contains the speed. */

	tLabel=(header>>10)&0x3F;
	SendResponse(WriteReqHeader.offset_high>>16, (header>>16)>>6, rCode, tLabel, IEEE1394_TCODE_WRITE_RESPONSE, (data>>16)&0x7, NULL, 0);

	DEBUG_PRINTF("Response sent.\n");

	if(CallBackFunction!=NULL) CallBackFunction(iLink_CB_WRITE_REQUEST, WriteReqHeader.offset_low, WriteReqHeader.misc);
}

struct ieee1394_TrCommonRdPktHdr{
	unsigned int header;
	unsigned int offset_high;
	unsigned int offset_low;
};

struct ieee1394_TrBlockRdPacketHdr{
	struct ieee1394_TrCommonRdPktHdr header;

	unsigned int nBytes;
	unsigned int trailer;
};

struct ieee1394_TrQuadRdPacketHdr{
	struct ieee1394_TrCommonRdPktHdr header;

	unsigned int trailer;
};

static void ReadRequestHandler(unsigned int header, volatile unsigned int *buffer, unsigned int nQuads){
	unsigned int QuadsToRead, offset_low, offset_high, nBytes;
	unsigned int ReadReqHeader[5];
	unsigned int i;
	unsigned short int DestinationNodeID;
	unsigned char tCode, speed, rCode, tLabel;

	rCode=0;
	DEBUG_PRINTF("Incoming read request. Header: 0x%08x; Buffer: 0x%p; nQuads: 0x%08x.\n", header, buffer, nQuads);

	/* Read in the header. */
	ReadReqHeader[0]=header;
	tCode=(header>>4)&0xF;
	if(tCode==IEEE1394_TCODE_READQ){
		QuadsToRead=sizeof(struct ieee1394_TrQuadRdPacketHdr)>>2;
		DEBUG_PRINTF("Quadlet");
	}
	else{
		QuadsToRead=sizeof(struct ieee1394_TrBlockRdPacketHdr)>>2;
		DEBUG_PRINTF("Block");
	}

	for(i=1; i<QuadsToRead; i++){
		ReadReqHeader[i]=*buffer;
	}

	if(tCode==IEEE1394_TCODE_READB){
		offset_low=BSWAP32(((struct ieee1394_TrCommonRdPktHdr *)ReadReqHeader)->offset_low);
		offset_high=BSWAP32(((struct ieee1394_TrCommonRdPktHdr *)ReadReqHeader)->offset_high);
		DestinationNodeID=BSWAP16((unsigned short int)offset_high);
	}
	else{
		offset_low=((struct ieee1394_TrCommonRdPktHdr *)ReadReqHeader)->offset_low;
		offset_high=((struct ieee1394_TrCommonRdPktHdr *)ReadReqHeader)->offset_high;
		DestinationNodeID=(unsigned short int)(offset_high>>16);
	}

	if(tCode==IEEE1394_TCODE_READQ){
		nBytes=4;
		speed=(((struct ieee1394_TrQuadRdPacketHdr *)ReadReqHeader)->trailer>>16)&0x7;
	}
	else{
		nBytes=((struct ieee1394_TrBlockRdPacketHdr *)ReadReqHeader)->nBytes>>16;
		speed=(((struct ieee1394_TrBlockRdPacketHdr *)ReadReqHeader)->trailer>>16)&0x7;
	}

	DEBUG_PRINTF(" read request to 0x%08x %08x with length %u.\n", offset_high, offset_low, nBytes);

#ifdef REQ_CHECK_MEM_BOUNDARIES
	if(((offset_high&0x0000FFFF)==0x0000FFFF)&&((offset_low&0xF0000000)==0xF0000000)){
#endif
		offset_low&=0x0FFFFFFF;
		if((offset_low-0x400)<ConfigurationROMSize){
			offset_high=0;
			offset_low=(unsigned int)ConfigurationROM+(offset_low-0x400);
		}
#ifdef REQ_CHECK_MEM_BOUNDARIES
		else{
			DEBUG_PRINTF("Read request is beyond the range of the configuration ROM.\n");
			rCode=6;
			nBytes=0;
		}
	}
	else if((offset_low>=0x00200000)||(offset_low==0)){
		DEBUG_PRINTF("Invalid read request.");
		rCode=6;
		nBytes=0;
	}
#endif

	tCode=(tCode==IEEE1394_TCODE_READQ)?IEEE1394_TCODE_READQ_RESPONSE:IEEE1394_TCODE_READB_RESPONSE;
	tLabel=(header>>10)&0x3F;
	SendResponse(DestinationNodeID, DestinationNodeID>>6, rCode, tLabel, tCode, speed, (unsigned int *)offset_low, nBytes>>2);

	DEBUG_PRINTF("Response sent!2.\n");

	if(CallBackFunction!=NULL) CallBackFunction(iLink_CB_READ_REQUEST, offset_low, nBytes);
}

const void *RequestHandlers[]={
	&WriteRequestHandler,	/*  0: write quadlet         */
	&WriteRequestHandler,	/*  1: write block           */
	&ResponseHandler,	/*  2: write response        */
	&NoResponseHandler,	/*  3: reserved              */
	&ReadRequestHandler,	/*  4: read quadlet          */
	&ReadRequestHandler,	/*  5: read block            */
	&ResponseHandler,	/*  6: read quadlet response */
	&ResponseHandler,	/*  7: read block response   */
	&NoResponseHandler,	/*  8: cycle start           */
	&NoResponseHandler,	/*  9: lock request          */
	&NoResponseHandler,	/* 10: ISO/stream data       */
	&NoResponseHandler,	/* 11: lock response         */
	&NoResponseHandler,	/* 12: reserved              */
	&NoResponseHandler,	/* 13: reserved              */
	&PHYPacketHandler,	/* 14: PHY packet            */
	&NoResponseHandler	/* 15: reserved              */
};

int iLinkIntrHandler(void *arg){
	unsigned int LocalCachedIntr1Register, EventFlagBitsToSet;
#ifdef REQ_CHECK_DMAC_STAT
	unsigned int i;
#endif

	while(1){
		/* Acknowledge interrupts while determining which interrupt events have occurred. Some events need to be acknowledged as early as possible or they will prevent the hardware from working properly. */
		ILINKRegisterBase->intr0=LocalCachedIntr0Register=ILINKRegisterBase->intr0&ILINKRegisterBase->intr0Mask;
		ILINKRegisterBase->intr1=LocalCachedIntr1Register=ILINKRegisterBase->intr1&ILINKRegisterBase->intr1Mask;

		if(!LocalCachedIntr0Register && !LocalCachedIntr1Register) break;

		EventFlagBitsToSet=0;

		DEBUG_PRINTF("iLink interrupt: 0x%08x 0x%08x\n", LocalCachedIntr0Register, LocalCachedIntr1Register);

		if(LocalCachedIntr1Register&iLink_INTR1_UTD) EventFlagBitsToSet|=iLinkEventDataSent;

#ifdef REQ_CHECK_DMAC_STAT
		if(LocalCachedIntr0Register&iLink_INTR0_DRFR){
			iDEBUG_PRINTF("Handling DBUF FIFO data...\n");

			for(i=0; i<3; i++){
				iDEBUG_PRINTF("DMA Channel %d CHCR: 0x%08x MADR: 0x%08x DLEN: 0x%05x SLICE: 0x%03x\n", 13+i, iLinkDMACRegs[i].chcr, iLinkDMACRegs[i].madr, iLinkDMACRegs[i].dlen, iLinkDMACRegs[i].slice);

				if((i>0) && !(iLinkDMACRegs[i].chcr&DMAC_CHCR_AR)){
					iLinkDMACRegs[i].chcr=0;
					switch(i){
						case 1:
							ILINKRegisterBase->dmaCtrlSR0=0;
							break;
						case 2:
							ILINKRegisterBase->dmaCtrlSR1=0;
							break;
					}

					iDEBUG_PRINTF("DMAC Channel %d stalled. Flushing FIFOs and reseting PHTs.\n", 13+i);

					iLinkInitPHT();
				}
			}
		}
#endif

		if(LocalCachedIntr0Register&iLink_INTR0_PhyRst){
			iDEBUG_PRINTF("-=Bus reset start detected=-\n");

			GenerationNumber++;

			iLinkInitPHT();
			EventFlagBitsToSet|=(iLinkEventBusReady|iLinkEventBusReset);

			iDEBUG_PRINTF("-=PHTs initialized=-\n");
		}

		if(LocalCachedIntr0Register&iLink_INTR0_URx){
			EventFlagBitsToSet|=iLinkEventURx;
		}

		if(LocalCachedIntr0Register&(iLink_INTR0_InvAck|iLink_INTR0_RetEx|iLink_INTR0_UResp)){
			EventFlagBitsToSet|=iLinkEventError;
			iDEBUG_PRINTF("-=Missing/invalid response detected=-\n");
		}

		/* For debugging purposes */
#ifdef REQ_CHECK_ERRORS
		if(LocalCachedIntr0Register&iLink_INTR0_HdrErr){
			iDEBUG_PRINTF("-=HdrErr=-\n");
		}

		if(LocalCachedIntr0Register&iLink_INTR0_SntBsyAck){
			iDEBUG_PRINTF("-=iLink_INTR0_SntBsyAck=-\n");
		}
#endif

		if(LocalCachedIntr0Register&iLink_INTR0_PBCntR){
			EventFlagBitsToSet|=iLinkEventDMATransEnd;
			iDEBUG_PRINTF("-=End of DMA transfer=-\n");
		}

		if(LocalCachedIntr0Register&iLink_INTR0_CmdRst){
			iDEBUG_PRINTF("-=CmdRst=-\n");
			iLinkPHYBusReset();
		}

		iSetEventFlag(IntrEventFlag, EventFlagBitsToSet);
	}

	return 1;
}
