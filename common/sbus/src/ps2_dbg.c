#include <tamtypes.h>
#include <ps2.h>
#include "ps2_sbus.h"
#include "sbus_priv.h"

#define DBG_CMD_PUTS (1)

u32 _dbg_cmd_dma_buf[128] __attribute__ ((aligned(128)));

#ifdef _EE

void sio_write(char *buf, int size)
{
    while(size--) { sio_putc(*(buf++)); }
}

void _sif2_cmd_puts(SIF2_CmdPkt *cmd, void *param)
{
    char *str;
    int left, toget;

    if(cmd->extra)
    {
//        sio_printf("getting %d extra bytes\n", cmd->extra_size);
        left = cmd->extra_size;

        while(left > 0)
        {
            toget = left;
            if(toget > sizeof(_dbg_cmd_dma_buf)) { toget = sizeof(_dbg_cmd_dma_buf); }

//            sio_printf("getting %d of %d bytes left\n", toget, left);

            SIF2_set_dma((u32) &_dbg_cmd_dma_buf, toget, PS2_DMA_TO_MEM);

            // wait for transfer to complete.
            SIF2_sync_dma();

//            sio_printf("sunk!\n");

            str = (char *) ((((u32) _dbg_cmd_dma_buf)) | 0x20000000);
            sio_write(str, toget);
//            str[toget] = '\0';
//            sio_puts(str);

            left -= toget;
        }
    }
    else
    {
        str = (char *) (((u32) cmd) + sizeof(SIF2_CmdPkt));
        sio_puts(str);
    }
}
#endif

int dbg_puts(char *str)
{
    SIF2_send_cmd(DBG_CMD_PUTS, str, strlen(str) + 1);
    return(0);
}

int dbg_init(void)
{

#ifdef _EE
    SIF2_set_cmd_handler(DBG_CMD_PUTS, &_sif2_cmd_puts, NULL);
#endif

    return(0);
}
