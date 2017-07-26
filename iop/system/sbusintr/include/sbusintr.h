/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * SBUS interrupts.
 */

#ifndef __SBUSINTR_H__
#define __SBUSINTR_H__

#include <types.h>
#include <irx.h>

/* SBUS interface */
enum sbus_errors {
	SBUS_E_OK,

	/** Initialization error */
	SBUS_E_INIT = 0xd600,
	/** Error with an argument */
	SBUS_E_ARG,
	/** Wrong IRQ number / already in use */
	SBUS_E_IRQ,
};

typedef int (*sbus_intr_handler_t)(u32, void *);

int sbus_intr_init();
void sbus_intr_exit();

int sbus_intr_handler_add(u32 irq, sbus_intr_handler_t handler, void *arg);
int sbus_intr_handler_del(u32 irq);

void sbus_intr_main_interrupt(u32 irq);

#define sbusintr_IMPORTS_start DECLARE_IMPORT_TABLE(sbusintr, 1, 1)
#define sbusintr_IMPORTS_end END_IMPORT_TABLE

#define I_sbus_intr_init DECLARE_IMPORT(4, sbus_intr_init)
#define I_sbus_intr_exit DECLARE_IMPORT(2, sbus_intr_exit)
#define I_sbus_intr_handler_add DECLARE_IMPORT(5, sbus_intr_handler_add)
#define I_sbus_intr_handler_del DECLARE_IMPORT(6, sbus_intr_handler_del)
#define I_sbus_intr_main_interrupt DECLARE_IMPORT(7, sbus_intr_main_interrupt)

#endif /* __SBUSINTR_H__ */
