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
# EE kernel glue and utility routines.
*/

#include "kernel.h"

#ifdef F_DIntr
int DIntr()
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;
	res = eie != 0;

	if (!eie)
		return 0;

	asm (".p2align 3");
	do {
		asm volatile ("di");
		asm volatile ("sync.p");
		asm volatile ("mfc0\t%0, $12" : "=r" (eie));
		eie &= 0x10000;
	} while (eie);

	return res;
}
#endif

#ifdef F_EIntr
int EIntr()
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;
	asm volatile ("ei");

	return eie != 0;
}
#endif

#ifdef F_EnableIntc
int EnableIntc(int intc)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _EnableIntc(intc);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_DisableIntc
int DisableIntc(int intc)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _DisableIntc(intc);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_EnableDmac
int EnableDmac(int dmac)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _EnableDmac(dmac);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_DisableDmac
int DisableDmac(int dmac)
{
	int eie, res;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	res = _DisableDmac(dmac);
	EE_SYNC();

	if (eie)
		EI();

	return res;
}
#endif

#ifdef F_iEnableIntc
int iEnableIntc(int intc)
{
	int res = _iEnableIntc(intc);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iDisableIntc
int iDisableIntc(int intc)
{
	int res = _iDisableIntc(intc);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iEnableDmac
int iEnableDmac(int dmac)
{
	int res = _iEnableDmac(dmac);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_iDisableDmac
int iDisableDmac(int dmac)
{
	int res = _iDisableDmac(dmac);
	EE_SYNC();

	return res;
}
#endif

#ifdef F_SyncDCache
void SyncDCache(void *start, void *end)
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	_SyncDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));

	if (eie)
		EI();
}
#endif

#ifdef F_iSyncDCache
void iSyncDCache(void *start, void *end)
{
	_SyncDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));
}
#endif

#ifdef F_InvalidDCache
void InvalidDCache(void *start, void *end)
{
	int eie;

	asm volatile ("mfc0\t%0, $12" : "=r" (eie));
	eie &= 0x10000;

	if (eie)
		DI();

	_InvalidDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));

	if (eie)
		EI();
}
#endif

#ifdef F_iInvalidDCache
void iInvalidDCache(void *start, void *end)
{
	_InvalidDCache((void *)((u32)start & 0xffffffc0), (void *)((u32)end & 0xffffffc0));
}
#endif
