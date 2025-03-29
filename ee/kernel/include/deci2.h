/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * DECI2 library functions
 */

#ifndef DECI2_H_
#define DECI2_H_

enum {
    DECI2_ERR_INVALID     = -1,
    DECI2_ERR_INVALSOCK   = -2,
    DECI2_ERR_ALREADYUSE  = -3,
    DECI2_ERR_MFILE       = -4,
    DECI2_ERR_INVALADDR   = -5,
    DECI2_ERR_PKTSIZE     = -6,
    DECI2_ERR_WOULDBLOCK  = -7,
    DECI2_ERR_ALREADYLOCK = -8,
    DECI2_ERR_NOTLOCKED   = -9,
    DECI2_ERR_NOROUTE     = -10,
    DECI2_ERR_NOSPACE     = -11,
    DECI2_ERR_INVALHEAD   = -12,
};

enum {
    DECI2_READ       = 1,
    DECI2_READ_DONE  = 2,
    DECI2_WRITE      = 3,
    DECI2_WRITE_DONE = 4,
    DECI2_CHSTATUS   = 5,
    DECI2_ERROR      = 6,
};

int sceDeci2Open(unsigned short protocol, void *opt, void (*handler)(int event, int param, void *opt));
int sceDeci2Close(int s);
int sceDeci2ReqSend(int s, char dest);
void sceDeci2Poll(int s);
int sceDeci2ExRecv(int s, void *buf, unsigned short len);
int sceDeci2ExSend(int s, void *buf, unsigned short len);
int sceDeci2ExReqSend(int s, char dest);
int sceDeci2ExLock(int s);
int sceDeci2ExUnLock(int s);
int kputs(char *s);

#endif // DECI2_H_
