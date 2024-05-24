/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#ifndef _ACUART_H
#define _ACUART_H

#include <accore.h>

typedef enum ac_uart_flag
{
	AC_UART_FLAG_READ = 0x1,
	AC_UART_FLAG_WRITE = 0x2,
	AC_UART_FLAG_BOTH = 0x3,
} acUartFlag;

typedef struct ac_uart_attr
{
	acInt32 ua_speed;
	acInt32 ua_fifo;
	acInt32 ua_loopback;
	acInt32 ua_padding;
} acUartAttrData;
typedef acUartAttrData *acUartAttrT;

extern int acUartModuleRestart(int argc, char **argv);
extern int acUartModuleStart(int argc, char **argv);
extern int acUartModuleStatus();
extern int acUartModuleStop();
extern int acUartRead(void *buf, int count);
extern int acUartWrite(void *buf, int count);
extern int acUartWait(acUartFlag rw, int usec);
extern int acUartGetAttr(acUartAttrData *attr);
extern int acUartSetAttr(const acUartAttrData *attr);

#define acuart_IMPORTS_start DECLARE_IMPORT_TABLE(acuart, 1, 1)
#define acuart_IMPORTS_end END_IMPORT_TABLE

#define I_acUartModuleRestart DECLARE_IMPORT(4, acUartModuleRestart)
#define I_acUartModuleStart DECLARE_IMPORT(5, acUartModuleStart)
#define I_acUartModuleStatus DECLARE_IMPORT(6, acUartModuleStatus)
#define I_acUartModuleStop DECLARE_IMPORT(7, acUartModuleStop)
#define I_acUartRead DECLARE_IMPORT(8, acUartRead)
#define I_acUartWrite DECLARE_IMPORT(9, acUartWrite)
#define I_acUartWait DECLARE_IMPORT(10, acUartWait)
#define I_acUartGetAttr DECLARE_IMPORT(11, acUartGetAttr)
#define I_acUartSetAttr DECLARE_IMPORT(12, acUartSetAttr)

#endif
