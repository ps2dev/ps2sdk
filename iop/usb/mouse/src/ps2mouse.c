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
# USB Mouse Driver for PS2
*/

#include "types.h"
#include "iomanX.h"
#include "loadcore.h"
#include "stdio.h"
#include "sifcmd.h"
#include "sifrpc.h"
#include "sysclib.h"
#include "sysmem.h"
#include "usbd.h"
#include "usbd_macro.h"
#include "thbase.h"
#include "thevent.h"
#include "thsemap.h"

#include "ps2mouse.h"

#define PS2MOUSE_VERSION 0x100

#define USB_SUBCLASS_BOOT 1
#define USB_HIDPROTO_MOUSE 2

#define PS2MOUSE_MAXDEV 2
#define PS2MOUSE_MAXBUTTONS 3

#define PS2MOUSE_DEFDBLCLICK 500

#define PS2MOUSE_DEFACCEL (1 << 16)
#define PS2MOUSE_DEFTHRES 65536;

static SifRpcDataQueue_t ps2mouse_queue __attribute__((aligned(64)));
static SifRpcServerData_t ps2mouse_server __attribute((aligned(64)));
static int _rpc_buffer[512] __attribute((aligned(64)));

#define ABS(x) (x < 0 ? -x : x)

void rpcMainThread(void* param);
void *rpcCommandHandler(u32 command, void *buffer, int size);

int ps2mouse_init();
void ps2mouse_config_set(int resultCode, int bytes, void *arg);
void ps2mouse_data_recv(int resultCode, int bytes, void *arg);
int ps2mouse_probe(int devId);
int ps2mouse_connect(int devId);
int ps2mouse_disconnect(int devId);
void usb_getstring(int endp, int index, char *desc);

typedef struct _mouse_data_recv

{
  unsigned char buttons;
  char x, y, wheel;
} mouse_data_recv __attribute__ ((packed));

typedef struct _mouse_dev

{
  int configEndp;
  int dataEndp;
  int packetSize;
  int devId;
  mouse_data_recv data; /* Holds the data for the transfers */
  u32 timer[PS2MOUSE_MAXBUTTONS];         /* Array to hold timers for double click */
} mouse_dev;

/* Global Variables */

int mouse_readmode;
int mousex_min;
int mousex_max;
int mousey_min;
int mousey_max;
int mouse_thres;
int mouse_accel;
int mouse_dblclicktime;
int mouse_sema;
mouse_data mouse; /* Holds the current mouse information */
mouse_dev *devices[PS2MOUSE_MAXDEV]; /* Holds a list of current devices */
int dev_count;
UsbDriver mouse_driver = { NULL, NULL, "PS2Mouse", ps2mouse_probe, ps2mouse_connect, ps2mouse_disconnect };

int _start ()
{
  iop_thread_t param;
  int th;

  FlushDcache();

  ps2mouse_init();

  param.attr         = TH_C;
  param.thread     = rpcMainThread;
  param.priority 	 = 40;
  param.stacksize    = 0x800;
  param.option      = 0;

  th = CreateThread(&param);
  if (th > 0) {
  	StartThread(th,0);
	return 0;
  }
  else return 1;
}

void rpcMainThread(void* param)
{
  int tid;

  SifInitRpc(0);

  //printf("PS2MOUSE - RPC Initialise\n");
  printf("PS2MOUSE - USB Mouse Library\n");

  tid = GetThreadId();
  SifSetRpcQueue(&ps2mouse_queue, tid);
  SifRegisterRpc(&ps2mouse_server, PS2MOUSE_BIND_RPC_ID, (void *) rpcCommandHandler, (u8 *) &_rpc_buffer, 0, 0, &ps2mouse_queue);
  SifRpcLoop(&ps2mouse_queue);
}

int ps2mouse_probe(int devId)

