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
/* partition is useful when you're launching an ELF from HDD. If this is the case a example could be:
*   filename = pfs:something/myprogram.elf
*   partition = hdd0:__common:
* What iternally we will do is to concatenate partition + filename, and send it in the argv[0], which is used by CWD
* So folowing the previous example. when elf is launched argv[0] = hdd0:__common:pfs:something/myprogram.elf
* If you aren't going to load from HDD you can pass in partition either NULL or ""
*/

int LoadELFFromFileWithPartition(const char *filename, const char *partition, int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __ELFLOADER_H__ */
