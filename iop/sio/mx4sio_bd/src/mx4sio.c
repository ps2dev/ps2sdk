#include <bdm.h>
#include <dmacman.h>
#include <intrman.h>
#include <loadcore.h>
#include <thbase.h>
#include <thevent.h>
#include <thsemap.h>
#include <xsio2man.h>

#include "mx4sio.h"
#include "crc16.h"
#include "ioplib.h"
#include "sio2man_hook.h"
#include "sio2regs.h"
#include "spi_sdcard_driver.h"

#include "module_debug.h"

IRX_ID("mx4sio", 1, 2);

/* globals */
dma_command_t cmd;
int sio2_event_flag;

static int sd_detect_thread_id = -1;
static sio2_transfer_data_t global_td;
static uint8_t sio2_current_baud = SIO2_BAUD_DIV_SLOW;
static uint32_t sio2_save_crtl;

/* ISR triggered by the completion of DMA transfer: SIO2 RX FIFO -> MEM */
int mx_sio2_dma_isr_rx(void *arg)
{
    int ef     = *(int *)arg;
    int eflags = EF_SIO2_INTR_REVERSE;

    /* clear SIO2 stat reg */
    inl_sio2_stat_set(inl_sio2_stat_get());

    /* wait for PIO portion of transfer to complete */
    while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
        ;

#ifdef CONFIG_USE_CRC16
    /* get CRC16 bytes from RX FIFO */
    cmd.crc[cmd.sectors_transferred] = reverse_byte_LUT8[inl_sio2_data_in()] << 8;
    cmd.crc[cmd.sectors_transferred] |= reverse_byte_LUT8[inl_sio2_data_in()];
#else
    inl_sio2_data_in();
    inl_sio2_data_in();
#endif

    /* NOTE: Some cards respond with 0xFE immediately after the CRC16, others do not.
     * Try to get 0xFE regardless of whether the transfers complete as it's
     * needed for correct alignment of CMD12 (STOP_TRANSMISSION) later */
    if (inl_sio2_data_in() != 0xFE) {
        cmd.response = mx_sio2_wait_equal(0xFE, READ_TOKEN_TIMEOUT);
    } else {
        cmd.response = SPISD_RESULT_OK;
    }

    cmd.sectors_transferred++;

    if (cmd.sectors_transferred < cmd.sector_count && cmd.abort == 0) {

        if (cmd.response == SPISD_RESULT_OK) {
            /* start next DMA transfer */
            mx_sio2_start_rx_dma(&cmd.buffer[cmd.sectors_transferred * SECTOR_SIZE]);
            /* notify thread data is ready to be reversed */
            iSetEventFlag(ef, eflags);
            /* return from ISR */
            return 1;
        }

        cmd.abort = CMD_ABORT_NO_READ_TOKEN;
    }

    /* done or error, notify thread */
    eflags |= EF_SIO2_INTR_COMPLETE;
    iSetEventFlag(ef, eflags);

    return 1;
}

/* ISR triggered by the completetion of DMA transfer: MEM -> SIO2 TX FIFO */
int mx_sio2_dma_isr_tx(void *arg)
{
    int ef     = *(int *)arg;
    int eflags = 0;

    /* clear SIO2 stat reg */
    inl_sio2_stat_set(inl_sio2_stat_get());

    /* SIO2 requires all data to be sent to be present in the TX FIFO
     * prior to starting the transfer. As such, SIO2 will not
     * have completed the transfer prior to reaching this ISR */

    /* wait for transfer to complete */
    while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
        ;

    /* CRC16 not yet implemented for writes */
    mx_sio2_write_dummy();
    mx_sio2_write_dummy();

    cmd.sectors_transferred++;

    /* get data response token */
    /* 0101 = data accepted */
    /* 1011 = data rejected, CRC error */
    /* 1101 = data rejected, write error */
    cmd.response = mx_sio2_wait_equal_masked(0x5, 0x1F, 10);
    if (cmd.response != 0x5) {
        M_DEBUG("ERROR: write data rejected, token 0x%x\n", cmd.response);
        cmd.abort = 1;
    }

    /* wait for card to finish programming */
    cmd.response = mx_sio2_wait_equal(0xFF, 0x80000);
    if (cmd.response != SPISD_RESULT_OK) {
        M_DEBUG("ERROR: failed to finish programming\n");
        cmd.abort = 1;
    }

    if (cmd.sectors_transferred < cmd.sector_count && cmd.abort == 0) {
        /* send write token */
        mx_sio2_write_byte(0xFC);
        /* start next DMA transfer */
        mx_sio2_start_tx_dma(&cmd.buffer[cmd.sectors_transferred * SECTOR_SIZE]);
        /* return from ISR */
        return 1;
    }

    /* done or error, notify thread */
    eflags = EF_SIO2_INTR_COMPLETE;
    iSetEventFlag(ef, eflags);

    return 1;
}

