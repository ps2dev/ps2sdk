#include "irx_imports.h"

#define MODNAME "dogbait"
#define MAJOR 1
#define MINOR 0

#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

IRX_ID(MODNAME, MAJOR, MINOR);
char rdata[16];
char wdata[2] = {0x42, (char)(1 << 8)};

//the loop waiting was made to mirror what rom0:DAEMON did
void bait(void*)
{
  int x;
  printf("DOGBAIT v%d.%d   by El_isra\n", MAJOR, MINOR);
  do {
#ifdef DEBUG
    x =
#endif
    //thanks uyjulian for the idea. arcade CDVDMAN has the blue led control export stubbed so directly calling the CMD was the only choice
    sceCdApplySCmd(0x1c, wdata, sizeof(wdata), rdata);
    DPRINTF("sceCdApplySCmd() ret %d\n", x);
    x = 0x3c;
    while (0 < x) {
      DelayThread(1000000);
      x = x + -1;
    }
  } while(1);
}

int _start(int argc, char** argv)
{
    int x;
    iop_thread_t T;
    CpuEnableIntr();
    T.attr = 0x2000000;
    T.thread = bait;
    T.priority = 0x7e;
    T.stacksize = 0x800;
    T.option = 0;
    x = CreateThread(&T);
    if (x > 0) {
        DPRINTF("Starting Thread\n");
        StartThread(x,0);
        return MODULE_RESIDENT_END;
    } else {DPRINTF("CreateThread: %d\n", x);}
    return MODULE_NO_RESIDENT_END;
}
