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
 * PS2IP library.
 */

#ifndef __PS2IPS_H__
#define __PS2IPS_H__

#ifndef LIBCGLUE_SYS_SOCKET_ALIASES
#define LIBCGLUE_SYS_SOCKET_ALIASES 1
#endif

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

int ps2ip_init(void);
void ps2ip_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __PS2IPS_H__ */