void mx_sio2_init_ports(sio2_transfer_data_t *td)
{
    for (uint8_t i = 0; i < 4; i++) {
        inl_sio2_portN_ctrl1_set(i, td->port_ctrl1[i]);
        inl_sio2_portN_ctrl2_set(i, td->port_ctrl2[i]);
    }
}

void mx_sio2_init_td(sio2_transfer_data_t *td)
{
    for (uint8_t i = 0; i < 4; i++) {
        td->port_ctrl1[i] = 0;
        td->port_ctrl2[i] = 0;
    }

    td->port_ctrl1[PORT_NR] =
        PCTRL0_ATT_LOW_PER(0x5) |
        PCTRL0_ATT_MIN_HIGH_PER(0x5) |
        PCTRL0_BAUD0_DIV(0x78) |             /* BAUD0 is unused */
        PCTRL0_BAUD1_DIV(sio2_current_baud); /* BAUD1 is used for every transfer */

    td->port_ctrl2[PORT_NR] =
        PCTRL1_ACK_TIMEOUT_PER(0x12C) |
        PCTRL1_INTER_BYTE_PER(0x0) |
        PCTRL1_UNK24(0x0) |
        PCTRL1_IF_MODE_SPI_DIFF(0x0);
}

void mx_sio2_lock(uint8_t intr_type)
{
    int state;

    /* lock sio2man driver so we can use it exclusively */
    sio2man_hook_sio2_lock();

    /* save ctrl state */
    sio2_save_crtl = inl_sio2_ctrl_get();

    /* we're in control, setup the ports for our use */
    mx_sio2_init_ports(&global_td);

    /* enable DMA interrupts */
    CpuSuspendIntr(&state);

    if (intr_type == INTR_RX) {
        RegisterIntrHandler(IOP_IRQ_DMA_SIO2_OUT, 1, mx_sio2_dma_isr_rx, &sio2_event_flag);
        EnableIntr(IOP_IRQ_DMA_SIO2_OUT);
    }

    if (intr_type == INTR_TX) {
        RegisterIntrHandler(IOP_IRQ_DMA_SIO2_IN, 1, mx_sio2_dma_isr_tx, &sio2_event_flag);
        EnableIntr(IOP_IRQ_DMA_SIO2_IN);
    }

    CpuResumeIntr(state);
}

void mx_sio2_unlock(uint8_t intr_type)
{
    int state;
    int res;

    /* disable DMA interrupts */
    CpuSuspendIntr(&state);

    if (intr_type == INTR_RX)
        DisableIntr(IOP_IRQ_DMA_SIO2_OUT, &res);

    if (intr_type == INTR_TX)
        DisableIntr(IOP_IRQ_DMA_SIO2_IN, &res);

    CpuResumeIntr(state);

    /* restore ctrl state, and reset STATE + FIFOS */
    inl_sio2_ctrl_set(sio2_save_crtl | 0xc);

    /* unlock sio2man driver */
    sio2man_hook_sio2_unlock();
}

void mx_sio2_set_baud(uint8_t baud)
{
    sio2_current_baud = baud;

    mx_sio2_init_td(&global_td);
    mx_sio2_init_ports(&global_td);
}

