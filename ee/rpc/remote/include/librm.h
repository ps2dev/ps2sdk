/*
 * librm.h - RPC Interface for PS2 Remote Control Driver (RMMAN)
 *
 * (c) 2004 TyRaNiD <tiraniddo@hotmail.com>
 *
 * See the file LICENSE included with this distribution for licensing terms.
 */

#ifndef __LIBRM_H__
#define __LIBRM_H__

struct remote_data 

{
   u32 status;
   u32 button;
};

/* Status values (guess) */
#define RM_READY      	0x005A12FF 
#define RM_KEYPRESSED 	0x145A12FF
#define RM_NOREMOTE   	0xFFFFFFFF

/* Button Values */
#define RM_AUDIO 	0x0040D649
#define RM_SHUFFLE	0x0050D349
#define RM_ANGLE	0x0050D649
#define RM_PROGRAM	0x00F0D149
#define RM_SUBTITLE	0x0030D649
#define RM_REPEAT	0x00C0D249
#define RM_SLOW_BACK	0x0000D649
#define RM_SLOW_FORW	0x0010D649
#define RM_SCAN_BACK	0x0030D349
#define RM_SCAN_FORW	0x0040D349
#define RM_ONE		0x0000D049
#define RM_TWO		0x0010D049
#define RM_THREE	0x0020D049
#define RM_FOUR		0x0030D049
#define RM_FIVE		0x0040D049
#define RM_SIX		0x0050D049
#define RM_SEVEN	0x0060D049
#define RM_EIGHT	0x0070D049
#define RM_NINE		0x0080D049
#define RM_ZERO		0x0090D049
#define RM_CLEAR	0x00F0D049
#define RM_TIME		0x0080D049
#define RM_PREV		0x0000D349
#define RM_NEXT		0x0010D349
#define RM_ATOB		0x00A0D249
#define RM_PLAY		0x0020D349
#define RM_PAUSE	0x0090D349
#define RM_STOP		0x0080D349
#define RM_DISPLAY	0x0040D549
#define RM_TITLE	0x00A0D149
#define RM_MENU		0x00B0D149
#define RM_RETURN	0x00E0D049
#define RM_TRIANGLE	0x00C0D5DA
#define RM_SQUARE	0x00F0D5DA
#define RM_CIRCLE	0x00D0D5DA
#define RM_CROSS	0x00E0D5DA
#define RM_UP		0x0040D5DA
#define RM_DOWN		0x00A0D749
#define RM_LEFT		0x00B0D749
#define RM_RIGHT	0x0040D5DA
#define RM_ENTER	0x00B0D049
#define RM_L1		0x00A0D5DA
#define RM_L2		0x0080D5DA
#define RM_L3		0x0010D5DA
#define RM_R1		0x00B0D5DA
#define RM_R2		0x0090D5DA
#define RM_R3		0x0020D5DA
#define RM_START	0x0030D5DA
#define RM_SELECT	0x0000D5DA


int RMMan_Init(void);
int RMMan_End(void);
int RMMan_Open(int port, int slot, void *pData);
int RMMan_Close(int port, int slot);
void RMMan_Read(int port, int slot, struct remote_data *data);
u32 RMMan_GetModuleVersion(void);

#endif
