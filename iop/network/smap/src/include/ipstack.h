
#ifndef IPSTACK_H
#define IPSTACK_H

extern int SMAPCommonTxPacketNext(struct SmapDriverData *SmapDrivPrivData, void **data);
extern void SMAPCommonTxPacketDeQ(struct SmapDriverData *SmapDrivPrivData, void **data);
extern void SMapCommonLinkStateDown(struct SmapDriverData *SmapDrivPrivData);
extern void SMapCommonLinkStateUp(struct SmapDriverData *SmapDrivPrivData);
extern void *SMapCommonStackAllocRxPacket(struct SmapDriverData *SmapDrivPrivData, u16 LengthRounded, void **payload);
extern void SMapStackEnQRxPacket(struct SmapDriverData *SmapDrivPrivData, void *pbuf);

#endif