{
  UsbDeviceDescriptor *dev;
  UsbConfigDescriptor *conf;
  UsbInterfaceDescriptor *intf;
  UsbEndpointDescriptor *endp;
  //UsbStringDescriptor *str;

  if(dev_count >= PS2MOUSE_MAXDEV)
    {
      printf("ERROR: Maximum mouse devices reached\n");
      return 0;
    }

  //printf("PS2Mouse_probe devId %d\n", devId);

  dev = UsbGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE); /* Get device descriptor */
  if(!dev) 
    {
      printf("ERROR: Couldn't get device descriptor\n");
      return 0;
    }

  //printf("Device class %d, Size %d, Man %d, Product %d\n", dev->bDeviceClass, dev->bMaxPacketSize0, dev->iManufacturer, dev->iProduct);
  /* Check that the device class is specified in the interfaces and it has at least one configuration */
  if((dev->bDeviceClass != USB_CLASS_PER_INTERFACE) || (dev->bNumConfigurations < 1))
    {
      //printf("This is not the droid you're looking for\n");
      return 0;
    }

  conf = UsbGetDeviceStaticDescriptor(devId, dev, USB_DT_CONFIG);
  if(!conf)
    {
      //printf("ERROR: Couldn't get configuration descriptor\n");
      return 0;
    }
  //printf("Config Length %d Total %d Interfaces %d\n", conf->bLength, conf->wTotalLength, conf->bNumInterfaces);

  if((conf->bNumInterfaces < 1) || (conf->wTotalLength < (sizeof(UsbConfigDescriptor) + sizeof(UsbInterfaceDescriptor))))
    {
      printf("ERROR: No interfaces available\n");
      return 0;
    }
     
  intf = (UsbInterfaceDescriptor *) ((char *) conf + conf->bLength); /* Get first interface */
/*   printf("Interface Length %d Endpoints %d Class %d Sub %d Proto %d\n", intf->bLength, */
/* 	 intf->bNumEndpoints, intf->bInterfaceClass, intf->bInterfaceSubClass, */
/* 	 intf->bInterfaceProtocol); */

  if((intf->bInterfaceClass != USB_CLASS_HID) || (intf->bInterfaceSubClass != USB_SUBCLASS_BOOT) ||
     (intf->bInterfaceProtocol != USB_HIDPROTO_MOUSE) || (intf->bNumEndpoints < 1))
    {
      //printf("We came, we saw, we told it to fuck off\n");
      return 0;
    }

  endp = (UsbEndpointDescriptor *) ((char *) intf + intf->bLength);
  endp = (UsbEndpointDescriptor *) ((char *) endp + endp->bLength); /* Go to the data endpoint */

  //printf("Endpoint 1 Addr %d, Attr %d, MaxPacket %d\n", endp->bEndpointAddress, endp->bmAttributes, endp->wMaxPacketSizeLB);
  
  if(((endp->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT) ||
     ((endp->bEndpointAddress & USB_ENDPOINT_DIR_MASK) != USB_DIR_IN))
    {
      printf("ERROR: Endpoint not interrupt type and/or an input\n");
      return 0;
    }

  printf("PS2MOUSE: Found a mouse device\n");

  return 1;
}

int ps2mouse_connect(int devId)

