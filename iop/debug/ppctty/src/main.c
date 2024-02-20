#include <tamtypes.h>
#include <stdio.h>
#include <loadcore.h>
#include <excepman.h>
#include <ioman.h>
#include <fcntl.h>
#include "tty.h"

IRX_ID(MODNAME, 1, 0);

#define IS_DECKARD() \
({ \
    u16 iop_revision; \
    asm("mfc0 %0, $15\n" \
        : "=r"(iop_revision):); \
    iop_revision == 0x30; \
})

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
    if (!IS_DECKARD())
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
