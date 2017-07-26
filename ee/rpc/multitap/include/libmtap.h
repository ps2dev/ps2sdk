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
 * Functions to provide access to multi-taps.
 */

#ifndef __LIBMTAP_H__
#define __LIBMTAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise the multitap library. 
 * @return 1 on success; 1 on failure 
 */
int mtapInit(void);

/** Open a port for the multitap. 
 * @param port specifies the port that is to be monitored as a multitap connection destination. 
 * @return 1 on success; !1 on failure. 
 */
int mtapPortOpen(int port);

/** Closes a port for the multitap. 
 * @param port is a port that is to be closed (must have been previously opened by mtapPortOpen). 
 * @return 1 on success; !1 on failure. 
 */
int mtapPortClose(int port);

/** Checks if a multitap is connected to an opened port. 
 * @param port is the port to be checked. 
 * @return 1 if a multitap exists on the specified port; !1 if there isno multitap on the specified port. 
 */
int mtapGetConnection(int port);

#ifdef __cplusplus
}
#endif

#endif /* __LIBMTAP_H__ */
