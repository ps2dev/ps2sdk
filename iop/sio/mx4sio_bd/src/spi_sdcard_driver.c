#include <bdm.h>
#include <stdint.h>
#include <thevent.h>
#include <thbase.h>

#include "spi_sdcard_driver.h"
#include "spi_sdcard_crc7.h"
#include "crc16.h"
#include "mx4sio.h"
#include "sio2regs.h"

#include "module_debug.h"

#define CMD0   (0  | 0x40)  /* Reset */
#define CMD1   (1  | 0x40)  /* Send Operator Condition - SEND_OP_COND */
#define CMD8   (8  | 0x40)  /* Send Interface Condition - SEND_IF_COND */
#define CMD9   (9  | 0x40)  /* Read CSD */
#define CMD10  (10 | 0x40)  /* Read CID */
#define CMD12  (12 | 0x40)  /* Stop data transmit */
#define CMD13  (13 | 0x40)  /* SEND_STATUS */
#define CMD16  (16 | 0x40)  /* Set block size, should return 0x00 */
#define CMD17  (17 | 0x40)  /* Read single block */
#define CMD18  (18 | 0x40)  /* Read multi block */
#define ACMD23 (23 | 0x40)  /* Prepare erase N-blocks before multi block write */
#define CMD24  (24 | 0x40)  /* Write single block */
#define CMD25  (25 | 0x40)  /* Write multi block */
#define ACMD41 (41 | 0x40)  /* should return 0x00 */
#define CMD55  (55 | 0x40)  /* should return 0x01 */
#define CMD58  (58 | 0x40)  /* Read OCR */
#define CMD59  (59 | 0x40)  /* CRC disable/enable, should return 0x00 */

#define SPISD_R1_IDLE_FLAG            (0x01)
#define SPISD_R1_ERASE_RESET_FLAG     (0x02)
#define SPISD_R1_ILLEGAL_CMD_FLAG     (0x04)
#define SPISD_R1_CMD_CRC_FLAG         (0x08)
#define SPISD_R1_ERASE_SEQ_ERROR_FLAG (0x10)
#define SPISD_R1_ADDR_ERROR_FLAG      (0x20)
#define SPISD_R1_PARAM_ERROR_FLAG     (0x40)
#define SPISD_R1_ZERO_FLAG            (0x80)

#define CMD_WAIT_RESP_TIMEOUT (100U)
#define WAIT_IDLE_TIMEOUT     (50U)
#define MAX_RETRIES            4

#define CMD_CRC_OFFSET        5
#define CRC7_SHIFT_MASK(crc7) ((crc7) << 1U | 1U)

/* globals */
spisd_t sdcard;

/* BDM interface */
struct block_device bd = {
    NULL,        /* priv */
    "sdc",       /* name */
    0,           /* devNr */
    0,           /* parNr */
    0x00,        /* parId */
    SECTOR_SIZE, /* sectorSize */
    0,           /* sectorOffset */
    0,           /* sectorCount */
    spisd_read,
    spisd_write,
    spisd_flush,
    spisd_stop };

/* NOTE: SIO2 does *NOT* allow for direct control of /CS line.
 * It's controlled by the SIO2 hardware and automatically asserted at the start 
 * of a transfer and deasserted at the end. This has lead to the need for some
 * conditions surrounding dummy writes to avoid timing disruption. */
static uint8_t spisd_send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t response = 0xFF;

    /* avoid disrupting CMD12 alignment */
    if (cmd != CMD12 && cmd != CMD0)
        mx_sio2_write_dummy();

    uint8_t packet[]       = {cmd, arg >> 24, arg >> 16, arg >> 8, arg, 0};
    packet[CMD_CRC_OFFSET] = CRC7_SHIFT_MASK(crc7(packet, CMD_CRC_OFFSET));

    /* begin SIO2 PIO TX transfer */
    mx_sio2_tx_pio(packet, sizeof(packet));

    /* discard extra data on CMD12 */
    if (cmd == CMD12)
        mx_sio2_write_dummy();

    /* wait for card to respond */
    response = mx_sio2_wait_not_equal(0xFF, CMD_WAIT_RESP_TIMEOUT);

    /* avoid disrupting alignment on commands with mutli byte responses */
    if (cmd != CMD9 && cmd != CMD10 && cmd != CMD13 && cmd != CMD18 && cmd != CMD25) {
        mx_sio2_write_dummy();
    }

    return response;
}

