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
 * DECI2 TTY
 */

#include "kernel.h"
#include "deci2.h"

#define READ_ONCE(x)       (*(const volatile typeof(x) *)(&x))
#define WRITE_ONCE(x, val) (*(volatile typeof(x) *)(&x) = (val))

int ttyinit = 0;

struct tty_packet
{
    /* standard deci2 header */
    u16 length;
    u16 reserved;
    u16 protocol;
    u8 source;
    u8 destination;

    /* TTYP */
    u32 ttyp_reserved;
    char buf[256];
};

static struct tty_packet wbuf __attribute__((aligned(16)));
static struct tty_packet rbuf __attribute__((aligned(16)));

struct tty_queue
{
    u32 read;
    u32 write;
    char buf[256];
};

struct tinfo
{
    int socket;
    int wlen;
    int rlen;
    int writing;
    char *wptr;
    char *rptr;
    struct tty_queue *read_queue;
} tinfo;

static struct tty_queue *QueueInit()
{
    static struct tty_queue q;

    q.read  = 0;
    q.write = 0;

    return &q;
}

/* Attempt at safe spsc fifo without disabling interrupts
 * User thread spins on QueueEmpty waiting for Handler to push data */

static int QueueEmpty(struct tty_queue *q)
{
    return READ_ONCE(q->read) == READ_ONCE(q->write);
}

static void QueuePush(struct tty_queue *q, char c)
{
    int w = READ_ONCE(q->write);

    q->buf[w & 0xff] = c;
    WRITE_ONCE(q->write, w + 1);
}

static char QueuePop(struct tty_queue *q)
{
    int r = READ_ONCE(q->read);
    char ret;

    ret = q->buf[r & 0xff];
    WRITE_ONCE(q->read, r + 1);

    return ret;
}

void sceTtyHandler(int event, int param, void *opt)
{
    struct tinfo *tinfo = opt;
    struct tty_packet *pkt;
    int ret;

    switch (event) {
        case DECI2_READ:
        case DECI2_READ_DONE:
            if (param) {
                ret = sceDeci2ExRecv(tinfo->socket, tinfo->rptr + tinfo->rlen, param);
                tinfo->rlen += ret;
                break;
            }

            pkt = (struct tty_packet *)tinfo->rptr;
            if (pkt->length > offsetof(struct tty_packet, buf)) {
                for (int i = 0; i < pkt->length - offsetof(struct tty_packet, buf); i++) {
                    QueuePush(tinfo->read_queue, pkt->buf[i]);
                }
            }

            tinfo->rlen = 0;
            break;
        case DECI2_WRITE:
            ret = sceDeci2ExSend(tinfo->socket, tinfo->wptr, tinfo->wlen);
            if (ret < 0) {
                tinfo->writing = 0;
                break;
            }

            tinfo->wptr += ret;
            tinfo->wlen -= ret;
            break;
        case DECI2_WRITE_DONE:
            tinfo->writing = 0;
            break;
    }
}

int sceTtyWrite(char *buf, int len)
{
    struct tty_packet *pkt;
    int oldintr, ret, ssize;
    char *wp;

    if (tinfo.writing) {
        return -1;
    }

    oldintr = DIntr();

    tinfo.writing = 1;
    tinfo.wptr    = UNCACHED_SEG(&wbuf);
    pkt           = UNCACHED_SEG(&wbuf);

    ssize = 0;
    wp    = pkt->buf;
    for (ret = 0; ret < len; ret++) {
        if (*buf == '\n') {
            *wp++ = '\r';
            ssize++;
            if (ssize >= 256)
                break;
        }

        *wp++ = *buf++;
        ssize++;
        if (ssize >= 256)
            break;
    }

    tinfo.wlen  = ssize + offsetof(struct tty_packet, buf);
    pkt->length = tinfo.wlen;
    if (sceDeci2ReqSend(tinfo.socket, pkt->destination) < 0) {
        tinfo.writing = 0;
        if (oldintr) {
            EIntr();
        }

        return -1;
    }

    while (READ_ONCE(tinfo.writing)) {
        sceDeci2Poll(tinfo.socket);
    }

    if (oldintr) {
        EIntr();
    }

    return ret;
}


int sceTtyRead(char *buf, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        while (QueueEmpty(tinfo.read_queue))
            ;

        buf[i] = QueuePop(tinfo.read_queue);
        if (buf[i] == '\n' || buf[i] == '\r') {
            break;
        }
    }

    return i;
}

int sceTtyInit()
{
    struct tty_packet *pkt;

    FlushCache(0);
    tinfo.socket = sceDeci2Open(0x210, &tinfo, sceTtyHandler);
    if (tinfo.socket < 0) {
        return 0;
    }

    tinfo.writing = 0;
    tinfo.wlen    = 0;
    tinfo.rptr    = UNCACHED_SEG(&rbuf);
    tinfo.wptr    = UNCACHED_SEG(&wbuf);

    pkt              = UNCACHED_SEG(&wbuf);
    pkt->protocol    = 0x210;
    pkt->source      = 'E';
    pkt->destination = 'H';

    pkt->reserved      = 0;
    pkt->ttyp_reserved = 0;

    tinfo.read_queue = QueueInit();

    return 1;
}
