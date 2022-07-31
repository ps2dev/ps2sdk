
#ifndef XFER_H
#define XFER_H

#include "main.h"

int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData);
int HandleTxReqs(struct SmapDriverData *SmapDrivPrivData);

#endif