static uint8_t spisd_send_cmd_recv_data(uint8_t cmd, uint32_t arg, uint8_t *data, size_t size)
{
    uint8_t response = 0xFF;

    mx_sio2_write_dummy();

    uint8_t packet[]       = {cmd, arg >> 24, arg >> 16, arg >> 8, arg, 0};
    packet[CMD_CRC_OFFSET] = CRC7_SHIFT_MASK(crc7(packet, CMD_CRC_OFFSET));

    /* begin SIO2 PIO TX transfer */
    mx_sio2_tx_pio(packet, sizeof(packet));

    /* wait for card to respond */
    response = mx_sio2_wait_not_equal(0xFF, CMD_WAIT_RESP_TIMEOUT);
    if (response != 0xFF) {
        /* start SIO2 PIO RX transfer */
        mx_sio2_rx_pio(data, size);
    }

    mx_sio2_write_dummy();

    return response;
}

static uint8_t spisd_read_register(uint8_t *buff, uint32_t len)
{
    uint8_t results = SPISD_RESULT_ERROR;
    
    results = mx_sio2_wait_equal(0xFE, 2000);
    if (results == SPISD_RESULT_OK) {
        /* got read token, start SIO2 PIO RX transfer */
        mx_sio2_rx_pio(buff, len);
    } 

    return results;
}

/* TODO: use this to verify writes */
uint16_t spisd_read_status_register()
{
    uint16_t response;

    response = spisd_send_cmd(CMD13, 0) << 8;
    response |= mx_sio2_write_dummy();

    mx_sio2_write_dummy();

    return response;
}

