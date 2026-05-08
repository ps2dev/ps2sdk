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
 * Minimal stdlib.h for IOP tcpip
 * Provides basic functions needed by lwIP on the IOP
 */

#ifndef __IOP_TCPIP_STDLIB_H__
#define __IOP_TCPIP_STDLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* atoi - convert string to integer
 * Implemented as a macro using strtol from sysclib
 */
#define atoi(x) strtol(x, NULL, 10)

/* Required for strtol */
long int strtol(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif /* __IOP_TCPIP_STDLIB_H__ */
