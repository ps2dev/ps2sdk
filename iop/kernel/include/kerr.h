/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Kernel Errors for the IOP
# Extracted from TESTSPU by TyRaNiD
*/

/* I am not going to explain these errors, not sure myself for most of them */
#ifndef __KERR_H__

#define KE_OK			0
#define KE_ERROR		-1
#define KE_ILLEGAL_EXPCODE	-50
#define KE_EXPHANDLER_NOUSE	-51
#define KE_EXPHANDLER_USED	-52
#define KE_ILLEGAL_CONTEXT	-100
#define KE_ILLEGAL_INTRCODE	-101
#define KE_CPUDI		-102
#define KE_INTRDISABLE		-103
#define KE_FOUND_HANDLER	-104
#define KE_NOTFOUND_HANDLER	-105
#define KE_NO_TIMER		-150
#define KE_ILLEGAL_TIMERID	-151
#define KE_UNIT_USED		-160
#define KE_UNIT_NOUSE		-161
#define KE_NO_ROMDIR		-162
#define KE_LINKERR		-200
#define KE_ILLEGAL_OBJECT	-201
#define KE_UNKNOWN_MODULE	-202
#define KE_NOFILE		-203
#define KE_FILEERR		-204
#define KE_MEMINUSE		-205
#define KE_NO_MEMORY		-400
#define KE_ILLEGAL_ATTR		-401
#define KE_ILLEGAL_ENTRY	-402
#define KE_ILLEGAL_PRIORITY	-403
#define KE_ILLEGAL_STACK_SIZE	-404
#define KE_ILLEGAL_MODE		-405
#define KE_ILLEGAL_THID		-406
#define KE_UNKNOWN_THID		-407
#define KE_UNKNOWN_SEMID	-408
#define KE_UNKNOWN_EVFID	-409
#define KE_UNKNOWN_MBXID	-410
#define KE_UNKNOWN_VPLID	-411
#define KE_UNKNOWN_FPLID	-412
#define KE_DORMANT		-413
#define KE_NOT_DORMANT		-414
#define KE_NOT_SUSPEND		-415
#define KE_NOT_WAIT		-416
#define KE_CAN_NOT_WAIT		-417
#define KE_RELEASE_WAIT		-418
#define KE_SEMA_ZERO		-419
#define KE_SEMA_OVF		-420
#define KE_EVF_COND		-421
#define KE_EVF_MULTI		-422
#define KE_EVF_ILPAT		-423
#define KE_MBOX_NOMSG		-424
#define KE_WAIT_DELETE		-425
#define KE_ILLEGAL_MEMBLOCK	-426
#define KE_ILLEGAL_MEMSIZE	-427

#endif
