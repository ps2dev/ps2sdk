/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <dev9.h>
#include <irx_imports.h>

#define MODNAME "ARCADE_DEV9_driver"
IRX_ID(MODNAME, 1, 80);
// Text section hash:
// 1bfb37ce42eebcf7b04fc2d5e09d5f5d

extern struct irx_export_table _exp_dev9;

#define sceDev9Entry _start

void SpdRegisterIntrHandler(int intr, dev9_intr_cb_t cb)
{
	(void)intr;
	(void)cb;
	printf("SpdRegisterIntrHandler\n");
	while ( 1 )
		;
}

int SpdDmaTransfer(int ctrl, void *buf, int bcr, int dir)
{
	(void)ctrl;
	(void)buf;
	(void)bcr;
	(void)dir;
	printf("SpdDmaTransfer\n");
	while ( 1 )
		;
}

void Dev9CardStop(void)
{
	printf("Dev9CardStop\n");
	while ( 1 )
		;
}

void SpdIntrEnable(int mask)
{
	(void)mask;
	printf("SpdIntrEnable\n");
	while ( 1 )
		;
}

void SpdIntrDisable(int mask)
{
	(void)mask;
	printf("SpdIntrDisable\n");
	while ( 1 )
		;
}

int SpdGetEthernetID(u16 *buf)
{
	(void)buf;
	printf("SpdGetEthernetID\n");
	while ( 1 )
		;
}

void SpdSetLED(int ctl)
{
	(void)ctl;
	printf("SpdSetLED\n");
	while ( 1 )
		;
}

int Dev9RegisterPowerOffHandler(int idx, dev9_shutdown_cb_t cb)
{
	(void)idx;
	(void)cb;
	printf("Dev9RegisterPowerOffHandler no Support\n");
	return 0;
}

int sceDev9Entry()
{
	if ( RegisterLibraryEntries(&_exp_dev9) )
		printf("atad: module already loaded\n");
	printf("sceDev9Entry\n");
	return 0;
}

int sceDev9Terminate()
{
	printf(" sceDev9Terminate\n");
	return 0;
}
