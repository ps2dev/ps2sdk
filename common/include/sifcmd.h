/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (C)2001, Gustavo Scotti (gustavo@scotti.com)
# (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# EE SIF commands prototypes
*/

#ifndef _SIFCMD_H
#define _SIFCMD_H

#ifdef __cplusplus
extern "C" {
#endif


#define SYSTEM_CMD	0x80000000

typedef struct t_SifCmdHeader
{
   u32				size;
   void				*dest;
   int				cid;
   u32				unknown;
} SifCmdHeader_t;

typedef struct t_SifCmdHandlerData
{
   void     		(*handler)	( void *a, void *b);
   void	 			*harg;
} SifCmdHandlerData_t;

typedef void (*SifCmdHandler_t)(void *, void *);

u32	SifSendCmd( int, void *, int, void *, void *, int);
u32	iSifSendCmd( int, void *, int, void *, void *, int);
void SifAddCmdHandler( int, void (*)( void *, void *), void *);
void SifInitCmd(void);
void SifExitCmd(void);
int	SifGetSreg( int);

#ifdef __cplusplus
}
#endif

#endif
