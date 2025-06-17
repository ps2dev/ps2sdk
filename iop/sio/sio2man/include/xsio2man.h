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
 * SIO2MAN from SDK 1.4 and newer definitions
 * For non-Protokernel systems, rom0:XSIO2MAN module definitions
 */

#ifndef __XSIO2MAN_H__
#define __XSIO2MAN_H__

#include <sio2man.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sio2_mtap_change_slot_cb_t)(s32 *status);
typedef int (*sio2_mtap_get_slot_max_cb_t)(int port);
typedef int (*sio2_mtap_get_slot_max2_cb_t)(int port);
typedef void (*sio2_mtap_update_slots_t)(void);

extern void sio2_transfer_reset(void);
extern void sio2_ctrl_set2(u32 val);
extern u32 sio2_ctrl_get2(void);
extern u32 sio2_stat6c_get2(void);
extern void sio2_portN_ctrl1_set2(int N, u32 val);
extern u32 sio2_portN_ctrl1_get2(int N);
extern void sio2_portN_ctrl2_set2(int N, u32 val);
extern u32 sio2_portN_ctrl2_get2(int N);
extern u32 sio2_stat70_get2(void);
extern void sio2_regN_set2(int N, u32 val);
extern u32 sio2_regN_get2(int N);
extern u32 sio2_stat74_get2(void);
extern void sio2_unkn78_set2(u32 val);
extern u32 sio2_unkn78_get2(void);
extern void sio2_unkn7c_set2(u32 val);
extern u32 sio2_unkn7c_get2(void);
extern void sio2_data_out2(u8 val);
extern u8 sio2_data_in2(void);
extern void sio2_stat_set2(u32 val);
extern u32 sio2_stat_get2(void);
extern void sio2_pad_transfer_init2(void);
extern void sio2_mc_transfer_init2(void);
extern void sio2_mtap_transfer_init(void);
extern int sio2_transfer2(sio2_transfer_data_t *td);
extern void sio2_transfer_reset2(void);
extern void sio2_mtap_change_slot_set(sio2_mtap_change_slot_cb_t cb);
extern void sio2_mtap_get_slot_max_set(sio2_mtap_get_slot_max_cb_t cb);
extern void sio2_mtap_get_slot_max2_set(sio2_mtap_get_slot_max2_cb_t cb);
extern void sio2_mtap_update_slots_set(sio2_mtap_update_slots_t cb);
extern int sio2_mtap_change_slot(s32 *arg);
extern int sio2_mtap_get_slot_max(int port);
extern int sio2_mtap_get_slot_max2(int port);
extern void sio2_mtap_update_slots(void);

#define xsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 1, 2)
#define xsio2man_IMPORTS_end END_IMPORT_TABLE

#ifndef NO_XSIO2MAN_IMPORTS
#define I_sio2_transfer_reset DECLARE_IMPORT(26, sio2_transfer_reset)
#define I_sio2_ctrl_set2 DECLARE_IMPORT(27, sio2_ctrl_set2)
#define I_sio2_ctrl_get2 DECLARE_IMPORT(28, sio2_ctrl_get2)
#define I_sio2_stat6c_get2 DECLARE_IMPORT(29, sio2_stat6c_get2)
#define I_sio2_portN_ctrl1_set2 DECLARE_IMPORT(30, sio2_portN_ctrl1_set2)
#define I_sio2_portN_ctrl1_get2 DECLARE_IMPORT(31, sio2_portN_ctrl1_get2)
#define I_sio2_portN_ctrl2_set2 DECLARE_IMPORT(32, sio2_portN_ctrl2_set2)
#define I_sio2_portN_ctrl2_get2 DECLARE_IMPORT(33, sio2_portN_ctrl2_get2)
#define I_sio2_stat70_get2 DECLARE_IMPORT(34, sio2_stat70_get2)
#define I_sio2_regN_set2 DECLARE_IMPORT(35, sio2_regN_set2)
#define I_sio2_regN_get2 DECLARE_IMPORT(36, sio2_regN_get2)
#define I_sio2_stat74_get2 DECLARE_IMPORT(37, sio2_stat74_get2)
#define I_sio2_unkn78_set2 DECLARE_IMPORT(38, sio2_unkn78_set2)
#define I_sio2_unkn78_get2 DECLARE_IMPORT(39, sio2_unkn78_get2)
#define I_sio2_unkn7c_set2 DECLARE_IMPORT(40, sio2_unkn7c_set2)
#define I_sio2_unkn7c_get2 DECLARE_IMPORT(41, sio2_unkn7c_get2)
#define I_sio2_data_out2 DECLARE_IMPORT(42, sio2_data_out2)
#define I_sio2_data_in2 DECLARE_IMPORT(43, sio2_data_in2)
#define I_sio2_stat_set2 DECLARE_IMPORT(44, sio2_stat_set2)
#define I_sio2_stat_get2 DECLARE_IMPORT(45, sio2_stat_get2)
#define I_sio2_pad_transfer_init2 DECLARE_IMPORT(46, sio2_pad_transfer_init2)
#define I_sio2_mc_transfer_init2 DECLARE_IMPORT(47, sio2_mc_transfer_init2)
#define I_sio2_mtap_transfer_init DECLARE_IMPORT(48, sio2_mtap_transfer_init)
#ifndef NO_XSIO2MAN_V2_CONFLICTING_IMPORTS
#define I_sio2_transfer2 DECLARE_IMPORT(49, sio2_transfer2)
#define I_sio2_transfer_reset2 DECLARE_IMPORT(50, sio2_transfer_reset2)
#define I_sio2_mtap_change_slot_set DECLARE_IMPORT(51, sio2_mtap_change_slot_set)
#define I_sio2_mtap_get_slot_max_set DECLARE_IMPORT(52, sio2_mtap_get_slot_max_set)
#define I_sio2_mtap_get_slot_max2_set DECLARE_IMPORT(53, sio2_mtap_get_slot_max2_set)
#define I_sio2_mtap_update_slots_set DECLARE_IMPORT(54, sio2_mtap_update_slots_set)
#define I_sio2_mtap_change_slot DECLARE_IMPORT(55, sio2_mtap_change_slot)
#define I_sio2_mtap_get_slot_max DECLARE_IMPORT(56, sio2_mtap_get_slot_max)
#define I_sio2_mtap_get_slot_max2 DECLARE_IMPORT(57, sio2_mtap_get_slot_max2)
#define I_sio2_mtap_update_slots DECLARE_IMPORT(58, sio2_mtap_update_slots)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XSIO2MAN_H__ */
