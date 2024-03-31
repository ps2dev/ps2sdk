#include <tamtypes.h>
#include <stdio.h>
#include <loadcore.h>
#include <excepman.h>
#include <ioman.h>
#include <iop_regs.h>
#include "tty.h"

IRX_ID(MODNAME, 1, 0);

volatile uint8_t *PPC_UART_TX = (volatile uint8_t *) 0x1F80380C;

// send a string to PPC TTY for output.
void tty_puts(const char *str)
{
    const char *p = str;
    while(p && *p) *PPC_UART_TX = *(p++);
}

int _start(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    if (IOP_CPU_TYPE == IOP_TYPE_MIPSR3000)
    {
        printf(MODNAME ": THIS PS2 IS NOT DECKARD\n");
        return MODULE_NO_RESIDENT_END;
    }
    
    if(tty_init() != 0)
    {
        printf("Failed initializing PPC TTY system!\n");
        return MODULE_NO_RESIDENT_END;
    }

    FlushIcache();
    FlushDcache();

    printf("IOP PPC TTY installed!\n");

    return MODULE_RESIDENT_END;
}
