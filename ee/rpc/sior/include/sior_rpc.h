/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * EE-side SIO remote service.
 */

#ifndef __SIOR_RPC_H__
#define __SIOR_RPC_H__

#define	SIOR_IRX              0xC001510

#ifdef __cplusplus
extern "C" {
#endif

int SIOR_Init(int priority);

#ifdef __cplusplus
}
#endif

#endif /* __SIOR_RPC_H__ */