{
  /* Assume we can only get here if we have already checked the device is kosher */

  UsbDeviceDescriptor *dev;
  UsbConfigDescriptor *conf;
  UsbInterfaceDescriptor *intf;
  UsbEndpointDescriptor *endp;
  mouse_dev *currDev;
  int devLoop;

  //printf("PS2Mouse_connect devId %d\n", devId);

  dev = UsbGetDeviceStaticDescriptor(devId, NULL, USB_DT_DEVICE); /* Get device descriptor */
  if(!dev) 
    {
      printf("ERROR: Couldn't get device descriptor\n");
      return 1;
    }

  conf = UsbGetDeviceStaticDescriptor(devId, dev, USB_DT_CONFIG);
  if(!conf)
    {
      printf("ERROR: Couldn't get configuration descriptor\n");
      return 1;
    }
     
  intf = (UsbInterfaceDescriptor *) ((char *) conf + conf->bLength); /* Get first interface */
  endp = (UsbEndpointDescriptor *) ((char *) intf + intf->bLength);
  endp = (UsbEndpointDescriptor *) ((char *) endp + endp->bLength); /* Go to the data endpoint */

  currDev = (mouse_dev *) AllocSysMemory(0, sizeof(mouse_dev), NULL);
  if(!currDev)
    {
      printf("ERROR: Couldn't allocate a device point for the mouse\n");
      return 1;
    }

  currDev->configEndp = UsbOpenEndpoint(devId, NULL);
  currDev->dataEndp = UsbOpenEndpoint(devId, endp);
  currDev->packetSize = endp->wMaxPacketSizeLB | ((int) endp->wMaxPacketSizeHB << 8);
  if(currDev->packetSize > sizeof(mouse_data_recv))
    {
      currDev->packetSize = sizeof(mouse_data_recv);
    }

  currDev->devId = devId;

  if(dev->iManufacturer)
    {
      usb_getstring(currDev->configEndp, dev->iManufacturer, "Mouse Manufacturer");
    }

  if(dev->iProduct)
    {
      usb_getstring(currDev->configEndp, dev->iProduct, "Mouse Product");
    }

  for(devLoop = 0; devLoop < PS2MOUSE_MAXDEV; devLoop++)
    {
      if(devices[devLoop] == NULL)
	{
	  devices[devLoop] = currDev;
	  break;
	}
    }

  if(devLoop == PS2MOUSE_MAXDEV)
    {
      /* How the f*** did we end up here ??? */
      printf("ERROR: Device Weirdness!!\n");
      return 1;
    }

  UsbSetDevicePrivateData(devId, currDev); /* Set the index for the device data */

  //printf("Configuration value %d\n", conf->bConfigurationValue);
  UsbSetDeviceConfiguration(currDev->configEndp, conf->bConfigurationValue, ps2mouse_config_set, currDev);

  dev_count++; /* Increment device count */
  printf("PS2MOUSE: Connected device\n");

  return 0;
}

int ps2mouse_disconnect(int devId)

{
  int devLoop;
  //printf("PS2Mouse_disconnect devId %d\n", devId);

  for(devLoop = 0; devLoop < PS2MOUSE_MAXDEV; devLoop++)
    {
      if((devices[devLoop]) && (devices[devLoop]->devId == devId))
	{
	  dev_count--;
	  FreeSysMemory(devices[devLoop]);
	  devices[devLoop] = NULL;
	  printf("PS2MOUSE: Disconnected device\n");
	  break;
	}
    }

  return 0;
}


typedef struct _string_descriptor

{
  u8 buf[200];
  char *desc;
} string_descriptor; 

void ps2mouse_getstring_set(int resultCode, int bytes, void *arg)

{
  UsbStringDescriptor *str = (UsbStringDescriptor *) arg;
  string_descriptor *strBuf = (string_descriptor *) arg;
  char string[50];
  int strLoop;

/*   printf("=========getstring=========\n"); */

/*   printf("PS2MOUSE: GET_DESCRIPTOR res %d, bytes %d, arg %p\n", resultCode, bytes, arg); */

  if(resultCode == USB_RC_OK)
    {
      memset(string, 0, 50);
      for(strLoop = 0; strLoop < ((bytes - 2) / 2); strLoop++)
	{
	  string[strLoop] = str->wData[strLoop] & 0xFF;
	}
      printf("%s: %s\n", strBuf->desc, string);
    }
  
  FreeSysMemory(arg);
}

void usb_getstring(int endp, int index, char *desc)

{
  u8 *data;
  string_descriptor *str;
  int ret; 

  data = (u8 *) AllocSysMemory(0, sizeof(string_descriptor), NULL);
  str = (string_descriptor *) data;

  if(data != NULL)
    {
      str->desc = desc;
      ret = UsbControlTransfer(endp, 0x80, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) | index, 
			       0, sizeof(string_descriptor) - 4, data, ps2mouse_getstring_set, data);
      if(ret != USB_RC_OK)
	{
	  printf("PS2MOUSE: Error sending string descriptor request\n");
	  FreeSysMemory(data);
	}
    }
}

