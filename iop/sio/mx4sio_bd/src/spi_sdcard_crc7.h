#ifndef __SPI_SDCARD_CRC7_H__
#define __SPI_SDCARD_CRC7_H__

/* Includes ------------------------------------------------------------------*/
#include <tamtypes.h>
#include <stddef.h>
#include "spi_sdcard_driver_config.h"
// Set

#if SD_CRC7_MODE == CRC7_MODE_RAM_TABLE
void crc7_generate_table(void);
#endif

uint8_t crc7(const uint8_t message[], size_t length);

#endif //__SPI_SDCARD_CRC7_H__
