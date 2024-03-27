#include <xparam.h>
#include <iop_regs.h>
#include <fcntl.h>
#include <loadfile.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int ResetDeckardXParams()
{
    /*
    This is needed in the case of IGR because the previous game might have changed XPARAMS and the new game might need the default one.
    */

    if (IOP_CPU_TYPE == IOP_TYPE_MIPSR3000)
        return -ENODECKARD;

    int fd;
    char params[30];
    memset(params, 0, 30);
    strncpy(params, "SLPS_123.45", 11);
    params[11] = 0; // Terminate param string.

    // For PS3/4 emu we skip default.
    fd = open("rom0:XPARAM2", O_RDONLY);
    if (fd >= 0) {
        close(fd);
        return APPLIED_XPARAM_EMU;
    }

    u32 default_values[] = {
        0x01F4, // PARAM_MDEC_DELAY_CYCLE
        0x07D0, // PARAM_SPU_INT_DELAY_LIMIT
        0x0023, // PARAM_SPU_INT_DELAY_PPC_COEFF
        0x07D0, // PARAM_SPU2_INT_DELAY_LIMIT
        0x0014, // PARAM_SPU2_INT_DELAY_PPC_COEFF
        0x0000, // PARAM_DMAC_CH10_INT_DELAY
        0x0001, // PARAM_CPU_DELAY
        0x0020, // PARAM_SPU_DMA_WAIT_LIMIT
        0x0000, // PARAM_GPU_DMA_WAIT_LIMIT
        0x0000, // PARAM_DMAC_CH10_INT_DELAY_DPC
        0x0000, // PARAM_CPU_DELAY_DPC
        0x0000, // PARAM_USB_DELAYED_INT_ENABLE
        0x0000, // PARAM_TIMER_LOAD_DELAY
        0x229C, // PARAM_SIO0_DTR_SCK_DELAY
        0x06EC, // PARAM_SIO0_DSR_SCK_DELAY_C
        0x06EC, // PARAM_SIO0_DSR_SCK_DELAY_M
        0x0001, // PARAM_MIPS_DCACHE_ON
        0x0090  // PARAM_CACHE_FLASH_CHANNELS
    };

    fd = open("rom0:XPARAM", O_RDONLY);
    if (fd >= 0) {
        close(fd);

        // Reset all the params to default.
        int i;
        for (i = 0; i < 0x12; i++) {
            sprintf(&params[12], "0X%02X", i);
            params[16] = 0;
            sprintf(&params[17], "0X%08X", default_values[i]);
            params[27] = 0;
            SifLoadModule("rom0:XPARAM", 28, params);
        }
        return APPLIED_XPARAM;
    }
    return -ENOXPARAMF;
}