void ps2mouse_config_set(int resultCode, int bytes, void *arg)
     /* Called when we have finished choosing our configuration */

{
  mouse_dev *dev;

  if(resultCode != USB_RC_OK)
    {
      printf("PS2MOUSE: Configuration set error res %d, bytes %d, arg %p\n", resultCode, bytes, arg);
      return;
    }

  //printf("PS2MOUSE: Configuration set res %d, bytes %d, arg %p\n", resultCode, bytes, arg);
  /* Do a interrupt data transfer */

  dev = (mouse_dev *) arg;
  if(dev != NULL)
    {
      int ret;
      
      ret = UsbInterruptTransfer(dev->dataEndp, &dev->data, dev->packetSize, ps2mouse_data_recv, arg);
    }
}

void ps2mouse_data_recv(int resultCode, int bytes, void *arg)

{
  mouse_dev *dev;
  
  if((resultCode != USB_RC_OK) && (resultCode != USB_RC_DATAOVER))
    {
      printf("PS2MOUSE: Data Recv set res %d, bytes %d, arg %p\n", resultCode, bytes, arg);
      return;
    }
  
  //printf("PS2MOUSE: Data Recv set res %d, bytes %d, arg %p\n", resultCode, bytes, arg);

  dev = (mouse_dev *) arg;
  if(dev != NULL)
    {
      int ret;
      int buttonLoop;
      int buttonData;
      int mx, my;

      WaitSema(mouse_sema);

      mx = dev->data.x;
      my = dev->data.y;

      if(ABS(mx) >= mouse_thres)
	{
	  mx = (mx * mouse_accel) >> 16;
	}

      if(ABS(my) >= mouse_thres)
	{
	  my = (my * mouse_accel) >> 16;
	}

/*       if(mx > mouse_thres) mx = mouse_thres; */
/*       else if(mx < (-mouse_thres)) mx = -mouse_thres; */

/*       if(my > mouse_thres) my = mouse_thres; */
/*       else if(my < (-mouse_thres)) my = -mouse_thres; */

      mouse.x += mx;
      mouse.y += my;
      mouse.wheel += dev->data.wheel;
      buttonData = 0;
      for(buttonLoop = 0; buttonLoop < PS2MOUSE_MAXBUTTONS; buttonLoop++)
	{
	  int currButton = 1 << buttonLoop;

	  if( ((mouse.buttons & currButton) == 0) && (dev->data.buttons & currButton))
	    {
	      iop_sys_clock_t t;
	      int usec, sec;
	      int msec;

	      GetSystemTime(&t);
	      SysClock2USec(&t, (u32 *)&sec, (u32 *)&usec);
	      msec = (sec * 1000) + (usec / 1000);
	      //printf("%d %d %d\n", msec, sec, usec);

	      if(dev->timer[buttonLoop])
		{
		  if((msec - dev->timer[buttonLoop]) < mouse_dblclicktime)
		    {
		      //printf("Double click\n");
		      buttonData |= (1 << (buttonLoop + 8));
		      dev->timer[buttonLoop] = 0;
		    }
		  else
		    {
		      dev->timer[buttonLoop] = msec;
		    }
		}
	      else /* If not set a timer */
		{
		  dev->timer[buttonLoop] = msec;
		}
	    }
	}
      mouse.buttons = dev->data.buttons | buttonData;
      if(mouse_readmode == PS2MOUSE_READMODE_ABS)
	{
	  if(mouse.x < mousex_min) mouse.x = mousex_min;
	  if(mouse.x > mousex_max) mouse.x = mousex_max;
	  if(mouse.y < mousey_min) mouse.y = mousey_min;
	  if(mouse.y > mousey_max) mouse.y = mousey_max;
	}

      SignalSema(mouse_sema);
      //printf("X = %d, Y = %d, Wheel = %d, Buttons = %x\n", mouse.x, mouse.y, mouse.wheel, mouse.buttons);

      ret = UsbInterruptTransfer(dev->dataEndp, &dev->data, dev->packetSize, ps2mouse_data_recv, arg);
    }
}

