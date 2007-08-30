#ifndef IOP_XMTAPMAN_H
#define IOP_XMTAPMAN_H

#include "irx.h"

s32 mtapPortOpen(u32 port);
s32 mtapPortClose(u32 port);
s32 mtapGetSlotNumber(u32 port);
s32 mtapChangeSlot(u32 port, u32 slot);

#define xmtapman_IMPORTS_start DECLARE_IMPORT_TABLE(mtapman, 1, 2)
#define xmtapman_IMPORTS_end END_IMPORT_TABLE

#define I_mtapPortOpen				DECLARE_IMPORT(4, mtapPortOpen)
#define I_mtapPortClose				DECLARE_IMPORT(5, mtapPortClose)
#define I_mtapPortGetSlotNumber		DECLARE_IMPORT(6, mtapGetSlotNumber)
#define I_mtapChangeSlot			DECLARE_IMPORT(7, mtapChangeSlot)
	

#endif

