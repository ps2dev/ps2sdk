/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * rom1:SIO2MAN module definitions
 * rom1:SIO2MAN is a newer version of SIO2MAN than rom0:SIO2MAN.
 */

#ifndef __RSIO2MAN_H__
#define __RSIO2MAN_H__

#define NO_XSIO2MAN_IMPORTS
#include <xsio2man.h>
#undef NO_XSIO2MAN_IMPORTS

void sio2_rm_transfer_init(void);
void sio2_unk_transfer_init(void);

#define rsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 2, 3)
#define rsio2man_IMPORTS_end END_IMPORT_TABLE

#define I_sio2_transfer_reset DECLARE_IMPORT(26, sio2_transfer_reset)
#define I_sio2_mtap_transfer_init DECLARE_IMPORT(48, sio2_mtap_transfer_init)
#define I_sio2_rm_transfer_init DECLARE_IMPORT(49, sio2_rm_transfer_init)
#define I_sio2_unk_transfer_init DECLARE_IMPORT(50, sio2_unk_transfer_init)
#define I_sio2_transfer2 DECLARE_IMPORT(51, sio2_transfer2)
#define I_sio2_transfer_reset2 DECLARE_IMPORT(52, sio2_transfer_reset2)
#define I_sio2_mtap_change_slot_set DECLARE_IMPORT(53, sio2_mtap_change_slot_set)
#define I_sio2_mtap_get_slot_max_set DECLARE_IMPORT(54, sio2_mtap_get_slot_max_set)
#define I_sio2_mtap_get_slot_max2_set DECLARE_IMPORT(55, sio2_mtap_get_slot_max2_set)
#define I_sio2_mtap_update_slots_set DECLARE_IMPORT(56, sio2_mtap_update_slots_set)
#define I_sio2_mtap_change_slot DECLARE_IMPORT(57, sio2_mtap_change_slot)
#define I_sio2_mtap_get_slot_max DECLARE_IMPORT(58, sio2_mtap_get_slot_max)
#define I_sio2_mtap_get_slot_max2 DECLARE_IMPORT(59, sio2_mtap_get_slot_max2)
#define I_sio2_mtap_update_slots DECLARE_IMPORT(60, sio2_mtap_update_slots)

#endif /* __RSIO2MAN_H__ */
