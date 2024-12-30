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
 * SIO2MAN from SDK 2.0 and newer definitions
 * For DVD Player ROMs that support remote, rom1:SIO2MAN is a newer version of SIO2MAN than rom0:SIO2MAN.
 */

#ifndef __RSIO2MAN_H__
#define __RSIO2MAN_H__

#define NO_XSIO2MAN_V2_CONFLICTING_IMPORTS
#include <xsio2man.h>
#undef NO_XSIO2MAN_V2_CONFLICTING_IMPORTS

#ifdef __cplusplus
extern "C" {
#endif

extern void sio2_rm_transfer_init(void);
extern void sio2_imode_transfer_init(void);

// The following was added in SDK 2.4.
extern void sio2_pad2_transfer_init(void);
extern void sio2_transfer_reset3(void);

// The following was added with export version 2.4 in SDK 2.6.
extern void sio2_mc2_transfer_init(void);

// The following was added with export version 2.7 in SDK 3.0.3.
extern void sio2_set_intr_handler(int (*handler)(void *), void *userdata);
extern void sio2_set_ctrl_c(void);
extern void sio2_set_ctrl_1(void);
extern void sio2_wait_for_intr(void);

#define rsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 2, 3)
#define rsio2man_IMPORTS_end END_IMPORT_TABLE

#define I_sio2_rm_transfer_init DECLARE_IMPORT(49, sio2_rm_transfer_init)
#define I_sio2_imode_transfer_init DECLARE_IMPORT(50, sio2_imode_transfer_init)
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
#define I_sio2_pad2_transfer_init DECLARE_IMPORT(61, sio2_pad2_transfer_init)
#define I_sio2_transfer_reset3 DECLARE_IMPORT(62, sio2_transfer_reset3)
#define I_sio2_mc2_transfer_init DECLARE_IMPORT(63, sio2_mc2_transfer_init)
#define I_sio2_set_intr_handler DECLARE_IMPORT(64, sio2_set_intr_handler)
#define I_sio2_set_ctrl_c DECLARE_IMPORT(65, sio2_set_ctrl_c)
#define I_sio2_set_ctrl_1 DECLARE_IMPORT(66, sio2_set_ctrl_1)
#define I_sio2_wait_for_intr DECLARE_IMPORT(67, sio2_wait_for_intr)

#ifdef __cplusplus
}
#endif

#endif /* __RSIO2MAN_H__ */
