#ifndef __USBD_MOD_H__
#define __USBD_MOD_H__

#define DEBUG

#ifdef DEBUG
#define dbgprintf(args...) printf(args)
#else
#define dbgprintf(args...) do { } while(0)
#endif

#define MODNAME "usbd"
#define M_PRINTF(format, args...)       printf(MODNAME ": " format, ## args)
#define BANNER "USBD %s\n"
#define VERSION "v0.1"

/* Define this to enable DevFS support in the module */
#define ENABLE_DEVFS

/* Some helper defines */
#define LOCK(sema, ret) if(WaitSema((sema)) < 0) { M_PRINTF("Error, waiting for sema\n"); return (ret); }
#define UNLOCK(sema)    SignalSema((sema));

/* Callback event */
#define EVENT_CALLBACK 1
/* RHSC event */
#define EVENT_RHSC     2
/* Pending RH control transfer */
#define EVENT_RH_CTL   4
/* Pending RH Interrupt tansfer */
#define EVENT_RH_INT   8

#endif
