#ifndef XPARAM_H
#define XPARAM_H

#include <stdint.h>

typedef enum xparam_types {
    PARAM_MDEC_DELAY_CYCLE = (0x00),
    PARAM_SPU_INT_DELAY_LIMIT = (0x01),
    PARAM_SPU_INT_DELAY_PPC_COEFF = (0x02),
    PARAM_SPU2_INT_DELAY_LIMIT = (0x03),
    PARAM_SPU2_INT_DELAY_PPC_COEFF = (0x04),
    PARAM_DMAC_CH10_INT_DELAY = (0x05),
    PARAM_CPU_DELAY = (0x06),
    PARAM_SPU_DMA_WAIT_LIMIT = (0x07),
    PARAM_GPU_DMA_WAIT_LIMIT = (0x08),
    PARAM_DMAC_CH10_INT_DELAY_DPC = (0x09),
    PARAM_CPU_DELAY_DPC = (0x0A),
    PARAM_USB_DELAYED_INT_ENABLE = (0x0B),
    PARAM_TIMER_LOAD_DELAY = (0x0C),
    PARAM_SIO0_DTR_SCK_DELAY = (0x0D),
    PARAM_SIO0_DSR_SCK_DELAY_C = (0x0E),
    PARAM_SIO0_DSR_SCK_DELAY_M = (0x0F),
    PARAM_MIPS_DCACHE_ON = (0x10),
    PARAM_CACHE_FLASH_CHANNELS = (0x11)
} xparam_types_t;
#define XPARAM_PARAMS_AMMOUNT (PARAM_CACHE_FLASH_CHANNELS+1)

typedef struct xparam {
    char name[12];
    uint32_t param;
    uint32_t value;
} xparam_t;

extern const xparam_t XPARAMS[];

#define GET_XPARAM_DB_SIZE() (sizeof(*XPARAMS) / sizeof(XPARAMS[0]))

#endif