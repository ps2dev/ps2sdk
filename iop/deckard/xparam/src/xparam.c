#include "irx_imports.h"
#include <tamtypes.h>
#include <iop_regs.h>
#include <stdint.h>
#include "xparam.h"

#define MODNAME "xparam"
#define MAJOR 1
#define MINOR 0

IRX_ID(MODNAME, MAJOR, MINOR)

#define XPARAM_PARAM_ADDR *((uint32_t *)0xFFFE01A0)
#define XPARAM_VALUE_ADDR *((uint32_t *)0xFFFE01A4)

void SetDummyXparamValue(void)
{
    XPARAM_PARAM_ADDR = 0xFFFFFFFF;
    XPARAM_VALUE_ADDR = 0;
}
void CheckGameID(const char* ID);
int _start(int argc, char **argv)
{
    int i1;
    int i2;
    char **z;
    char **x;
    if (IOP_CPU_TYPE == IOP_TYPE_POWERPC) { //used to be a function. simplify it
        SetDummyXparamValue();
        if (1 < argc) {
            if (argc < 3) {
                CheckGameID(argv[1]);
            }
            else if (3 < argc) {
                z = argv + 2;
                i1 = 2;
                do {
                    XPARAM_PARAM_ADDR = strtol(*z, 0, 10);
                    x = z + 1;
                    z = z + 2;
                    XPARAM_VALUE_ADDR = strtol(*x, 0, 10);
                    i2 = i1 + 3;
                    i1 = i1 + 2;
                } while (i2 < argc);
            }
        }
    }
    return MODULE_NO_RESIDENT_END; //always NO_RESIDENT_END
}

//used to return int but retval was never used
void CheckGameID(const char *ID)
{
    int f=0; //have we found the entry?
    for (unsigned int i = 0; i < GET_XPARAM_DB_SIZE(); i++)
    {
        if (!strcmp(XPARAMS[i].name, ID)) {
            f = 1;
            XPARAM_PARAM_ADDR = XPARAMS[i].param;
            XPARAM_VALUE_ADDR = XPARAMS[i].value;
        } else if (f) break; //comparison failed but f is true. so there are no more consecutive entries for the same game
    }
}
