/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# USB driver for PS2
*/

#include "types.h"
#include "defs.h"
#include "irx.h"
#include "loadcore.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"
#include "intrman.h"
#include "stdio.h"
#include "sysclib.h"
#include "usbd.h"
#include "usbd_mod.h"
#include "usbhw.h"

IRX_ID(MODNAME, 1, 1);

struct irx_export_table _exp_usbd;

void hcd_intr_thread(void *arg);
void hcd_callback_thread(void *arg);
int hcd_inthandler(void *arg);
void hcd_dealloc_data();
void usbd_dealloc_data();
int usbd_init();
void hcd_dump_ohciregs();

int g_intr_thid = 0;
int g_callback_thid = 0;
int g_intr_event = 0;
int g_callback_event = 0;
/* This sema is locked when doing any work on the usb structures */
int g_lock_sema = 0;

int g_intr_thpri = 0x1E;
int g_callback_thpri = 0x24;

u32 g_num_devices = 32;
u32 g_num_ep = 64;
u32 g_num_gtd = 128;
u32 g_num_itd = 128;
u32 g_num_ioreq = 256;
u32 g_conf_size = 512;
u32 g_num_hubs = 8;
u32 g_num_ports = 8;
u32 g_reportd = 0;

typedef struct _cmdline_opt

{
   const char *opt;
   u32 *data;
} cmdline_opt;

#define OPT_SIZE 8

cmdline_opt opts[OPT_SIZE] = {
   { "dev", &g_num_devices },
   { "ep", &g_num_ep },
   { "gtd", &g_num_gtd },
   { "itd", &g_num_ioreq },
   { "conf", &g_conf_size },
   { "hub", &g_num_hubs },
   { "port", &g_num_ports },
   { "reportd", &g_reportd }
};

int process_args(int argc, char **argv)

{
   int arg_loop;
   int opt_loop;

   if(argc > 1)
   {
      for(arg_loop = 1; arg_loop < argc; arg_loop++)
      {
	 for(opt_loop = 0; opt_loop < OPT_SIZE; opt_loop++)
	 {
	    int opt_size;
	    int opt_val;

	    opt_size = strlen(opts[opt_loop].opt);

	    if((strncmp(argv[arg_loop], opts[opt_loop].opt, opt_size) == 0) && 
		  (argv[arg_loop][opt_size] == '='))
	    {
	        char *endp;
		opt_val = strtol(&argv[arg_loop][opt_size + 1], &endp, 10);
		M_PRINTF("Setting option %s to %d\n", opts[opt_loop].opt, opt_val);
		*opts[opt_loop].data = opt_val;
		break;
	    }
	 }

	 if(opt_loop == OPT_SIZE)
	 {
	    M_PRINTF("Unknown option %s\n", argv[arg_loop]);
	 }
      }
   }

   return 0;
}

int module_setup(void)

{
   int oldstat;
   int res;
   iop_event_t event;
   iop_thread_t thread;

   if ((res = RegisterLibraryEntries(&_exp_usbd)) != 0) {
      M_PRINTF("Library is already registered, exiting. res=%d\n", res);
      return -1;
   }

   memset(&event, 0, sizeof(iop_event_t));
   g_intr_event = CreateEventFlag(&event);
   if(g_intr_event <= 0)
   {
      return -1;
   }
   dbgprintf("Created Interrupt Event %d\n", g_intr_event);

   memset(&event, 0, sizeof(iop_event_t));
   g_callback_event = CreateEventFlag(&event);
   if(g_callback_event <= 0)
   {
      return -1;
   }
   dbgprintf("Created Callback Event %d\n", g_callback_event);

   thread.attr = TH_C;
   thread.option = 0;
   thread.stacksize = 0x4000;
   thread.thread = hcd_intr_thread;
   thread.priority = g_intr_thpri;
   g_intr_thid = CreateThread(&thread);
   if(g_intr_thid <= 0)
   {
      return -1;
   }
   dbgprintf("Created Interrupt Thread %d\n", g_intr_thid);
   
   thread.attr = TH_C;
   thread.option = 0;
   thread.stacksize = 0x4000;
   thread.thread = hcd_callback_thread;
   thread.priority = g_callback_thpri;
   g_callback_thid = CreateThread(&thread);
   if(g_callback_thid <= 0)
   {
      return -1;
   }
   dbgprintf("Created Callback Thread %d\n", g_callback_thid);

   g_lock_sema = CreateMutex(IOP_MUTEX_UNLOCKED);
   if(g_lock_sema < 0)
   {
      return -1;
   }

   if(StartThread(g_intr_thid, 0) < 0)
   {
      return -1;
   }

   if(StartThread(g_callback_thid, 0) < 0)
   {
      return -1;
   }

   DisableIntr(USB_INTR, &oldstat);
   RegisterIntrHandler(USB_INTR, 1, hcd_inthandler, (void *) g_intr_event);
   EnableIntr(USB_INTR); 

   return 0;
}

void module_cleanup(void)

{
   int oldstat; 

   dbgprintf("Module cleanup\n");
   DisableIntr(USB_INTR, &oldstat);
   ReleaseIntrHandler(USB_INTR);
   ReleaseLibraryEntries(&_exp_usbd);
   hcd_dealloc_data();
   usbd_dealloc_data();
   if(g_intr_thid > 0)
   {
      DeleteThread(g_intr_thid);
      g_intr_thid = 0;
   }

   if(g_intr_event > 0)
   {
      DeleteEventFlag(g_intr_event);
      g_intr_event = 0;
   }

   if(g_callback_thid > 0)
   {
      DeleteThread(g_callback_thid);
      g_callback_thid = 0;
   }

   if(g_callback_event > 0)
   {
      DeleteEventFlag(g_callback_event);
      g_callback_event = 0;
   }
}

int _start(int argc, char **argv)
{
   printf(BANNER, VERSION);
   printf("Currently WIP. Do not USE!!!!!!!!!!!!!!!!\n");

   process_args(argc, argv);

   if(module_setup() < 0)
   {
      module_cleanup();
      M_PRINTF("Failed to initialise\n");
      return -1;
   }
   
   if(usbd_init() < 0)
   { 
      module_cleanup();
      M_PRINTF("Error in initialisation\n");
      return -1;
   }

   M_PRINTF("Driver loaded.\n");

   return 0;
}
