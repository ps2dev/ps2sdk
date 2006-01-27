#ifndef __HUB_H__
#define __HUB_H__

#include "usbdpriv.h"

int removeEndpointFromDevice(Device *dev, Endpoint *ep);
int initHubDriver(void);
void flushPort(Device *dev);
int addTimerCallback(TimerCbStruct *arg, TimerCallback func, void *cbArg, uint32 delay);
int hubResetDevice(Device *dev);
int hubTimedSetFuncAddress(Device *dev);


#endif // __HUB_H__

