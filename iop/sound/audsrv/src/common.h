/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2005, ps2dev - http://www.ps2dev.org
# Licenced under GNU Library General Public License version 2
#
# $Id$
# audsrv helpers.
*/

#ifndef __COMMON_H__
#define __COMMON_H__

/** Helper function to easily create threads
    @param func       thread procedure
    @param priority   thread priority (usually 40)
    @param param      optional argument for thread procedure
    @returns thread_id (int), negative on error

    Creates a thread based on the given parameter. Upon completion,
    thread is started.
*/
int create_thread(void *func, int priority, void *param);

/** Helper function to send command via SIF channel
    @param id     command id [0 .. 31]
    @param arg    optional argument
    @returns identifier for this request

    Notes: MT-unsafe
*/
int sif_send_cmd(int id, int arg);

/** Helper to print buffer in hex. Useful for debugging.
    @param ptr   pointer to buffer
    @param len   buffer length
*/
void print_hex_buffer(unsigned char *ptr, int len);

#endif