uint8_t mx_sio2_write_byte(uint8_t byte)
{
    /* reset SIO2 + FIFO pointers, disable interrupts */
    inl_sio2_ctrl_set(0x0bc);

    inl_sio2_regN_set(0,
                      TR_CTRL_PORT_NR(PORT_NR) |
                          TR_CTRL_PAUSE(0) |
                          TR_CTRL_TX_MODE_PIO_DMA(0) |
                          TR_CTRL_RX_MODE_PIO_DMA(0) |
                          TR_CTRL_NORMAL_TR(1) |
                          TR_CTRL_SPECIAL_TR(0) |
                          TR_CTRL_BAUD_DIV(1) |
                          TR_CTRL_WAIT_ACK_FOREVER(0) |
                          TR_CTRL_TX_DATA_SZ(1) |
                          TR_CTRL_RX_DATA_SZ(1));
    inl_sio2_regN_set(1, 0);

    /* put byte in TX FIFO */
    inl_sio2_data_out(reverse_byte_LUT8[byte]);

    /* start queue exec */
    inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

    /* wait for completion */
    while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
        ;

#ifdef DEBUG_VERBOSE
    uint8_t rx = reverse_byte_LUT8[inl_sio2_data_in()];
    M_DEBUG("W:0x%x, R:0x%x\n", byte, rx);
    return rx;
#endif

    /* get byte from RX FIFO */
    return reverse_byte_LUT8[inl_sio2_data_in()];
}

uint8_t mx_sio2_write_dummy(void)
{
    /* reset SIO2 + FIFO pointers, disable interrupts */
    inl_sio2_ctrl_set(0x0bc);

    /* add transfer to queue */
    inl_sio2_regN_set(0,
                      TR_CTRL_PORT_NR(PORT_NR) |
                          TR_CTRL_PAUSE(0) |
                          TR_CTRL_TX_MODE_PIO_DMA(0) |
                          TR_CTRL_RX_MODE_PIO_DMA(0) |
                          TR_CTRL_NORMAL_TR(1) |
                          TR_CTRL_SPECIAL_TR(0) |
                          TR_CTRL_BAUD_DIV(1) |
                          TR_CTRL_WAIT_ACK_FOREVER(0) |
                          TR_CTRL_TX_DATA_SZ(1) |
                          TR_CTRL_RX_DATA_SZ(1));
    inl_sio2_regN_set(1, 0);

    /* put byte in TX FIFO */
    inl_sio2_data_out(0xFF);

    /* start queue exec */
    inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

    /* wait for completion */
    while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
        ;

#ifdef DEBUG_VERBOSE
    uint8_t rx = reverse_byte_LUT8[inl_sio2_data_in()];
    M_DEBUG("R:0x%x\n", rx);
    return rx;
#endif

    /* get byte from RX FIFO */
    return reverse_byte_LUT8[inl_sio2_data_in()];
}

/* 1.440uS delay between bytes with clk div 0x2 */
uint8_t mx_sio2_wait_equal(uint8_t value, uint32_t count)
{
    uint8_t exp_byte = reverse_byte_LUT8[value];
    uint8_t in_byte  = 0;

    while (count > 0 && in_byte != exp_byte) {
        /* reset SIO2 + FIFO pointers, disable interrupts */
        inl_sio2_ctrl_set(0x0bc);

        inl_sio2_regN_set(0,
                          TR_CTRL_PORT_NR(PORT_NR) |
                              TR_CTRL_PAUSE(0) |
                              TR_CTRL_TX_MODE_PIO_DMA(0) |
                              TR_CTRL_RX_MODE_PIO_DMA(0) |
                              TR_CTRL_NORMAL_TR(1) |
                              TR_CTRL_SPECIAL_TR(0) |
                              TR_CTRL_BAUD_DIV(1) |
                              TR_CTRL_WAIT_ACK_FOREVER(0) |
                              TR_CTRL_TX_DATA_SZ(0) |
                              TR_CTRL_RX_DATA_SZ(1));
        inl_sio2_regN_set(1, 0);

        /* start queue exec */
        inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

        /* wait for completion */
        while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
            ;

        in_byte = inl_sio2_data_in();

#ifdef DEBUG_VERBOSE
        M_DEBUG("WE: 0x%x, EX: 0x%x\n", reverse_byte_LUT8[in_byte], value);
#endif

        count--;
    }

    return (exp_byte != in_byte) ? SPISD_RESULT_ERROR : SPISD_RESULT_OK;
}

