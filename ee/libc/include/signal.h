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
# POSIX signal declarations.
*/
#ifndef __SIGNAL_H__

/* Incomplete. */

#if defined(__STDC__) || defined(__cplusplus)
#define SIG_DFL ((void (*)(int))0)    /* Default action */
#define SIG_IGN ((void (*)(int))1)    /* Ignore action */
#define SIG_ERR ((void (*)(int))-1)   /* Error return */
#else
#define SIG_DFL ((void (*)())0)		/* Default action */
#define SIG_IGN ((void (*)())1)		/* Ignore action */
#define SIG_ERR ((void (*)())-1)	/* Error return */
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int sig_atomic_t;
typedef void (*_sig_func_ptr) (int);

/* Undefined */
int raise(int);

#ifdef __cplusplus
}
#endif

#endif /* _SIGNAL_H_ */
