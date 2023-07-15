#ifndef MX4SIO_H
#define MX4SIO_H

#include <stdint.h>

/*
 * Clock divider for 48MHz clock:
 * 1 = 48  MHz - Invalid setting
 * 2 = 24  MHz - Fastest usable speed
 * 3 = 16  MHz
 * 4 = 12  MHz
 * 5 =  9.6MHz
 * 6 =  6  MHz
 * ...
 * 0x78 = 400KHz - Initialization speed
 */

/* baud dividers */
#define SIO2_BAUD_DIV_SLOW 0x78
#define SIO2_BAUD_DIV_FAST 0x2

/* SIO2 can only transfer 256 bytes at a time */
#define SIO2_MAX_TRANSFER_SIZE 256

/* interrupt event flags */
#define EF_SIO2_INTR_REVERSE  0x00000100
#define EF_SIO2_INTR_COMPLETE 0x00000200

/* interrupt types */
#define INTR_NONE 0x0
#define INTR_RX   0x1
#define INTR_TX   0x2

/* mem slot 2 */
#define PORT_NR 3

// #define CONFIG_USE_CRC16

typedef struct dma_command_t
{
    uint8_t *buffer;
    uint16_t sector_count;
    volatile uint16_t sectors_transferred;
    uint16_t sectors_reversed;
#ifdef CONFIG_USE_CRC16
    uint16_t crc[512]; /*FIXME*/
#endif
    uint8_t response;
    volatile uint8_t abort;
} dma_command_t;

/* globals */
extern dma_command_t cmd;
extern int sio2_event_flag;
extern const uint8_t reverse_byte_LUT8[256];

/* MX SIO2 functions */
void mx_sio2_lock(uint8_t intr_type);
void mx_sio2_unlock(uint8_t intr_type);
void mx_sio2_set_baud(uint8_t baud);

uint8_t mx_sio2_write_byte(uint8_t byte);
uint8_t mx_sio2_write_dummy(void);

void mx_sio2_rx_pio(uint8_t *buffer, uint32_t size); /* PIO only used for sending commands */
void mx_sio2_tx_pio(uint8_t *buffer, uint32_t size);

void mx_sio2_start_rx_dma(uint8_t *buffer); /* DMA used for all other transfers */
void mx_sio2_start_tx_dma(uint8_t *buffer);

uint8_t mx_sio2_wait_equal(uint8_t value, uint32_t count);
uint8_t mx_sio2_wait_not_equal(uint8_t value, uint32_t count);
uint8_t mx_sio2_wait_equal_masked(uint8_t value, uint8_t mask, uint32_t count);

/* misc */
void reverse_buffer(uint32_t *buffer, uint32_t count);

#endif
