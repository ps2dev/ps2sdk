#ifndef __SPI_SDCARD_DRIVER_CONFIG_H__
#define __SPI_SDCARD_DRIVER_CONFIG_H__

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>

#define SPISD_ASSERT(cond) \
    while (!cond) {};



#define SD_CRC7_MODE_ROM_TABLE 1 /* Table in ROM, no need to generate it */
#define SD_CRC7_MODE_RAM_TABLE 2 /* if you have no free 256B ROM or ROM is slow. Use crc7_generate_table() to init table */
#define SD_CRC7_MODE_RUNTIME   3 /* if you have no free ROM and RAM, but it is slow */


#if BUILD_BOOTLOADER == 1U
#define SD_CRC7_MODE (SD_CRC7_MODE_RUNTIME)
#else
#define SD_CRC7_MODE (SD_CRC7_MODE_RAM_TABLE)
#endif

#endif //__SPI_SDCARD_DRIVER_CONFIG_H__
