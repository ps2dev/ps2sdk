/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * Special IOP Reboot routines for rebooting with an IOPRP image/encrypted UDNL module in a buffer.
 */

#include "kernel.h"
#include "iopcontrol.h"
#include "iopcontrol_special.h"
#include "loadfile.h"
#include "iopheap.h"
#include "sifdma.h"
#include "sifrpc.h"
#include "string.h"
#include "sbv_patches.h"

#define IMGDRV_IRX_SIZE		((size__imgdrv_irx + 15) & ~15)	//Was a hardcoded value of 0x400 bytes
#define IOPBTCONF_IOP_MAX_SIZE	0x400

extern int _iop_reboot_count;

extern u8 iopbtconf_img[IOPBTCONF_IOP_MAX_SIZE];
extern unsigned char _imgdrv_irx[];
extern unsigned int size__imgdrv_irx;

//If for whatever reason imgdrv changes, update these offsets.
#define IMGDRV_IRX_PTRS		0x190
#define IMGDRV_IRX_SIZES	0x198

#ifdef F__iopcontrol_special_internals
u8 iopbtconf_img[IOPBTCONF_IOP_MAX_SIZE] __attribute__((aligned(64)));
#endif

//Our LOADFILE functions are slightly different.
#define SifLoadModuleSpecial(path, arg_len, args, dontwait)	\
	_SifLoadModule(path, arg_len, args, NULL, LF_F_MOD_LOAD, dontwait);

#define SifLoadModuleEncryptedSpecial(path, arg_len, args, dontwait)	\
	_SifLoadModule(path, arg_len, args, NULL, LF_F_MG_MOD_LOAD, dontwait);

#ifdef F_SifIopRebootBufferEncrypted
int SifIopRebootBufferEncrypted(const void *udnl, int size)
{
	int iopbtconf_img_size;
	void *imgdrv_iop, *udnl_iop,  *iopbtconf_img_iop;
	int *imgdrv_img_size;
	void **imgdrv_img;
	SifDmaTransfer_t dmat[3];

	SifInitRpc(0);
	SifExitRpc();

	SifIopReset("", 0);
	while(!SifIopSync()){};

	SifLoadFileInit();
	SifInitIopHeap();

	imgdrv_iop = SifAllocIopHeap(IMGDRV_IRX_SIZE);
	udnl_iop = SifAllocIopHeap(size);
	iopbtconf_img_iop = SifAllocIopHeap(IOPBTCONF_IOP_MAX_SIZE);

	if(imgdrv_iop == NULL || udnl_iop == NULL || iopbtconf_img_iop == NULL)
		return -1;

	iopbtconf_img_size = 0;	//No support for IOPBTCONF manipulation

	imgdrv_img = (void**)&_imgdrv_irx[IMGDRV_IRX_PTRS];
	imgdrv_img_size = (int*)&_imgdrv_irx[IMGDRV_IRX_SIZES];
	dmat[0].src = _imgdrv_irx;
	dmat[0].dest = imgdrv_iop;
	dmat[0].size = IMGDRV_IRX_SIZE;
	dmat[0].attr = 0;
	dmat[1].src = (void*)udnl;
	dmat[1].dest = udnl_iop;
	dmat[1].size = size;
	dmat[1].attr = 0;
	dmat[2].src = iopbtconf_img;
	dmat[2].dest = iopbtconf_img_iop;
	dmat[2].size = iopbtconf_img_size;
	dmat[2].attr = 0;
	imgdrv_img[0] = udnl_iop;
	imgdrv_img[1] = iopbtconf_img_iop;
	imgdrv_img_size[0] = size;
	imgdrv_img_size[1] = iopbtconf_img_size;
	FlushCache(0);
	SifSetDma(dmat, 3);

	sbv_patch_enable_lmb();

	SifLoadModule("rom0:SYSCLIB", 0, NULL);
	SifLoadModuleBuffer(imgdrv_iop, 0, NULL);
	SifLoadModuleEncryptedSpecial("img0:", 0, NULL, 1);

	SifExitRpc();
	SifLoadFileExit();

	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_SIFINIT);
	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_CMDINIT);
	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_BOOTEND);
	SifSetReg(SIF_SYSREG_RPCINIT, 0);
	SifSetReg(SIF_SYSREG_SUBADDR, 0);

	_iop_reboot_count++;	//Not originally here: increment to allow RPC clients to detect unbinding.

	return 1;
}
#endif

#ifdef F_SifIopRebootBuffer
static int generateIOPBTCONF_img(void *output, const void *ioprp);

