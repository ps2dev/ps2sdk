#include "irx_imports.h"

#define MODNAME "dogbait"
#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME ": " fmt, ##x)
#else
#define DPRINTF(x...)
#endif

IRX_ID(MODNAME, 2, 11);
//thanks uyjulian for the idea
char rdata[16];
char wdata[2] = {0x42, (char)(1 << 8)};

//the loop waiting was made to mirror what rom0:DAEMON did
void bait(void*)
{
  int x;
  printf("fake check card routine start\n");
  do {
#ifdef DEBUG
    x =
#endif
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
