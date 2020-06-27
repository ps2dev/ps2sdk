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

#ifdef __cplusplus
}
#endif

#endif /* __ELFLOADER_H__ */