int ps2mouse_init()

{
  int ret;
  iop_sema_t s;

  s.initial = 1;
  s.max = 1;
  s.option = 0;
  s.attr = 0;
  mouse_sema = CreateSema(&s);
  if(mouse_sema <= 0)
    {
      printf("ERROR: Couldn't create ps2mouse sema\n");
      return 1;
    }

  ret = UsbRegisterDriver(&mouse_driver);
  memset(&mouse, 0, sizeof(mouse_data));
  memset(devices, 0, sizeof(mouse_dev *) * PS2MOUSE_MAXDEV);
  dev_count = 0;
  mouse_readmode = PS2MOUSE_READMODE_DIFF;
  mousex_min = -1000000;
  mousex_max =  1000000;
  mousey_min = -1000000;
  mousey_max =  1000000;
  mouse_dblclicktime = PS2MOUSE_DEFDBLCLICK;
  mouse_accel = PS2MOUSE_DEFACCEL;
  mouse_thres = PS2MOUSE_DEFTHRES;

  //printf("UsbRegisterDriver %d\n", ret);

  return 0;
}

/* RPC Handlers */

void do_ps2mouse_read(u8 *data, int size)

{
  //printf("PS2MOUSE read\n");
  memcpy(data, &mouse, sizeof(mouse_data));

  //printf("%d %d %d %d\n", mouse.x, mouse.y, mouse.buttons, mouse.wheel);
  if(mouse_readmode == PS2MOUSE_READMODE_DIFF)
    {
      mouse.x = 0;
      mouse.y = 0;
      mouse.wheel = 0;
      mouse.buttons &= 0xFF; /* Clear the double click flags */
    }
  else
    {
      mouse.wheel = 0;
      mouse.buttons &= 0xFF;
    }
}

void do_ps2mouse_setreadmode(u32 *data, int size)

{
  //printf("PS2MOUSE setreadmode mode %d\n", data[0]);
  if(data[0] == mouse_readmode)
    {
      return;
    }

  if((data[0] != PS2MOUSE_READMODE_DIFF) && (data[0] != PS2MOUSE_READMODE_ABS))
    {
      printf("ERROR: Invalid readmode\n");
      return;
    }

  memset(&mouse, 0, sizeof(mouse_data));
  mouse_readmode = data[0];
  if(mouse_readmode == PS2MOUSE_READMODE_ABS)
    {
      mouse.x = mousex_min;
      mouse.y = mousey_min;
    }
}

void do_ps2mouse_getreadmode(u32 *data, int size)

{
  //printf("PS2MOUSE getreadmode\n");
  data[0] = mouse_readmode;
}

void do_ps2mouse_setthres(u32 *data, int size)

{
  //printf("PS2MOUSE setthres %d\n", data[0]);
  mouse_thres = data[0];
}

void do_ps2mouse_getthres(u32 *data, int size)

{
  //printf("PS2MOUSE getthres\n");
  data[0] = mouse_thres;
}

void do_ps2mouse_setaccel(u32 *data, int size)

{
  //printf("PS2MOUSE setsense %d\n", data[0]);
  mouse_accel = data[0];
}

void do_ps2mouse_getaccel(u32 *data, int size)

{
  //printf("PS2MOUSE getsense\n");
  data[0] = mouse_accel;
}

void do_ps2mouse_setboundary(s32 *data, int size)

{
  //printf("PS2MOUSE setboundry %d %d %d %d\n", data[0], data[1], data[2], data[3]);
  if(data[0] < data[1])
    {
      mousex_min = data[0];
      mousex_max = data[1];
    }

  if(data[2] < data[3])
    {
      mousey_min = data[2];
      mousey_max = data[3];
    }
  if(mouse_readmode == PS2MOUSE_READMODE_ABS)
    {

      mouse.x = mousex_min;
      mouse.y = mousey_min;
    }
}

