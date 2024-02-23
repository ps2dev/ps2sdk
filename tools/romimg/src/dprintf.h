#define REDBOLD "\033[1;31m"
#define YELBOLD "\033[1;33m"
#define GRNBOLD "\033[1;32m"

#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define DEFCOL "\033[0m"
#include <stdio.h>
#define ERROR(fmt, x...) printf(REDBOLD "ERROR: " fmt DEFCOL, ##x)
#define WARNING(fmt, x...) printf(YELBOLD "WARNING: " fmt DEFCOL, ##x)
// #define DEBUG
#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(REDBOLD "%s: " fmt DEFCOL, __func__, ##x)
#define PRINTLN() DPRINTF("%d\n", __LINE__)
#else
#define DPRINTF(x...)
#define PRINTLN()
#endif