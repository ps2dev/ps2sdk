
#ifndef XFER_H
#define XFER_H

#include "main.h"

extern int HandleRxIntr(struct SmapDriverData *SmapDrivPrivData);
extern int HandleTxReqs(struct SmapDriverData *SmapDrivPrivData);

#endif