int SifIopRebootBuffer(const void *ioprp, int size)
{
	int iopbtconf_img_size;
	void *imgdrv_iop, *ioprp_iop,  *iopbtconf_img_iop;
	int *imgdrv_img_size;
	void **imgdrv_img;
	SifDmaTransfer_t dmat[3];

	SifInitRpc(0);
	SifExitRpc();

	SifIopReset("", 0);
	while(!SifIopSync()){};

	SifLoadFileInit();
	SifInitIopHeap();

	imgdrv_iop = SifAllocIopHeap(IMGDRV_IRX_SIZE);
	ioprp_iop = SifAllocIopHeap(size);
	iopbtconf_img_iop = SifAllocIopHeap(IOPBTCONF_IOP_MAX_SIZE);

	if(imgdrv_iop == NULL || ioprp_iop == NULL || iopbtconf_img_iop == NULL)
		return -1;

	iopbtconf_img_size = generateIOPBTCONF_img(iopbtconf_img, ioprp);

	imgdrv_img = (void**)&_imgdrv_irx[IMGDRV_IRX_PTRS];
	imgdrv_img_size = (int*)&_imgdrv_irx[IMGDRV_IRX_SIZES];
	dmat[0].src = _imgdrv_irx;
	dmat[0].dest = imgdrv_iop;
	dmat[0].size = IMGDRV_IRX_SIZE;
	dmat[0].attr = 0;
	dmat[1].src = (void*)ioprp;
	dmat[1].dest = ioprp_iop;
	dmat[1].size = size;
	dmat[1].attr = 0;
	dmat[2].src = iopbtconf_img;
	dmat[2].dest = iopbtconf_img_iop;
	dmat[2].size = iopbtconf_img_size;
	dmat[2].attr = 0;
	imgdrv_img[0] = ioprp_iop;
	imgdrv_img[1] = iopbtconf_img_iop;
	imgdrv_img_size[0] = size;
	imgdrv_img_size[1] = iopbtconf_img_size;
	FlushCache(0);
	SifSetDma(dmat, 3);

	sbv_patch_enable_lmb();

	SifLoadModule("rom0:SYSCLIB", 0, NULL);
	SifLoadModuleBuffer(imgdrv_iop, 0, NULL);
	SifLoadModuleSpecial("rom0:UDNL", 11, "img0:\0img1:", 1);

	SifExitRpc();
	SifLoadFileExit();

	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_SIFINIT);
	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_CMDINIT);
	SifSetReg(SIF_REG_SMFLAG, SIF_STAT_BOOTEND);
	SifSetReg(SIF_SYSREG_RPCINIT, 0);
	SifSetReg(SIF_SYSREG_SUBADDR, 0);

	_iop_reboot_count++;	//Not originally here: increment to allow RPC clients to detect unbinding.

	return 1;
}

typedef struct romdir{
	char name[10];
	u16 ExtInfoEntrySize;
	u32 size;
} romdir_t;

typedef struct extinfo{
	u16 value;		/* Only applicable for the version field type. */
	u8 ExtLength;	/* The length of data appended to the end of this entry. */
	u8 type;
} extinfo_t;

enum EXTINFO_TYPE{
	EXTINFO_TYPE_DATE = 1,
	EXTINFO_TYPE_VERSION,
	EXTINFO_TYPE_COMMENT,
	EXTINFO_TYPE_FIXED = 0x7F	//Must exist at a fixed location.
};

struct iopbtconf_img {
	romdir_t romdir[5];
	u32 extinfo[4];
};

static const struct iopbtconf_img iopbtconf_img_base = {
	{
		{"RESET", 8, 0},
		{"ROMDIR", 0, 0x50},
		{"EXTINFO", 0, 0x10},
		{"IOPBTCONF", 8, -1},
		{"", 0, 0}
	},
	{
		0x01040000,	//EXTINFO {0x0000, 4, EXTINFO_TYPE_DATE},
		0x20010406,	//Date: 2001/04/06
		0x01040000,	//EXTINFO {0x0000, 4, EXTINFO_TYPE_DATE},
		0x20010406,	//Date: 2001/04/06
	}
};

/*	Generate an IOPRP image that contains IOPBTCONF, if the original contains IOPBTCONF.
	This is required because UDNL will only seach succeeding IOPRP images for modules specified within IOPBTCONF.	*/
static int generateIOPBTCONF_img(void *output, const void *ioprp)
{
	int size, offset, fsize_rounded;
	romdir_t *romdir;
	const u8 *ptr_in;
	u8 *ptr_out;

	romdir = (romdir_t*)ioprp;
	if(strcmp(romdir[0].name, "RESET") || strcmp(romdir[1].name, "ROMDIR"))
		return -1;

	//Now locate IOPBTCONF
	size = 0;
	offset = 0;
	for(; romdir->name[0] != '\0'; romdir++,offset += fsize_rounded)
	{
		fsize_rounded = (romdir->size + 15) & ~15;
		if(strcmp(romdir->name, "IOPBTCONF") == 0)
		{
			romdir->name[0] = 'X';	//Change 'IOPBTCONF' to 'XOPBTCONF', so that UDNL will not find the original.
			//Copy IOPBTCONF into the new image.
			size = romdir->size + sizeof(iopbtconf_img_base);
			ptr_out = (u8*)output;
			ptr_in = (const u8*)ioprp;
			memcpy(ptr_out, &iopbtconf_img_base, sizeof(iopbtconf_img_base));
			memcpy(ptr_out + sizeof(iopbtconf_img_base), &ptr_in[offset], romdir->size);
			((romdir_t*)ptr_out)[3].size = romdir->size;	//Update the size of IOPBTCONF within the generated IOPRP image.
			break;
		}
	}

	return size;
}
#endif
