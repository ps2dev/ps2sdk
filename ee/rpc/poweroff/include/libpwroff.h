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
 *  @file
 *  @brief Power-off library.
 *  @details Poweroff is a library that substitutes for the missing poweroff
 *  functionality in <b>rom0:CDVDMAN</b>, which is offered by newer CDVDMAN
 *  modules (i.e. `sceCdPoweroff`). Other than allowing the PlayStation 2
 *  to be switched off via software means, this is used to safeguard
 *  against the user switching off/resetting the console before data
 *  can be completely written to disk.
 */

#ifndef __LIBPWROFF_H__
#define __LIBPWROFF_H__

#define POWEROFF_THREAD_PRIORITY 0x70

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief User defined function for use in poweroffSetCallback()
 *
 *  @details In fact there can be anything.
 *  This function will be called when power button is pressed.
 *  In common cases it should end everything and poweroff console.
 *  You should close all files (`close(fd)`) and unmount all partitions. If you
 *  use PFS, close all files and unmount all partitions.
 *  ~~~~~~~~~~~~~~~{.c}
 *  fileXioDevctl("pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0)
 *  ~~~~~~~~~~~~~~~
 *  Shut down DEV9 (Network module), if you used it:
 *  ~~~~~~~~~~~~~~~{.c}
 *  while(fileXioDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0){};
 *  ~~~~~~~~~~~~~~~
 *  Last function inside callback should be
 *  poweroffShutdown().
 */
typedef void (*poweroff_callback)(void *arg);

/**
 *  @brief Initializes the poweroff library.
 *  @details A service thread with a default priority of <b>POWEROFF_THREAD_PRIORITY</b>
 *  will be created.
 */
int poweroffInit(void);

/**
 *  @brief Set callback function
 *
 *  @param [in] cb Function that will be called when power button is pressed.
 *  @param [in] arg Arguments that are sent to cb function (can be <b>NULL</b>)
 *
 *  @details Callback function should be user-defined elsewhere. There are some
 *  standart specifications.
 *
 */
void poweroffSetCallback(poweroff_callback cb, void *arg);

/**
 *  @brief Immidiate console shutdown.
 *  Do not call it without closing all stuff.
 *
 */
void poweroffShutdown(void);

/**
 *  @brief Change thread priority
 *
 *  @details The callback thread runs at priority
 *  <b>POWEROFF_THREAD_PRIORITY</b> by default. You can change the priority
 *  by calling this function after poweroffSetCallback().
 *  You can call this function before poweroffSetCallback() as well (but after
 *  poweroffInit() ). In that case poweroffSetCallback() will be registered with
 *  the provided priority.
 */
void poweroffChangeThreadPriority(int priority);

#ifdef __cplusplus
}
#endif

#endif /* __LIBPWROFF_H__ */