int spisd_init_card()
{
    uint16_t timeout = WAIT_IDLE_TIMEOUT;
    uint8_t response = 0;
    uint8_t buffer[6];

    /* set baud to (400kHZ) for init */
    mx_sio2_set_baud(SIO2_BAUD_DIV_SLOW);

    /* send at least 74 dummy clocks */
    /*
    uint8_t dummy_clks[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    mx_sio2_tx_pio(dummy_clks, 10);*/

    for (int i = 0; i < 16; i++) {
        mx_sio2_write_dummy();
    }

    do {
        response = spisd_send_cmd(CMD0, 0);
        timeout--;
    } while ((response != SPISD_R1_IDLE_FLAG) && timeout > 0);

    if (!timeout) {
        M_DEBUG("ERROR: CMD0 returned 0x%x, exp: 0x1\n", response);
        return SPISD_RESULT_TIMEOUT;
    }

    /* send CMD8 with check pattern, store R3 response in buffer */
    response = spisd_send_cmd_recv_data(CMD8, 0x1AA, buffer, sizeof(buffer));

    /* if CMD8 response is idle, card is CSD v2 */  
    if (response == SPISD_R1_IDLE_FLAG) {
        
        /* valid check pattern */
        if (buffer[2] == 0x01 && buffer[3] == 0xAA) {          
            /* CMD55 / ACMD41 pairs */
            for (int i = 0; i < 0xFFF; i++) {
                response = spisd_send_cmd(CMD55, 0);
                if (response != 0x01) {
                    M_DEBUG("ERROR: CMD55 returned 0x%x, exp: 0x1\n", response);
                    return SPISD_RESULT_TIMEOUT;
                }

                response = spisd_send_cmd(ACMD41, 0x40000000);
                if (response == 0x00) {
                    break;
                }
            }

            if (response != 0x00) {
                M_DEBUG("ERROR: ACMD41 returned 0x%x, exp: 0x1\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            /* read OCR by CMD58 */
            response = spisd_send_cmd_recv_data(CMD58, 0, buffer, sizeof(buffer));
            if (response != 0x00) {
                M_DEBUG("ERROR: CMD58 returned 0x%x, exp 0x0\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            /* OCR -> CCS(bit30)  1: SDV2HC     0: SDV2 */
            sdcard.card_type = (buffer[0] & 0x40) ? CARD_TYPE_SDV2HC : CARD_TYPE_SDV2;

            /* set baud to 25MHz */
            mx_sio2_set_baud(SIO2_BAUD_DIV_FAST);

        } else {
            M_DEBUG("ERROR: CMD8 check pattern failed, got 0x%x, 0x%x\n", buffer[2], buffer[3]);
            return SPISD_RESULT_ERROR;
        }

    /* if CMD8 response is illegal, card is CSD v1 / MMC */
    } else if (response & SPISD_R1_ILLEGAL_CMD_FLAG) {
        M_DEBUG("CMD8 illegal, trying CSD v1.0 init\n");

        /* end of CMD8, dummy write */
        mx_sio2_write_dummy();

        /* CMD55 / ACMD41 pairs */
        for (int i = 0; i < 0xFFF; i++) {
            response = spisd_send_cmd(CMD55, 0);
            if (response != 0x01) {
                M_DEBUG("ERROR: CMD55 returned 0x%x, exp 0x1\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            response = spisd_send_cmd(ACMD41, 0);
            if (response == 0x00) {
                break;
            }
        }

        /* no response to CMD55 / ACMD41 means MMC card */
        if (response != 0x00) {
            for (int i = 0; i < 0xFFF; i++) {
                response = spisd_send_cmd(CMD1, 0);
                if (response == 0x00) {
                    break;
                }
            }

            if (response != 0x00) {
                M_DEBUG("ERROR: CMD1 returned 0x%x, exp 0x0\n", response);
                return SPISD_RESULT_TIMEOUT;
            }

            sdcard.card_type = CARD_TYPE_MMC;
            M_DEBUG("Card Type : MMC\r\n");
        } else {
            sdcard.card_type = CARD_TYPE_SDV1;
            M_DEBUG("Card Type : CSD v1\r\n");
        }
        
        /* set baud to 25MHz */
        mx_sio2_set_baud(SIO2_BAUD_DIV_FAST);

        /* CRC disable */
        response = spisd_send_cmd(CMD59, 0);

        if (response != 0x00) {
            M_DEBUG("Send CMD59 should return 0x00, response=0x%02x\r\n", response);
            return SPISD_RESULT_TIMEOUT;
        }

        /* set the block size */
        response = spisd_send_cmd(CMD16, 512);
        if (response != 0x00) {
            M_DEBUG("ERROR: CMD16 returned 0x%x, exp 0x0\n", response);
            return SPISD_RESULT_TIMEOUT;
        }
    
    /* CMD8 response invalid */
    } else {
        M_DEBUG("ERROR: CMD8 returned 0x%x, exp 0x1 or 0x4\n", response);
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

/* get card info and attach to bdm driver */
int spisd_get_card_info()
{
    /* 16 bytes + 2 byte CRC16 */
    uint8_t reg_data[18];
    
    /* send CMD9, read CSD */
    uint8_t result = spisd_send_cmd(CMD9, 0);
    if (result != 0x0) {
        return result;
    }

    result = spisd_read_register(sdcard.csd, sizeof(reg_data));
    
    /* dummy write between reg reads */
    mx_sio2_write_dummy();

    if (result != 0x0) {
        return SPISD_RESULT_ERROR;
    }

    /* send CMD10, read CID */
    result = spisd_send_cmd(CMD10, 0);

    if (result != 0x0) {
        return SPISD_RESULT_ERROR;
    }

    result = spisd_read_register(reg_data, sizeof(reg_data));
    
    mx_sio2_write_dummy();

    if (result != 0x0) {
        return SPISD_RESULT_ERROR;
    }

    sdcard.cid.ManufacturerID = reg_data[0];
    /* Byte 1 */
    sdcard.cid.OEM_AppliID = reg_data[1] << 8;
    /* Byte 2 */
    sdcard.cid.OEM_AppliID |= reg_data[2];
    /* Byte 3 */
    sdcard.cid.ProdName1 = reg_data[3] << 24;
    /* Byte 4 */
    sdcard.cid.ProdName1 |= reg_data[4] << 16;
    /* Byte 5 */
    sdcard.cid.ProdName1 |= reg_data[5] << 8;
    /* Byte 6 */
    sdcard.cid.ProdName1 |= reg_data[6];
    /* Byte 7 */
    sdcard.cid.ProdName2 = reg_data[7];
    /* Byte 8 */
    sdcard.cid.ProdRev = reg_data[8];
    /* Byte 9 */
    sdcard.cid.ProdSN = reg_data[9] << 24;
    /* Byte 10 */
    sdcard.cid.ProdSN |= reg_data[10] << 16;
    /* Byte 11 */
    sdcard.cid.ProdSN |= reg_data[11] << 8;
    /* Byte 12 */
    sdcard.cid.ProdSN |= reg_data[12];
    /* Byte 13 */
    sdcard.cid.Reserved1 |= (reg_data[13] & 0xF0) >> 4;
    /* Byte 14 */
    sdcard.cid.ManufactDate = (reg_data[13] & 0x0F) << 8;
    /* Byte 15 */
    sdcard.cid.ManufactDate |= reg_data[14];
    /* Byte 16 */
    sdcard.cid.CID_CRC   = (reg_data[15] & 0xFE) >> 1;

    struct t_csdVer1 *csdv1 = (struct t_csdVer1 *)sdcard.csd;
    struct t_csdVer2 *csdv2 = (struct t_csdVer2 *)sdcard.csd;
    
    /* CSD v1 - SDSC */
    if (csdv1->csd_structure == 0) {
        unsigned int c_size_mult = (csdv1->c_size_multHi << 1) | csdv1->c_size_multLo;
        unsigned int c_size      = (csdv1->c_sizeHi << 10) | (csdv1->c_sizeMd << 2) | csdv1->c_sizeLo;
        unsigned int blockNr     = (c_size + 1) << (c_size_mult + 2);
        unsigned int blockLen    = 1 << csdv1->read_bl_len;
        unsigned int capacity    = blockNr * blockLen;

        bd.sectorCount = capacity / 512;

    /* CSD v2 - SDHC, SDXC */
    } else if (csdv1->csd_structure == 1) {
        unsigned int c_size = (csdv2->c_sizeHi << 16) | (csdv2->c_sizeMd << 8) | csdv2->c_sizeLo;
        bd.sectorCount = (c_size + 1) * 1024;
    }

    M_PRINTF("%lu %u-byte logical blocks: (%luMB / %luMiB)\n", (u32)bd.sectorCount, bd.sectorSize, (u32)bd.sectorCount / ((1000 * 1000) / bd.sectorSize), (u32)bd.sectorCount / ((1024 * 1024) / bd.sectorSize));

    return SPISD_RESULT_OK;
}

int spisd_recover()
{
    int rv;
    /* flush 256 bytes */
    for (int i = 0; i < 64; i++)
        mx_sio2_rx_pio((void *)&rv, 4);

    if (spisd_init_card() != SPISD_RESULT_OK) {
        M_DEBUG("recovery failed to reinit card!\n");
        return SPISD_RESULT_ERROR;
    }

    if (spisd_get_card_info() != SPISD_RESULT_OK) {
        M_DEBUG("recovery failed to get card info!\n");
        return SPISD_RESULT_ERROR;
    }

    return SPISD_RESULT_OK;
}

/* read functions */
static int spisd_read_multi_begin(uint32_t sector)
{
    uint8_t results = SPISD_RESULT_ERROR;
    /* get idle */
    results = mx_sio2_wait_equal(0xFF, 4000);

    if (results == SPISD_RESULT_OK) {
        /* non SDHC/SDXC are addressed in 1-byte units */
        if (sdcard.card_type != CARD_TYPE_SDV2HC) {
            sector = sector << 9;
        }

        /* send CMD18 to being multi block read */
        results = spisd_send_cmd(CMD18, sector); 
        if (results == SPISD_RESULT_OK) {
            /* wait for first read token (0xFE) */
            results = mx_sio2_wait_equal(0xFE, 100000);
        }
    }

    return results;
}

static int spisd_read_multi_do(void *buffer, uint16_t count)
{
    /* setup DMA cmd struct */
    cmd.buffer              = buffer;
    cmd.sector_count        = count;
    cmd.sectors_transferred = 0;
    cmd.sectors_reversed    = 0;
    cmd.response            = SPISD_RESULT_OK;
    cmd.abort               = 0;

    /* start first DMA transfer */
    mx_sio2_start_rx_dma(buffer);

    /* process events from DMA completion interrupt */
    while (1) {
        uint32_t resbits;

        WaitEventFlag(sio2_event_flag, EF_SIO2_INTR_REVERSE | EF_SIO2_INTR_COMPLETE, 1, &resbits);

        if (resbits & EF_SIO2_INTR_REVERSE) {
            ClearEventFlag(sio2_event_flag, ~EF_SIO2_INTR_REVERSE);
            
            while (cmd.sectors_reversed < cmd.sectors_transferred && cmd.abort == 0) {
                void *buf = (uint32_t *)&cmd.buffer[cmd.sectors_reversed * SECTOR_SIZE]; 
                reverse_buffer(buf, SECTOR_SIZE / 4);

#ifdef CONFIG_USE_CRC16
                uint16_t crc_a = crc16(buf, SECTOR_SIZE);
                uint16_t crc_b = cmd.crc[cmd.sectors_reversed];
                if (crc_a != crc_b) {
                    // CRC mismatch:
                    // - Signal ISR to stop reading
                    // - Wait for complete event from ISR
                    M_DEBUG("CRC mismatch on sector %i, got: 0x%x, exp 0x%x\n", cmd.sectors_reversed, crc_b, crc_a);
                    cmd.abort = CMD_ERROR_CRC16_INVALID;
                }
#endif
                if (cmd.abort == 0)
                    cmd.sectors_reversed++;
            }
        }

        if (resbits & EF_SIO2_INTR_COMPLETE) {
            ClearEventFlag(sio2_event_flag, ~EF_SIO2_INTR_COMPLETE);
            break;
        }
    }

    return cmd.sectors_reversed;
}

static void spisd_read_multi_end()
{
    /* 0xFE token will be received in the ISR prior to this function being called
     * ensuring the start of CMD12 is aligned with the end of 0xFE
     * See 7.5.2.2 Stop Transmission Timing of the SD Physical Layer Specification
     * for more details */
    spisd_send_cmd(CMD12, 0);
}

/* write functions */
static int spisd_write_multi_begin(uint32_t sector, uint16_t count)
{
    uint8_t results = SPISD_RESULT_ERROR;
    
    /* get idle */
    results = mx_sio2_wait_equal(0xFF, 4000);

    if (results == SPISD_RESULT_OK) {
        /* non SDHC/SDXC are addressed in 1-byte units */
        if (sdcard.card_type != CARD_TYPE_SDV2HC) {
            sector = sector << 9;
        }

        /* issue ACMD23 to pre-erase sectors on non MMC cards */
        if (sdcard.card_type != CARD_TYPE_MMC) {
            results = spisd_send_cmd(CMD55, 0);
            results = spisd_send_cmd(ACMD23, count);
        }

        /* send CMD25 to begin multi block write */
        results = spisd_send_cmd(CMD25, sector);
    }

    mx_sio2_write_dummy();
    mx_sio2_write_dummy();
    mx_sio2_write_dummy();

    return results;
} 

static int spisd_write_multi_do(void* buffer, uint16_t count)
{
    uint32_t resbits;

    /* setup DMA cmd struct */
    cmd.buffer              = buffer;
    cmd.sector_count        = count;
    cmd.sectors_transferred = 0;
    cmd.sectors_reversed    = 0;
    cmd.response            = SPISD_RESULT_OK;
    cmd.abort               = 0;

    /* send initial write token */
    mx_sio2_write_byte(0xFC);
    
    /* start transfer */
    mx_sio2_start_tx_dma(buffer);

    /* wait for transfer to complete */
    WaitEventFlag(sio2_event_flag, EF_SIO2_INTR_COMPLETE, 1, &resbits);

    if (resbits & EF_SIO2_INTR_COMPLETE) {
        ClearEventFlag(sio2_event_flag, ~EF_SIO2_INTR_COMPLETE);
    }

    return cmd.sectors_transferred;
}

static int spisd_write_multi_end()
{
    uint8_t results = SPISD_RESULT_ERROR;

    /* issue stop transmission token */
    results = mx_sio2_write_byte(0xFD);
    if (results != 0x0) {
        mx_sio2_write_dummy();
        mx_sio2_write_dummy();

        /* give card time to finish programming */
        results = mx_sio2_wait_equal(0xFF, 0x800000);
        if (results == SPISD_RESULT_OK) {
            mx_sio2_write_dummy();
            mx_sio2_write_dummy();
        }
    } else {
        M_DEBUG("ERROR: failed to end write, 0xFD response 0x%x\n", results);
    }

    mx_sio2_write_dummy();

    return results;
}

/*
 * BDM interface:
 * - BDM -> "spisd" library
 */
int spisd_read(struct block_device *bd, uint64_t sector, void *buffer, uint16_t count)
{
    uint16_t sectors_left = count;
    uint16_t results = 0;
    uint16_t retries = 0;

    (void)bd;

    if (count == 0)
        return 0;

    M_DEBUG("%s: sector %i, count: %i \n", __FUNCTION__, (u32)sector, count);

    mx_sio2_lock(INTR_RX);

    while (sectors_left > 0 && retries < MAX_RETRIES) {

        /* issue CMD18 to begin transfer */
        results = spisd_read_multi_begin((uint32_t)sector);
        if (results != SPISD_RESULT_OK) {
            M_DEBUG("ERROR: failed to start multi-block read\n");
            break;
        }

        /* start reading blocks */
        results = spisd_read_multi_do(buffer, sectors_left);
        sectors_left = sectors_left - results;
        
        /* fail condition */
        if (sectors_left > 0) {
            buffer = (uint8_t *)buffer + (results * 512); 
            M_DEBUG("ERROR: failed to read all sectors, read:%i, abort:%i\n", sectors_left, cmd.abort);

            if (cmd.abort == CMD_ABORT_NO_READ_TOKEN) {
                /* this can only be resolved by resetting the card */
                if (spisd_recover() != SPISD_RESULT_OK) {
                    /* if recovery fails, do not try to continue  */
                    break;
                }
            }
        }
        /* send CMD12, end transfer */
        spisd_read_multi_end();

        retries++;
    }

    sdcard.used = 1;

    mx_sio2_unlock(INTR_RX);

    return count - sectors_left;
}

int spisd_write(struct block_device *bd, uint64_t sector, const void *buffer, uint16_t count)
{
    (void)bd;
    
    uint16_t sectors_left = count;
    uint16_t results = 0;
    uint16_t retries = 0;

    if (count == 0)
        return 0;

    M_DEBUG("%s: sector %i, count: %i \n", __FUNCTION__, (u32)sector, count);

    /* recast */
    void *write_buffer = (uint32_t*)buffer;

    /* pre-reverse the entire buffer */
    reverse_buffer(write_buffer, ((count * SECTOR_SIZE) / 4)); 

    mx_sio2_lock(INTR_TX);

    while (sectors_left > 0 && retries < MAX_RETRIES) {

        /* issue CMD25 to begin transfer */
        results = spisd_write_multi_begin(sector, count);
        if (results != SPISD_RESULT_OK) {
            M_DEBUG("ERROR: failed to start multi-block write\n");
            break;
        }

        /* start writing blocks */
        results = spisd_write_multi_do(write_buffer, sectors_left);
        sectors_left = sectors_left - results;

        /* fail condition */
        if (sectors_left > 0) {
            write_buffer = (uint8_t *)write_buffer + (results * 512); /* update buffer for next attempt */
            M_DEBUG("ERROR: failed to write all sectors, wrote: %i\n", results);
        }

        /* send 0xFD, end transfer */
        results = spisd_write_multi_end();
        if (results != SPISD_RESULT_OK) {
            M_DEBUG("ERROR: failed to end multi-block write\n");
            break;
        }

        retries++;
    }

    sdcard.used = 1;
    
    mx_sio2_unlock(INTR_TX);

    return count - sectors_left;
}

void spisd_flush(struct block_device *bd)
{
    (void)bd;
    return;
}

int spisd_stop(struct block_device *bd)
{   
    (void)bd;
    return 0;
}