uint8_t mx_sio2_wait_not_equal(uint8_t value, uint32_t count)
{
    uint8_t exp_byte = reverse_byte_LUT8[value];
    uint8_t in_byte  = exp_byte;

    while (count > 0 && in_byte == exp_byte) {
        /* reset SIO2 + FIFO pointers, disable interrupts */
        inl_sio2_ctrl_set(0x0bc);

        /* add transfer to queue */
        inl_sio2_regN_set(0,
                          TR_CTRL_PORT_NR(PORT_NR) |
                              TR_CTRL_PAUSE(0) |
                              TR_CTRL_TX_MODE_PIO_DMA(0) |
                              TR_CTRL_RX_MODE_PIO_DMA(0) |
                              TR_CTRL_NORMAL_TR(1) |
                              TR_CTRL_SPECIAL_TR(0) |
                              TR_CTRL_BAUD_DIV(1) |
                              TR_CTRL_WAIT_ACK_FOREVER(0) |
                              TR_CTRL_TX_DATA_SZ(0) |
                              TR_CTRL_RX_DATA_SZ(1));
        inl_sio2_regN_set(1, 0);

        /* start queue exec */
        inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

        /* wait for completion */
        while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
            ;

        /* get byte from RX FIFO */
        in_byte = inl_sio2_data_in();

#ifdef DEBUG_VERBOSE
        M_DEBUG("WNE: 0x%x, EX: !0x%x\n", reverse_byte_LUT8[in_byte], value);
#endif
        count--;
    }

    return reverse_byte_LUT8[in_byte];
}

uint8_t mx_sio2_wait_equal_masked(uint8_t value, uint8_t mask, uint32_t count)
{
    uint8_t exp_byte = reverse_byte_LUT8[value];
    uint8_t rev_mask = reverse_byte_LUT8[mask];
    uint8_t in_byte  = 0;

    while (count > 0 && in_byte != exp_byte) {
        /* reset SIO2 + FIFO pointers, disable interrupts */
        inl_sio2_ctrl_set(0x0bc);

        inl_sio2_regN_set(0,
                          TR_CTRL_PORT_NR(PORT_NR) |
                              TR_CTRL_PAUSE(0) |
                              TR_CTRL_TX_MODE_PIO_DMA(0) |
                              TR_CTRL_RX_MODE_PIO_DMA(0) |
                              TR_CTRL_NORMAL_TR(1) |
                              TR_CTRL_SPECIAL_TR(0) |
                              TR_CTRL_BAUD_DIV(1) |
                              TR_CTRL_WAIT_ACK_FOREVER(0) |
                              TR_CTRL_TX_DATA_SZ(0) |
                              TR_CTRL_RX_DATA_SZ(1));
        inl_sio2_regN_set(1, 0);

        /* start queue exec */
        inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

        /* wait for completion */
        while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
            ;

        in_byte = inl_sio2_data_in();

#ifdef DEBUG_VERBOSE
        M_DEBUG("WEM: 0x%x M:\n", reverse_byte_LUT8[in_byte], reverse_byte_LUT8[in_byte & rev_mask]);
#endif
        in_byte = in_byte & rev_mask;

        count--;
    }

    return reverse_byte_LUT8[in_byte];
}

void mx_sio2_rx_pio(uint8_t *buffer, uint32_t size)
{
    uint32_t transfer_size;
#ifdef DEBUG_VERBOSE
    uint8_t *buffer_start = buffer;
#endif
    while (size > 0) {
        /* SIO2 can only transfer 256 bytes at a time */
        transfer_size = size > SIO2_MAX_TRANSFER_SIZE ? SIO2_MAX_TRANSFER_SIZE : size;

        /* reset SIO2 + FIFO pointers, disable interrupts */
        inl_sio2_ctrl_set(0x0bc);

        /* add transfer to queue */
        inl_sio2_regN_set(0,
                          TR_CTRL_PORT_NR(PORT_NR) |
                              TR_CTRL_PAUSE(0) |
                              TR_CTRL_TX_MODE_PIO_DMA(0) |
                              TR_CTRL_RX_MODE_PIO_DMA(0) |
                              TR_CTRL_NORMAL_TR(1) |
                              TR_CTRL_SPECIAL_TR(0) |
                              TR_CTRL_BAUD_DIV(1) |
                              TR_CTRL_WAIT_ACK_FOREVER(0) |
                              TR_CTRL_TX_DATA_SZ(0) |
                              TR_CTRL_RX_DATA_SZ(transfer_size));
        inl_sio2_regN_set(1, 0);

        /* start queue exec */
        inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

        /* wait for completion */
        while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
            ;

        /* PIO: IOP <- SIO2 */
        for (int i = 0; i < transfer_size; i++) {
            *buffer++ = reverse_byte_LUT8[inl_sio2_data_in()];
        }

#ifdef DEBUG_VERBOSE
        for (int i = 0; i < transfer_size; i++) {
            M_DEBUG("W:0xFF, R:0x%x\n", buffer_start[i]);
        }
#endif

        size -= transfer_size;
    }
}

