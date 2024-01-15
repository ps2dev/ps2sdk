/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003 Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Inter-thread message boxes.
 */

#ifndef __THMSGBX_H__
#define __THMSGBX_H__

#include <types.h>
#include <irx.h>

#ifdef __cplusplus
extern "C" {
#endif

// Message box attributes
#define MBA_THFIFO 0x000
#define MBA_THPRI  0x001
#define MBA_MSFIFO 0x000
#define MBA_MSPRI  0x004

typedef struct
{
    unsigned int attr;
    unsigned int option;
} iop_mbx_t;

/* Define your own messages by putting iop_message_t at the
 * head of your struct.
 *
 *    typedef struct {
 *        iop_message_t msg;
 *        u32    mystuff;
 *        ...
 *    } my_message_t;
 *
 *    You can then pass it as a message with
 *    SendMbx(mbxid, &struct.msg);
 */

typedef struct _iop_message
{
    struct _iop_message *next;
    unsigned char priority;
    unsigned char unused[3];
} iop_message_t;

typedef struct _iop_mbx_status
{
    unsigned int attr;
    unsigned int option;
    int numWaitThreads;
    int numMessage;
    iop_message_t *topPacket;
    int reserved[2];
} iop_mbx_status_t;

int CreateMbx(iop_mbx_t *mbx);
int DeleteMbx(int mbxid);

int SendMbx(int mbxid, void *msg);
int iSendMbx(int mbxid, void *msg);
int ReceiveMbx(void **msgvar, int mbxid);
int PollMbx(void **msgvar, int mbxid);

int ReferMbxStatus(int mbxid, iop_mbx_status_t *info);
int iReferMbxStatus(int mbxid, iop_mbx_status_t *info);

#define thmsgbx_IMPORTS_start DECLARE_IMPORT_TABLE(thmsgbx, 1, 1)
#define thmsgbx_IMPORTS_end   END_IMPORT_TABLE

#define I_CreateMbx       DECLARE_IMPORT(4, CreateMbx)
#define I_DeleteMbx       DECLARE_IMPORT(5, DeleteMbx)
#define I_SendMbx         DECLARE_IMPORT(6, SendMbx)
#define I_iSendMbx        DECLARE_IMPORT(7, iSendMbx)
#define I_ReceiveMbx      DECLARE_IMPORT(8, ReceiveMbx)
#define I_PollMbx         DECLARE_IMPORT(9, PollMbx)
#define I_ReferMbxStatus  DECLARE_IMPORT(11, ReferMbxStatus)
#define I_iReferMbxStatus DECLARE_IMPORT(12, iReferMbxStatus)

#ifdef __cplusplus
}
#endif

#endif /* __THMSGBX_H__ */
