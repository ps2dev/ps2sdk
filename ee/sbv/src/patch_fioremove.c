#include "tamtypes.h"
#include "kernel.h"
#include "string.h"

#include "smem.h"
#include "smod.h"

int sbv_patch_fioremove()
{
	/* This patch is a fix for FILEIO on IOP: 				*/
	/* the handler of rpc_fioremove doesn't exit just after file is		*/
	/* removed due to a bug, and chain with rpc_fiomkdir which have for	*/
	/* effect to create a folder of the same name that the deleted file...	*/
	/* We'll search for FILEIO text section start and patch the rpc		*/
	/* handler to jump to a subroutine to correct this.			*/
	/* This subroutine is 16 bytes and will be written in a FILEIO string	*/
	/* that have poor interest: "iop heap service (99/11/03)\n" will	*/
	/* be cutted to "iop heap service".					*/
	/*									*/
	/* text + 0x1447: patched with 0 (1 byte, terminate the FILEIO string)	*/
	/* text + 0x1448: patched with the patch given below (16 bytes, this is	*/
	/* 		  our new rpc_fioremove handler)			*/
	/* text + 0x0bb8: patched with 0x0c000512(+text_start) (4 bytes, this	*/
	/*		  is the modified jump to our handler)			*/
	/*									*/
	/* Here is what the 16 bytes patch look like:				*/
	/* 0x0c0001ce(+text_start) 	// jal fio_remove			*/
	/* 0x00000000			// nop					*/
	/* 0x0800033a(+text_start)	// j rpc_handler_exit			*/
	/* 0x00000000			// nop					*/

	smod_mod_info_t mod_info;
	u8 jal_fioremove[4];
	u8 new_fioremove[16];
	volatile u32 j_new_fioremove = 0x08000512;
	volatile u32 j_rpc_handler_exit = 0x0800033a;
	volatile u8 string_term = 0;

	memset(&mod_info, 0, sizeof(mod_info));
	int ret = smod_get_mod_by_name("FILEIO_service", &mod_info);
	if ((!ret) || (mod_info.version != 0x101))
		return -1;

	/* setup our jump opcodes */
	j_rpc_handler_exit += ((u32)mod_info.text_start >> 2);
	j_new_fioremove    += ((u32)mod_info.text_start >> 2);

	/* get the "jal fioremove" opcode */
	smem_read((void *)mod_info.text_start  + 0x0bb8, jal_fioremove, sizeof(jal_fioremove));

	/* setup our patch code */
	memset(new_fioremove, 0, sizeof(new_fioremove));
	memcpy(&new_fioremove[0], jal_fioremove, sizeof(jal_fioremove));
	memcpy(&new_fioremove[8], (void *)&j_rpc_handler_exit, sizeof(j_rpc_handler_exit));

	/* apply it */
	SyncDCache((void *)&string_term, (void *)(&string_term + sizeof(string_term)));
	smem_write((void *)mod_info.text_start + 0x1447, (void *)&string_term, sizeof(string_term));
	SyncDCache(new_fioremove, (void *)(new_fioremove + sizeof(new_fioremove)));
	smem_write((void *)mod_info.text_start + 0x1448, new_fioremove, sizeof(new_fioremove));
	SyncDCache((void *)&j_new_fioremove, (void *)(&j_new_fioremove + sizeof(j_new_fioremove)));
	smem_write((void *)mod_info.text_start + 0x0bb8, (void *)&j_new_fioremove, sizeof(j_new_fioremove));

	return 0;
}

