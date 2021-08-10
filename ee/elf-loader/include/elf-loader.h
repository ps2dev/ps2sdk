/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2020 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * ELF Loader functions.
 */

#ifndef __ELFLOADER_H__
#define __ELFLOADER_H__

#ifdef __cplusplus
extern "C" {
#endif

// Before call this method be sure that you have previously called sbv_patch_disable_prefix_check();
int LoadELFFromFile(const char *filename, int argc, char *argv[]);

/** Modify argv[0] when partition info should be kept
 *
 * @param filename  path to the file itself
 * @param partition block device + partition name
 * @param argc      Number of arguments.
 * @param argv      Pointer to array of arguments.
 * @returns         positive on success, zero value on failure
 *
 * You should prepare filename and partition before parsing in function 
 *
 * filename should be changed to <fs_type>:/path_to_file_without_mountpoint
 * for example `pfs3:/TEST.ELF` (where `pfs3` is mountpoint name) will become
 * `pfs:/TEST.ELF` (where `pfs` - filesystem type)
 *
 * partition should be in form <block_device><partition_name>:
 * for example `hdd0:__common:`
 * block_device - can be `hdd0:`, `hdd1:` , `dvr_hdd0:`
 * <partition_name> - name of APA partition on particular block device
 * dont forget about leading ":"
 *
 * It is not necessary that filename should be on that partition
 * filename - mass:/TEST.ELF
 * partition - pfs:/__common
 * will be valid usage
 */
int LoadELFFromFileWithPartition(const char *filename, const char *partition, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __ELFLOADER_H__ */
