/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright (c) 2003  Marcus R. Brown <mrbrown@0xd6.org>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# SBUS interrupts.
*/

#ifndef IOP_SBUSINTR_H
#define IOP_SBUSINTR_H

#include "types.h"
#include "irx.h"

/* SBUS interface */
enum sbus_errors {
	SBUS_E_OK,

	SBUS_E_INIT = 0xd600,	/* Initialization error */
	SBUS_E_ARG,		/* Error with an argument */
	SBUS_E_IRQ,		/* Wrong IRQ number / already in use */
};

typedef int (*sbus_intr_handler_t)(u32, void *);

#define sbusintr_IMPORTS_start DECLARE_IMPORT_TABLE(sbusintr, 1, 1)
#define sbusintr_IMPORTS_end END_IMPORT_TABLE

int sbus_intr_init();
#define I_sbus_intr_init DECLARE_IMPORT(4, sbus_intr_init)
void sbus_intr_exit();
#define I_sbus_intr_exit DECLARE_IMPORT(2, sbus_intr_exit)

int sbus_intr_handler_add(u32 irq, sbus_intr_handler_t handler, void *arg);
#define I_sbus_intr_handler_add DECLARE_IMPORT(5, sbus_intr_handler_add)
int sbus_intr_handler_del(u32 irq);
#define I_sbus_intr_handler_del DECLARE_IMPORT(6, sbus_intr_handler_del)

void sbus_intr_main_interrupt(u32 irq);
#define I_sbus_intr_main_interrupt DECLARE_IMPORT(7, sbus_intr_main_interrupt)

#endif /* IOP_SBUSINTR_H */
