#ifndef _APA_OPT_H
#define _APA_OPT_H

#define APA_PRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define APA_DRV_NAME            "hdsk"

// Module version
#define APA_MODVER_MAJOR 1
#define APA_MODVER_MINOR 4

#ifdef _IOP
#define APA_ENTRYPOINT _start
#else
#define APA_ENTRYPOINT hdsk_start
#endif

#endif