void mx_sio2_tx_pio(uint8_t *buffer, uint32_t size)
{
    uint32_t transfer_size;

#ifdef DEBUG_VERBOSE
    uint8_t *buffer_start = buffer;
#endif

    while (size > 0) {
        /* SIO2 can only transfer 256 bytes at a time */
        transfer_size = size > SIO2_MAX_TRANSFER_SIZE ? SIO2_MAX_TRANSFER_SIZE : size;

        /* reset SIO2 + FIFO pointers, disable interrupts */
        inl_sio2_ctrl_set(0x0bc);

        /* add transfer to queue */
        inl_sio2_regN_set(0,
                          TR_CTRL_PORT_NR(PORT_NR) |
                              TR_CTRL_PAUSE(0) |
                              TR_CTRL_TX_MODE_PIO_DMA(0) |
                              TR_CTRL_RX_MODE_PIO_DMA(0) |
                              TR_CTRL_NORMAL_TR(1) |
                              TR_CTRL_SPECIAL_TR(0) |
                              TR_CTRL_BAUD_DIV(1) |
                              TR_CTRL_WAIT_ACK_FOREVER(0) |
                              TR_CTRL_TX_DATA_SZ(transfer_size) |
#ifdef DEBUG_VERBOSE
                              TR_CTRL_RX_DATA_SZ(transfer_size));
#else
                              TR_CTRL_RX_DATA_SZ(0));
#endif

        inl_sio2_regN_set(1, 0);

        /* PIO: IOP -> SIO2 */
        for (int i = 0; i < transfer_size; i++) {
            inl_sio2_data_out(reverse_byte_LUT8[*buffer++]);
        }

        /* start queue exec */
        inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);

        /* wait for completion */
        while ((inl_sio2_stat6c_get() & (1 << 12)) == 0)
            ;

#ifdef DEBUG_VERBOSE
        for (int i = 0; i < transfer_size; i++) {
            M_DEBUG("W:0x%x, R:0x%x\n", buffer_start[i], reverse_byte_LUT8[inl_sio2_data_in()]);
        }
#endif
        size -= transfer_size;
    }
}

void mx_sio2_start_rx_dma(uint8_t *buffer)
{
    /* DMA: 256 bytes */
    const uint32_t tr_ctrl_dma =
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(1) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(0) |
        TR_CTRL_RX_DATA_SZ(0x100) |
        TR_CTRL_BAUD_DIV(1) |
        TR_CTRL_WAIT_ACK_FOREVER(0);

    /* PIO: CRC16 (2 bytes) + 0xFE token (1 byte) */
    const uint32_t tr_ctrl_pio =
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(0) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(0) |
        TR_CTRL_RX_DATA_SZ(3) |
        TR_CTRL_BAUD_DIV(1) |
        TR_CTRL_WAIT_ACK_FOREVER(0);

    /* reset SIO2 + FIFO pointers, disable interrupts */
    inl_sio2_ctrl_set(0x0bc);

    /* add transfers to queue */
    inl_sio2_regN_set(0, tr_ctrl_dma | TR_CTRL_PORT_NR(PORT_NR));
    inl_sio2_regN_set(1, tr_ctrl_dma | TR_CTRL_PORT_NR(PORT_NR));
    inl_sio2_regN_set(2, tr_ctrl_pio | TR_CTRL_PORT_NR(PORT_NR));
    inl_sio2_regN_set(3, 0);

    /* enable dmac transfer */
    dmac_request(IOP_DMAC_SIO2out, buffer, 0x100 >> 2, 2, DMAC_TO_MEM);
    dmac_transfer(IOP_DMAC_SIO2out);

    /* start queue exec */
    inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);
}

