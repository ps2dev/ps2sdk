#ifndef IOP_XSIO2MAN_H
#define IOP_XSIO2MAN_H

#include "irx.h"
#include "sio2man.h"

u32 sio2_stat70_get();
void sio2_pad_transfer_init();
int sio2_transfer(sio2_transfer_data_t *td);
void sio2_mtap_transfer_init();
void sio2_transfer_reset();

void sio2_mtap_change_slot_set(int (*cb)(u32 *));
void sio2_mtap_get_slot_max_set(int (*cb)(int)); 
void sio2_mtap_get_slot_max_set2(int (*cb)(int));
void sio2_mtap_update_slots_set(void (*cb)(void));

int sio2_mtap_change_slot(u32 *arg);
int sio2_mtap_get_slot_max(u32 port);
void sio2_mtap_update_slots();

// IRX Imports
#define xsio2man_IMPORTS_start DECLARE_IMPORT_TABLE(sio2man, 1, 2)
#define xsio2man_IMPORTS_end END_IMPORT_TABLE

#define I_sio2_stat70_get DECLARE_IMPORT(11, sio2_stat70_get)
#define I_sio2_pad_transfer_init DECLARE_IMPORT(23, sio2_pad_transfer_init)
#define I_sio2_transfer DECLARE_IMPORT(25, sio2_transfer)
#define I_sio2_mtap_transfer_init	DECLARE_IMPORT(48, sio2_mtap_transfer_init)
//#define I_sio2_transfer DECLARE_IMPORT(49, sio2_transfer)
#define I_sio2_transfer_reset DECLARE_IMPORT(50, sio2_transfer_reset)
#define I_sio2_mtap_change_slot_set DECLARE_IMPORT(51, sio2_mtap_change_slot_set)
#define I_sio2_mtap_get_slot_max_set DECLARE_IMPORT(52, sio2_mtap_get_slot_max_set)
#define I_sio2_mtap_get_slot_max_set2 DECLARE_IMPORT(53, sio2_mtap_get_slot_max_set2)
#define I_sio2_mtap_update_slots_set DECLARE_IMPORT(54, sio2_mtap_update_slots_set)
#define I_sio2_mtap_change_slot DECLARE_IMPORT(55, sio2_mtap_change_slot)
#define I_sio2_mtap_get_slot_max DECLARE_IMPORT(56, sio2_mtap_get_slot_max)
#define I_sio2_mtap_update_slots DECLARE_IMPORT(58, sio2_mtap_update_slots)

#endif

