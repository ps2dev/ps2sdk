/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
*/

/**
 * @file
 * audsrv iop rpc client.
 */

#ifndef __RPC_CLIENT_H__
#define __RPC_CLIENT_H__

void initialize_rpc_client(void);
void deinitialize_rpc_client(void);
void call_client_callback(int id);

#endif
