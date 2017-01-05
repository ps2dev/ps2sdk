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
# rom1:SIO2MAN module definitions
# rom1:SIO2MAN is a newer version of SIO2MAN than rom0:SIO2MAN.
*/

#ifndef IOP_RSIO2MAN_H
#define IOP_RSIO2MAN_H

#include "irx.h"
#include "sio2man.h"

// Callback function definitions
typedef int (*sio2_mtap_change_slot_cb_t)(s32 *status);
typedef int (*sio2_mtap_get_slot_max_cb_t)(int port);
typedef int (*sio2_mtap_get_slot_max2_cb_t)(int port);
typedef void (*sio2_mtap_update_slots_t)(void);

// IRX Imports
#define rsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 2, 3)
#define rsio2man_IMPORTS_end END_IMPORT_TABLE

void sio2_transfer_reset(void);
#define I_sio2_transfer_reset DECLARE_IMPORT(26, sio2_transfer_reset)

void sio2_mtap_transfer_init(void);
#define I_sio2_mtap_transfer_init DECLARE_IMPORT(48, sio2_mtap_transfer_init)

void sio2_rm_transfer_init(void);
#define I_sio2_rm_transfer_init DECLARE_IMPORT(49, sio2_rm_transfer_init)

void sio2_unk_transfer_init(void);
#define I_sio2_unk_transfer_init DECLARE_IMPORT(50, sio2_unk_transfer_init)

int sio2_transfer2(sio2_transfer_data_t *td);
#define I_sio2_transfer2 DECLARE_IMPORT(51, sio2_transfer2)

void sio2_transfer_reset2(void);
#define I_sio2_transfer_reset2 DECLARE_IMPORT(52, sio2_transfer_reset2)

void sio2_mtap_change_slot_set(sio2_mtap_change_slot_cb_t cb);
#define I_sio2_mtap_change_slot_set DECLARE_IMPORT(53, sio2_mtap_change_slot_set)

void sio2_mtap_get_slot_max_set(sio2_mtap_get_slot_max_cb_t cb);
#define I_sio2_mtap_get_slot_max_set DECLARE_IMPORT(54, sio2_mtap_get_slot_max_set)

void sio2_mtap_get_slot_max2_set(sio2_mtap_get_slot_max2_cb_t cb);
#define I_sio2_mtap_get_slot_max2_set DECLARE_IMPORT(55, sio2_mtap_get_slot_max2_set)

void sio2_mtap_update_slots_set(sio2_mtap_update_slots_t cb);
#define I_sio2_mtap_update_slots_set DECLARE_IMPORT(56, sio2_mtap_update_slots_set)

int sio2_mtap_change_slot(s32 *arg);
#define I_sio2_mtap_change_slot DECLARE_IMPORT(57, sio2_mtap_change_slot)

int sio2_mtap_get_slot_max(int port);
#define I_sio2_mtap_get_slot_max DECLARE_IMPORT(58, sio2_mtap_get_slot_max)

int sio2_mtap_get_slot_max2(int port);
#define I_sio2_mtap_get_slot_max2 DECLARE_IMPORT(59, sio2_mtap_get_slot_max2)

void sio2_mtap_update_slots(void);
#define I_sio2_mtap_update_slots DECLARE_IMPORT(60, sio2_mtap_update_slots)

#endif	// IOP_RSIO2MAN_H