void mx_sio2_start_tx_dma(uint8_t *buffer)
{
    /* DMA: 256 bytes */
    const uint32_t tr_ctrl_dma =
        TR_CTRL_PAUSE(0) |
        TR_CTRL_TX_MODE_PIO_DMA(1) |
        TR_CTRL_RX_MODE_PIO_DMA(0) |
        TR_CTRL_NORMAL_TR(1) |
        TR_CTRL_SPECIAL_TR(0) |
        TR_CTRL_TX_DATA_SZ(0x100) |
        TR_CTRL_RX_DATA_SZ(0) |
        TR_CTRL_BAUD_DIV(1) |
        TR_CTRL_WAIT_ACK_FOREVER(0);

    /* reset SIO2 + FIFO pointers, disable interrupts */
    inl_sio2_ctrl_set(0x0bc);

    /* add transfers to queue */
    inl_sio2_regN_set(0, tr_ctrl_dma | TR_CTRL_PORT_NR(PORT_NR));
    inl_sio2_regN_set(1, tr_ctrl_dma | TR_CTRL_PORT_NR(PORT_NR));
    inl_sio2_regN_set(2, 0);
    inl_sio2_regN_set(3, 0);

    /* enable dmac transfer */
    dmac_request(IOP_DMAC_SIO2in, buffer, 0x100 >> 2, 2, DMAC_FROM_MEM);
    dmac_transfer(IOP_DMAC_SIO2in);

    /* start queue exec */
    inl_sio2_ctrl_set(inl_sio2_ctrl_get() | 1);
}

static void sd_detect()
{
    uint16_t results;

    mx_sio2_lock(INTR_NONE);

    if (sdcard.initialized == 0) {
        M_PRINTF("Trying to init card\n");
        /* bring card up from identification mode to data-transfer mode */
        if (spisd_init_card() == SPISD_RESULT_OK) {

            /* get card capacity and attach to BDM */
            if (spisd_get_card_info() == SPISD_RESULT_OK) {
                bdm_connect_bd(&bd);
                sdcard.initialized = 1;
            }
        }
    } else {
        /* try to detect card removal by requesting card status (CMD13) */
        results = spisd_read_status_register();
        /* comparing without a mask might be a bit overkill */
        if (results != 0x0) {
            /* try to recover */
            results = spisd_recover();
            /* maybe add a var to keep track of capacity */
            if ((sdcard.initialized == 1) && (results != SPISD_RESULT_OK)) {
                /* recovery failed, disconnect from BDM */
                M_DEBUG("Recovery failed, disconnecting from bdm.\n");
                bdm_disconnect_bd(&bd);
                sdcard.initialized = 0;
            }
        }
    }

    mx_sio2_unlock(INTR_NONE);
}

static void sd_detect_thread(void *arg)
{
    (void)arg;

    M_PRINTF("card detection thread running\n");

    while (1) {
        DelayThread(1000 * 1000);

        /* try to detect card removal if it hasn't been used recently */
        if (sdcard.used == 0)
            sd_detect();
        sdcard.used = 0;
    }
}


/* Maximus32's C r3000 optimized byte reversal */
/* 58-59uS avg on DECKARD */
#pragma GCC push_options
#pragma GCC optimize("-O3")
inline void reverse_buffer(uint32_t *buffer, uint32_t count)
{
    const uint32_t mask0F = 0x0F0F0F0F;
    const uint32_t maskF0 = 0xF0F0F0F0;
    const uint32_t mask33 = 0x33333333;
    const uint32_t maskCC = 0xCCCCCCCC;
    const uint32_t mask55 = 0x55555555;
    const uint32_t maskAA = 0xAAAAAAAA;
    uint32_t n;

#pragma GCC unroll 2
    for (int i = 0; i < count; i++) {
        n = buffer[i];
        n = ((n & maskF0) >> 4) | ((n & mask0F) << 4);
        n = ((n & maskCC) >> 2) | ((n & mask33) << 2);
        n = ((n & maskAA) >> 1) | ((n & mask55) << 1);

        buffer[i] = n;
    }
}
#pragma GCC pop_options

