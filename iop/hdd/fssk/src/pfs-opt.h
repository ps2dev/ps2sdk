#ifndef PFS_OPT
#define PFS_OPT

#define PFS_PRINTF(args, ...) printf(args, ##__VA_ARGS__)
#define PFS_DRV_NAME          "fssk"

// Module version
#define PFS_MAJOR 1
#define PFS_MINOR 4

#ifdef _IOP
#define PFS_ENTRYPOINT _start
#else
#define PFS_ENTRYPOINT fssk_start
#endif

#endif
