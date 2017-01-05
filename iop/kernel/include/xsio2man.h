#ifndef IOP_XSIO2MAN_H
#define IOP_XSIO2MAN_H

#include "irx.h"
#include "sio2man.h"

// Callback function definitions
typedef int (*sio2_mtap_change_slot_cb_t)(s32 *status);
typedef int (*sio2_mtap_get_slot_max_cb_t)(int port);
typedef int (*sio2_mtap_get_slot_max2_cb_t)(int port);
typedef void (*sio2_mtap_update_slots_t)(void);

u32 sio2_stat70_get();
void sio2_pad_transfer_init(void);
int sio2_transfer(sio2_transfer_data_t *td);
void sio2_transfer_reset(void);
void sio2_mtap_transfer_init(void);
int sio2_transfer2(sio2_transfer_data_t *td);
void sio2_transfer_reset2(void);

void sio2_mtap_change_slot_set(sio2_mtap_change_slot_cb_t cb);
void sio2_mtap_get_slot_max_set(sio2_mtap_get_slot_max_cb_t cb);
void sio2_mtap_get_slot_max2_set(sio2_mtap_get_slot_max2_cb_t cb);
void sio2_mtap_update_slots_set(sio2_mtap_update_slots_t cb);

int sio2_mtap_change_slot(s32 *arg);
int sio2_mtap_get_slot_max(int port);
int sio2_mtap_get_slot_max2(int port);
void sio2_mtap_update_slots(void);

// IRX Imports
#define xsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 1, 2)
#define xsio2man_IMPORTS_end END_IMPORT_TABLE

#define I_sio2_stat70_get DECLARE_IMPORT(11, sio2_stat70_get)
#define I_sio2_pad_transfer_init DECLARE_IMPORT(23, sio2_pad_transfer_init)
#define I_sio2_transfer DECLARE_IMPORT(25, sio2_transfer)
#define I_sio2_transfer_reset DECLARE_IMPORT(26, sio2_transfer_reset)
#define I_sio2_mtap_transfer_init	DECLARE_IMPORT(48, sio2_mtap_transfer_init)
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