/* LUT for single byte reversal */
const uint8_t reverse_byte_LUT8[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff};

/* module */
int module_start(int argc, char *argv[])
{
    iop_library_t *lib_modload;
    iop_event_t event;
    iop_thread_t thread;
    int rv;

#ifndef MINI_DRIVER
    int i;

    M_PRINTF("Starting module\n");
    for (i = 0; i < argc; i++)
        M_PRINTF(" - argv[%d] = %s\n", i, argv[i]);
#else
    (void)argc;
    (void)argv;
#endif

    /* create default transfer descriptor */
    mx_sio2_init_td(&global_td);

    /* create event flag */
    event.attr      = 2;
    event.option    = 0;
    event.bits      = 0;
    sio2_event_flag = CreateEventFlag(&event);
    if (sio2_event_flag < 0) {
        M_PRINTF("ERROR: CreateEventFlag returned %d\n", sio2_event_flag);
        goto error1;
    }

    rv = sio2man_hook_init();
    if (rv < 0) {
        M_PRINTF("ERROR: sio2man_hook_init returned %d\n", rv);
        goto error2;
    }

    /* Just in case sio2man was not loaded, we initialize the dmac channels too */
    dmac_ch_set_dpcr(IOP_DMAC_SIO2in, 3);
    dmac_ch_set_dpcr(IOP_DMAC_SIO2out, 3);
    dmac_enable(IOP_DMAC_SIO2in);
    dmac_enable(IOP_DMAC_SIO2out);

    /* After a reboot the SD will always respond with:
     * - 0xff 0xff 0xc1 0x3f
     * - followed by an infinite amount of 0xff
     */
    mx_sio2_lock(INTR_NONE);
    mx_sio2_rx_pio((void *)&rv, 4);
    mx_sio2_unlock(INTR_NONE);

    /* create SD card detection thread */
    thread.attr      = TH_C;
    thread.thread    = sd_detect_thread;
    thread.option    = 0;
    thread.priority  = USER_LOWEST_PRIORITY;
    thread.stacksize = 0x1000; // 4KiB
    rv = sd_detect_thread_id = CreateThread(&thread);
    if (rv < 0) {
        M_PRINTF("ERROR: CreateThread returned %d\n", rv);
        goto error3;
    }

    /* Start thread */
    rv = StartThread(sd_detect_thread_id, NULL);
    if (rv < 0) {
        M_PRINTF("ERROR: StartThread returned %d\n", rv);
        goto error4;
    }

    lib_modload = ioplib_getByName("modload");
    if (lib_modload != NULL) {
        M_DEBUG("modload 0x%x detected\n", lib_modload->version);
        // Newer modload versions allow modules to be unloaded
        // Let modload know we support unloading
        if (lib_modload->version > 0x102)
            return MODULE_REMOVABLE_END;
    } else {
        M_DEBUG("modload not detected!\n");
    }

    return MODULE_RESIDENT_END;

error4:
    DeleteThread(sd_detect_thread_id);
error3:
    sio2man_hook_deinit();
error2:
    DeleteEventFlag(sio2_event_flag);
error1:
    return MODULE_NO_RESIDENT_END;
}

int module_stop(int argc, char *argv[])
{
#ifndef MINI_DRIVER
    int i;

    M_PRINTF("Stopping module\n");
    for (i = 0; i < argc; i++)
        M_PRINTF(" - argv[%d] = %s\n", i, argv[i]);
#else
    (void)argc;
    (void)argv;
#endif

    DeleteThread(sd_detect_thread_id);
    sio2man_hook_deinit();
    DeleteEventFlag(sio2_event_flag);

    return MODULE_NO_RESIDENT_END;
}

int _start(int argc, char *argv[])
{
    M_PRINTF("MX4SIO v1.2\n");

    if (argc >= 0)
        return module_start(argc, argv);
    else
        return module_stop(-argc, argv);
}
