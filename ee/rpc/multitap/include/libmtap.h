/*
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (c) 2004 Nicholas Van Veen <nickvv@xtra.co.nz>
  ------------------------------------------------------------------------
  libmtap.h                  Functions to provide access to multi-taps.
*/

#ifndef _LIBMTAP_H
#define _LIBMTAP_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialise the multitap library. Returns 1 on success, other than 1 on failure.
int mtapInit(void);

// Open a port for the multitap. The "port" argument specifies the port that is to be monitored
// as a multitap connection destination. Returns 1 on success, other than 1 on failure.
int mtapPortOpen(int port);

// Closes a port for the multitap. The "port" argument is a port that is to be closed (must have
// been previously opened by mtapPortOpen. Returns 1 on success, other than 1 on failure.
int mtapPortClose(int port);

// Checks if a multitap is connected to an opened port. The "port" argument is the port to be
// checked. Returns 1 if a multitap exists on the specified port, other than 1 if there is
// no multitap on the specified port.
int mtapGetConnection(int port);

#ifdef __cplusplus
}
#endif

#endif // _LIBMTAP_H