void do_ps2mouse_getboundary(s32 *data, int size)

{
  //printf("PS2MOUSE getboundry\n");
  data[0] = mousex_min;
  data[1] = mousex_max;
  data[2] = mousey_min;
  data[3] = mousey_max;
}

void do_ps2mouse_setposition(s32 *data, int size)

{
  //printf("PS2MOUSE setposition %d %d\n", data[0], data[1]);

  WaitSema(mouse_sema);

  if(data[0] < mousex_min)
    {
      mouse.x = mousex_min;
    }
  else if(data[0] > mousex_max)
    {
      mouse.x = mousex_max;
    }
  else
    {
      mouse.x = data[0];
    }

  if(data[1] < mousey_min)
    {
      mouse.y = mousey_min;
    }
  else if(data[1] > mousey_max)
    {
      mouse.y = mousey_max;
    }
  else
    {
      mouse.y = data[1];
    }
  SignalSema(mouse_sema);
}

void do_ps2mouse_reset()

{
  //printf("PS2MOUSE reset\n");
  memset(&mouse, 0, sizeof(mouse_data));
  if(mouse_readmode == PS2MOUSE_READMODE_ABS) /* If ABS mode then set to lowest boundry */
    {
      mouse.x = mousex_min;
      mouse.y = mousey_min;
    }
}

void do_ps2mouse_enum(u32 *data, int size)

{
  //printf("PS2MOUSE enum\n");
  data[0] = dev_count;
}

void do_ps2mouse_setdblclicktime(u32 *data, int size)

{
  //printf("PS2MOUSE setdblclicktime %d\n", data[0]);
  mouse_dblclicktime = data[0];
}

void do_ps2mouse_getdblclicktime(u32 *data, int size)

{
  //printf("PS2MOUSE getdblclicktime\n");
  data[0] = mouse_dblclicktime;
}

void do_ps2mouse_getversion(u32 *data, int size)

{
  //printf("PS2MOUSE getversion\n");
  data[0] = PS2MOUSE_VERSION;
}

void *rpcCommandHandler(u32 command, void *buffer, int size)

{
  switch(command)
    {
    case PS2MOUSE_READ: do_ps2mouse_read((u8 *) buffer, size);
      break;
    case PS2MOUSE_SETREADMODE: do_ps2mouse_setreadmode((u32 *) buffer, size);
      break;
    case PS2MOUSE_GETREADMODE: do_ps2mouse_getreadmode((u32 *) buffer, size);
      break;
    case PS2MOUSE_SETTHRES: do_ps2mouse_setthres((u32 *) buffer, size);
      break;
    case PS2MOUSE_GETTHRES: do_ps2mouse_getthres((u32 *) buffer, size);
      break;
    case PS2MOUSE_SETACCEL: do_ps2mouse_setaccel((u32 *) buffer, size);
      break;
    case PS2MOUSE_GETACCEL: do_ps2mouse_getaccel((u32 *) buffer, size);
      break;
    case PS2MOUSE_SETBOUNDARY: do_ps2mouse_setboundary((s32 *) buffer, size);
      break;
    case PS2MOUSE_GETBOUNDARY: do_ps2mouse_getboundary((s32 *) buffer, size);
      break;
    case PS2MOUSE_SETPOSITION: do_ps2mouse_setposition((s32 *) buffer, size);
      break;
    case PS2MOUSE_RESET: do_ps2mouse_reset();
      break;
    case PS2MOUSE_ENUM: do_ps2mouse_enum((u32 *) buffer, size);
      break;
    case PS2MOUSE_SETDBLCLICKTIME: do_ps2mouse_setdblclicktime((u32 *) buffer, size);
      break;
    case PS2MOUSE_GETDBLCLICKTIME: do_ps2mouse_getdblclicktime((u32 *) buffer, size);
      break;
    case PS2MOUSE_GETVERSION: do_ps2mouse_getversion((u32 *) buffer, size);
      break;
    default : printf("Unknown PS2MOUSE command %ld\n", command);
      break;
    }

  return buffer;
}
