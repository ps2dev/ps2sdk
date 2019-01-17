/*-----------------------------------------------------------------------
- PSX2 OpenSource Project												-
- (C)2004 lion[PS2Dev]													-
- (C)2004 PS2Dev.org													-
-----------------------------------------------------------------------*/

/**
 * @file
 * PS2Camera driver irx
 */

//(1) Please keep this file neat.
//(2) This irx must always be backwords compatible.
#include <stdio.h>
#include <sysclib.h>
#include <tamtypes.h>
#include <thbase.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <loadcore.h>
#include <ioman.h>
#include <usbd.h>
#include <usbd_macro.h>
#include <thsemap.h>
#include <sysmem.h>


#include "ps2cam.h"


#define BIND_RPC_ID				0x00FD000 +2
#define DRIVER_VERSON_MAJOR		1
#define DRIVER_VERSON_MINOR		0

#define MAX_CAM_DEVICE			2
#define MAX_CAM_DEVICE_HANDLE	2


static SifRpcDataQueue_t	rpc_queue			__attribute((aligned(64)));
static SifRpcServerData_t	rpc_server			__attribute((aligned(64)));
static int					_rpc_buffer[1024]	__attribute((aligned(64)));
//static int					threadId;
static int					maintain_thread;
sceUsbdLddOps					cam_driver = {NULL,
											 NULL,
											 "ps2cam",
											   PS2CamProbe,
											    PS2CamConnect,
											     PS2CamDisconnect };



char						campacket[896]		__attribute((aligned(64)));
char						irx_initialized	= 0;
int							ps2cam_sema=0;
CAMERA_DEVICE				Camera[MAX_CAM_DEVICE];
PS2CAM_DEVICE_HANDLE		CamHandle[MAX_CAM_DEVICE_HANDLE];







int _start( int argc, char **argv)
{
	iop_thread_t	param;
	int				th;

	PS2CamInitDriver();


	/*create thread*/
	param.attr       = TH_C;
	param.thread     = rpcMainThread;
	param.priority	 = 40;
	param.stacksize  = 0x800;
	param.option     = 0;


	th = CreateThread(&param);

	if (th > 0)
	{
		StartThread(th,NULL);
		return MODULE_RESIDENT_END;
	}
	else
	{
		return MODULE_NO_RESIDENT_END;
	}
}








/** Register This driver with usbd.irx */
int PS2CamInitDriver(void)
{
	int				i;
	iop_sema_t		sema;

	printf("PS2 USB Camera Driver v.%d.%d  ((C) www.ps2dev.org)\n",DRIVER_VERSON_MAJOR,DRIVER_VERSON_MINOR);


	memset(&Camera[0],    0, sizeof(Camera)*MAX_CAM_DEVICE       );
	memset(&CamHandle[0], 0, sizeof(Camera)*MAX_CAM_DEVICE_HANDLE);

	for(i=0;i<MAX_CAM_DEVICE_HANDLE;i++)
	{
		CamHandle[i].fd = i+1;
	}

	//setup sema


	sema.initial= 1;
	sema.max	= 1;
	sema.option = 0;
	sema.attr	= 0;
	ps2cam_sema = CreateSema(&sema);


	//connect to usb.irx
	return sceUsbdRegisterLdd(&cam_driver);
}




/** check if the device is pluged in */
int PS2CamProbe(int devId)
{
	static short			count;
	int						i;
	UsbDeviceDescriptor		*dev;
	UsbConfigDescriptor		*conf;
	UsbInterfaceDescriptor	*intf;


	dev  = sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);
	conf = sceUsbdScanStaticDescriptor(devId, dev,  USB_DT_CONFIG);
	intf = (UsbInterfaceDescriptor  *) ((char *) conf + conf->bLength);




	// check for a free device slot. if none just ignore new device
	count=0;

	for(i=0;i<MAX_CAM_DEVICE;i++)
	{
		if(Camera[i].status==0)count++;
	}

	if(count <= 0) return 0;


	if(intf->bInterfaceClass == USB_CLASS_VENDOR_SPEC)
	{
		// eyetoy 1
		if(dev->idVendor == PS2CAM_VEND_SONY  &&  dev->idProduct == PS2CAM_PROD_EYETOY)
		{
			return 1;
		}

		// eyetoy 2
		if(dev->idVendor == PS2CAM_VEND_SONY  &&  dev->idProduct == PS2CAM_PROD_EYETOY2)
		{
			return 1;
		}

		// eyetoy 3
		if(dev->idVendor == PS2CAM_VEND_SONY  &&  dev->idProduct == PS2CAM_PROD_EYETOY3)
		{
			return 1;
		}

		// eyetoy 4
		if(dev->idVendor == PS2CAM_VEND_SONY  &&  dev->idProduct == PS2CAM_PROD_EYETOY4)
		{
			return 1;
		}

		// D-Link VisualStream DSB-C310
		if(dev->idVendor == PS2CAM_VEND_DLINK  &&  dev->idProduct == PS2CAM_PROD_VISUALSTREAM)
		{
			return 1;
		}
	}


	return 0;
}






/** this is executed when a compatible camera is detected */
int PS2CamConnect(int devId)
{

	int						i;
	UsbDeviceDescriptor		*dev;

	UsbInterfaceDescriptor	*intf0,*intf1;
	UsbEndpointDescriptor   *endp1;
	CAMERA_DEVICE			*cam = NULL;
	iop_thread_t			param;


	printf("camera was connected\n");



	dev   = sceUsbdScanStaticDescriptor(devId, NULL, USB_DT_DEVICE);

	intf0 = sceUsbdScanStaticDescriptor(devId, dev,  USB_DT_INTERFACE);
	intf1 = sceUsbdScanStaticDescriptor(devId, intf0,  USB_DT_INTERFACE);

	endp1= (UsbEndpointDescriptor  *) ((char *) intf1 + intf1->bLength);




	//get a free device slot
	for(i=0;i<MAX_CAM_DEVICE;i++)
	{
		if(Camera[i].status == 0)
		{
			cam = (CAMERA_DEVICE *)&Camera[i];

			cam->status = CAM_STATUS_CONNECTED;
			break;
		}
	}



	cam->device_id			= devId;

	cam->controll			= sceUsbdOpenPipe(devId, NULL);
	cam->stream				= sceUsbdOpenPipe(devId, endp1);
	cam->stream_pocket_size	= (endp1->wMaxPacketSizeHB * 256 + endp1->wMaxPacketSizeLB);













	/*create thread that will execute funtions that cant be called*/
	/*in this funtion*/
	param.attr       = TH_C;
	param.thread     = (void *)PS2CamInitializeNewDevice;
	param.priority	 = 40;
	param.stacksize  = 0x800;
	param.option     = 0;

	maintain_thread = CreateThread(&param);


	StartThread(maintain_thread, cam);

	return 0;
}






/** this is executed when a compatible camera is unplugged */
int PS2CamDisconnect(int devId)
{
	int						i;
	CAMERA_DEVICE			*cam = NULL;

	printf("camera was unplugged\n");

	//find device in slots and remove/clear/free it
	for(i=0;i<MAX_CAM_DEVICE;i++)
	{
		if(Camera[i].device_id == devId)
		{
			cam = (CAMERA_DEVICE *)&Camera[i];

			cam->status = CAM_STATUS_NOTCONNECTED;
			break;
		}
	}



	// inform any handle that is using this device

	//find device handle and set the status
	for(i=0;i<MAX_CAM_DEVICE_HANDLE;i++)
	{
		if(CamHandle[i].cam == cam)
		{
			CamHandle[i].status = -1;
		}
	}



	// close endpoints
	sceUsbdClosePipe(cam->controll);
	sceUsbdClosePipe(cam->stream  );

	cam->status	= CAM_STATUS_NOTCONNECTED;

	return 0;
}




















/* SIF RPC SERVER FUNCTIONS */


void rpcMainThread(void* param)
{
	//int ret=-1;
	int tid;

	sceSifInitRpc(0);
	tid = GetThreadId();
	sceSifSetRpcQueue(&rpc_queue, tid);
	sceSifRegisterRpc(&rpc_server, BIND_RPC_ID, (void *) rpcCommandHandler, (u8 *) &_rpc_buffer, 0, 0, &rpc_queue);
	sceSifRpcLoop(&rpc_queue);
}

/* CAMERA HIGH LEVEL FUNCTIONS */




/** called after a camera is accepted */
void PS2CamInitializeNewDevice(CAMERA_DEVICE *cam)
{
	unsigned char			*temp_str;
	UsbDeviceDescriptor		*d;


	PS2CamSetDeviceConfiguration(cam,1);

	camStopStream(cam);
	PS2CamSelectInterface(cam,0,0);
	camStartStream(cam);

	PS2CamSetDeviceDefaults(cam);

/*	camStopStream(dev);
	setReg16(dev, 0x30, 384);
	camStartStream(dev);
*/
	camStopStream(cam);
	PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_384);
	camStartStream(cam);

	cam->status = CAM_STATUS_CONNECTEDREADY;



	// connected message (alloc som mem and get device string then print it and free the mem we alloced)
	d   = sceUsbdScanStaticDescriptor(cam->device_id, NULL, USB_DT_DEVICE);
	temp_str = AllocSysMemory(0, 128, 0);
	temp_str[0]=0x00;
	PS2CamGetDeviceSring(cam, d->iProduct,      (char *)temp_str, 128);
	printf("cam initialized(%s)\n",temp_str);
	FreeSysMemory(temp_str);



	DeleteThread(maintain_thread);

	return;

}





/** select the configuration for the device */
void PS2CamSetDeviceConfiguration(CAMERA_DEVICE *dev,int id)
{
	int				ret;


	WaitSema(ps2cam_sema);

	ret = sceUsbdSetConfiguration(dev->controll, id, PS2CamCallback, (void*)ps2cam_sema);

	if(ret != USB_RC_OK)
	{
		printf("Usb: Error sending set_configuration\n");
	}
	else
	{
		WaitSema(ps2cam_sema);
	}

	SignalSema(ps2cam_sema);
}



/** set the defaults for the camera */
void PS2CamSetDeviceDefaults(CAMERA_DEVICE *dev)
{
	//int i;
	static int width,height;


	camResetDevice(dev);

	//stop cam before we change settings then start it back later
	camStopStream(dev);

	camEnableSystem(dev);

	/*set red led pin to out before turning it on*/
	setReg8(dev,EYETOY_GPIO_IO_CTRL0, 0xee);

	camSetUsbInit(dev);


	//turn red led off by default
	camTurnOffRedLed(dev);








	/* some info sent to cam while peeking at
	   the usb log under windows
	*/
	setReg8(dev, EYETOY_CREG_PWDN,		0x03);
	setReg8(dev, EYETOY_CREG_EN_CLK0,	0x9f);
	setReg8(dev, EYETOY_CREG_EN_CLK1,	0x0f);
	setReg8(dev, 0xa2,					0x20);
	setReg8(dev, 0xa3,					0x18);
	setReg8(dev, 0xa4,					0x04);
	setReg8(dev, 0xa5,					0x28);

	setReg8(dev, 0x37,					0x00);
	setReg8(dev, EYETOY_CREG_AUDIO_CLK,	0x02);
	setReg8(dev, EYETOY_IREG_FRAR,		0x1f);
	setReg8(dev, 0x17,					0x50);
	setReg8(dev, 0x37,					0x00);

	setReg8(dev, 0x40,					0xff);
	setReg8(dev, 0x46,					0x00);
	setReg8(dev, EYETOY_CREG_CAMERA_CLK,0x04);
//	setReg8(dev, 0xff,	0x00 );







	// my Y Enhancement
	setReg8(dev, 0xa8,	0x00 );
	setReg8(dev, 0xa9,	0x20 );
	setReg8(dev, 0xaa,	0x00 );






	/* Select 8-bit input mode */
	setReg8Mask(dev, EYETOY_IREG_DFR, 0x10, 0x10);

	// 320x240
	width = 320;
	height= 240;

	//set rez
	setReg8(dev, EYETOY_IREG_H_SIZE,		width>>4);
	setReg8(dev, EYETOY_IREG_V_SIZE,		height>>3);
	//set offset
	setReg8(dev, EYETOY_IREG_X_OFFSETL,		0x00);
	setReg8(dev, EYETOY_IREG_X_OFFSETH,		0x00);
	setReg8(dev, EYETOY_IREG_Y_OFFSETL,		0x00);
	setReg8(dev, EYETOY_IREG_Y_OFFSETH,		0x00);
	//div & color
	//setReg8(dev, EYETOY_IREG_DIVIDER,		0x11);
	camSetDIVIDER(dev, 1, 0, 1, 0);
	setReg8(dev, EYETOY_IREG_FORMAT,		0x03);
	setReg8(dev, 0x26,						0x00);


	// 30 fps
	setReg8(dev, 0xa4, 0x0c);
	setReg8(dev, 0x23, 0xff);



	// start cam after changing settings
	camStartStream(dev);

}














/* CAMERA LOW LEVEL FUNCTIONS */


/** call back for most lowlevel usb funtions */
void PS2CamCallback(int resultCode, int bytes, void *arg)
{
	if(resultCode !=0)
		printf("callback: result= %d, bytes= %d, arg= %p \n", resultCode, bytes, arg);

	SignalSema(ps2cam_sema);
}


/** Set the value in a  8bit register */
int setReg8(CAMERA_DEVICE *dev, unsigned char reg_id, unsigned char value)
{
	int						ret;

	WaitSema(ps2cam_sema);

	ret = sceUsbdControlTransfer(dev->controll, USB_TYPE_VENDOR | USB_RECIP_DEVICE, 1, 0, (unsigned short)reg_id, 1, &value, PS2CamCallback, (void*)ps2cam_sema);

	if(ret != USB_RC_OK)
	{
		printf("sceUsbdControlTransfer failed in 'setReg8'.\n");
		ret = -1;
	}
	else
	{
		WaitSema(ps2cam_sema);
	}

	SignalSema(ps2cam_sema);
	return ret;
}




/** Read the camera's 8bit register  and return the value */
int getReg8(CAMERA_DEVICE *dev, unsigned char reg_id, unsigned char *value)
{
	int					ret;

	WaitSema(ps2cam_sema);

	ret = sceUsbdControlTransfer(dev->controll,USB_DIR_IN|USB_TYPE_VENDOR | USB_RECIP_DEVICE, 1, 0,(unsigned short)reg_id, 1, value, PS2CamCallback, (void*)ps2cam_sema);

	if(ret != USB_RC_OK)
	{
		printf("sceUsbdControlTransfer failed in 'getRegValue'.\n");
		ret = -1;
	}
	else
	{
		WaitSema(ps2cam_sema);
	}


	SignalSema(ps2cam_sema);
	return ret;
}




/** Set the value in a 8bit register using a mask */
int setReg8Mask(CAMERA_DEVICE *dev,unsigned char reg_id, unsigned char value, unsigned char mask)
{
	int				ret;
	unsigned char	current, final;

	ret = getReg8(dev, reg_id, &current);

	if (ret < 0) return -1;

	current	&= (~mask);
	value	&= mask;
	final	 = current | value;

	ret = setReg8(dev, reg_id, final);
	return ret;
}

int setReg16(CAMERA_DEVICE *dev, unsigned char reg_id, unsigned short value)
{
	int			ret;

	WaitSema(ps2cam_sema);

	ret = sceUsbdControlTransfer(dev->controll,USB_TYPE_VENDOR | USB_RECIP_DEVICE, 1, 0, (unsigned short)reg_id, 2, &value, PS2CamCallback, (void*)ps2cam_sema);


	if(ret != USB_RC_OK)
	{
		printf("Usb: setReg16 Error (%d)\n",ret);
	}else
	{
		WaitSema(ps2cam_sema);
	}

	SignalSema(ps2cam_sema);
	return 0;
}



/** Get a string descriptor from device */
void PS2CamGetDeviceSring(CAMERA_DEVICE* dev, int index, char *str, int strmax)
{
	int				i;
	static char		buff[50];
	UsbStringDescriptor	*sd;
	int				ret;


	WaitSema(ps2cam_sema);


	ret = sceUsbdControlTransfer(dev->controll, 0x80, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) | index, 0, sizeof(buff), (char *)&buff[0], PS2CamCallback, (void*)ps2cam_sema);

	if(ret != USB_RC_OK)
	{
		printf("Usb: Error getting string (%d)\n",ret);
	}
	else
	{


		WaitSema(ps2cam_sema);

		sd = (UsbStringDescriptor *)&buff[0];
		for(i=0;(i<(sd->bLength-4)/2 && i<strmax-1);i++)
		{
			str[i]		= sd->wData[i] & 0xFF;
			str[i+1]	= 0x00;
		}
	}


	SignalSema(ps2cam_sema);
}



/** Select the interface and alternet settting to use */
int PS2CamSelectInterface(CAMERA_DEVICE* dev, int interface, int altSetting)
{
	int ret;
	//int semh;
	//iop_sema_t s;


	WaitSema(ps2cam_sema);


	ret = sceUsbdSetInterface(dev->controll, interface, altSetting, PS2CamCallback, (void*)ps2cam_sema);

    if(ret != USB_RC_OK)
	{
		printf("Usb: PS2CamSelectInterface  Error...\n");
	}
	else
	{
		WaitSema(ps2cam_sema);
	}

	SignalSema(ps2cam_sema);
	return ret;
}






/** Reset the eyetoy */
void camResetDevice(CAMERA_DEVICE *dev)
{
	setReg8(dev,EYETOY_CREG_RESET1, 0x0f);
	setReg8(dev,EYETOY_CREG_RESET1, 0x00);
}


void camEnableAutoLaunch(CAMERA_DEVICE *dev)
{
	setReg8(dev, EYETOY_CREG_SNAPSHOT, 23);
}


void camDisableAutoLaunch(CAMERA_DEVICE *dev)
{
	setReg8(dev, EYETOY_CREG_SNAPSHOT, 23);
}


void camClearSnapButton(CAMERA_DEVICE *dev)
{
	setReg8Mask(dev, EYETOY_CREG_SNAPSHOT, 03,02);
}


int  camCheckAutoLaunch(CAMERA_DEVICE *dev)
{
	//setReg8Mask(dev, EYETOY_CREG_SNAPSHOT, 10,10);
	return 0;
}


/** Enable some setting that make the cam capture */
void  camEnableSystem(CAMERA_DEVICE *dev)
{
	setReg8(dev,0x5a, 0x6d);
	setReg8(dev,0x53, 0x9b);
	setReg8(dev,0x54, 0x0f);
	setReg8(dev,0x5d, 0x03);
	setReg8(dev,0x49, 0x01);
	setReg8(dev,0x48, 0x00);
}


/** Disable some stuff */
void camDisableSystem(CAMERA_DEVICE *dev)
{
	setReg8Mask(dev, EYETOY_CREG_EN_CLK0, 0x9b,0x9b);
}


/** This is just a reguler reset to jpeg stuff */
void camResetUsb(CAMERA_DEVICE *dev)
{
	setReg8(dev,0x51, 0x0f);
	setReg8(dev,0x51, 0x00);
}


/** reset jpeg and clear the frame reg */
void camSetUsbInit(CAMERA_DEVICE *dev)
{
	setReg8(dev,0x51, 0x0f);
	setReg8(dev,0x51, 0x00);
	setReg8(dev,0x22, 0x00);
}


/** reset jpeg and set the frame default value */
void camSetUsbWork(CAMERA_DEVICE *dev)
{
	setReg8(dev,0x51, 0x0f);
	setReg8(dev,0x51, 0x00);
	setReg8(dev,0x22, 0x1d);
}


/** turn on the red leg on the GPIO pin */
void camTurnOnRedLed(CAMERA_DEVICE *dev)
{
	setReg8Mask(dev, EYETOY_GPIO_DATA_OUT0, 0x01,0x01);
}


/** turn off the red leg on the GPIO pin */
void camTurnOffRedLed(CAMERA_DEVICE *dev)
{
	setReg8Mask(dev, EYETOY_GPIO_DATA_OUT0, 0x00,0x01);
}


/** restart stream */
void camStartStream(CAMERA_DEVICE *dev)
{

	setReg8(dev, EYETOY_CREG_RESET1, 0x00);
}


void camStopStream(CAMERA_DEVICE *dev)
{

	setReg8(dev, EYETOY_CREG_RESET1, 0x0f);
}






















int read_rslt;
int read_byts;

void PS2CamReadDataCallback(int resultCode, int bytes, void *arg)
{
	//printf("read_data_callback: result= %d, bytes= %d, arg= %p \n", resultCode, bytes, arg);

	read_rslt = resultCode;
	read_byts = bytes;
	SignalSema(ps2cam_sema);
}




int PS2CamReadData(CAMERA_DEVICE* dev, void *addr, int size)
{
	int ret;

	WaitSema(ps2cam_sema);

	ret = sceUsbdIsochronousTransfer(dev->stream, addr, size, 0, PS2CamReadDataCallback, (void*)ps2cam_sema);

	if(ret != USB_RC_OK)
	{
		printf("Usb: Error sending sceUsbdIsochronousTransfer\n");
		return -1;
	}
	else
	{
		WaitSema(ps2cam_sema);
	}

	SignalSema(ps2cam_sema);
	return read_byts;
}













/* CAMERA MISC FUNTIONS */












/* CAMERA RPC/EXPORTED FUNTIONS */


/** Return the current version of the 'ps2cam.irx' */
int PS2CamGetIRXVersion(void)
{
	static unsigned short	ver[2];
	int						*ret;

	ret		= (int *)&ver[2];
	ver[0]	=DRIVER_VERSON_MAJOR;
	ver[1]	=DRIVER_VERSON_MINOR;

	return *ret;
}



/** initalize the camera driver. must be called 1st */
int PS2CamInit(int mode)
{
	irx_initialized = 1;

	return 0;
}



/** get the number of compatible camera connected */
int PS2CamGetDeviceCount(void)
{
	int				i;
	static int		count;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;


	//scan device slots for camara(s)
	for(i=0;i<MAX_CAM_DEVICE;i++)
	{
		if(Camera[i].status > 0)
		{
			count++;
		}
	}

	return count;
}



/** open one of the compatible camera for reading */
int PS2CamOpenDevice(int device_index)
{
	int						i;
	CAMERA_DEVICE			*cam = NULL;
	PS2CAM_DEVICE_HANDLE	*handle = NULL;
	int						index;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	//search for a connected camera
	for(i=0,index=0;i<MAX_CAM_DEVICE;i++)
	{
		if(Camera[i].status > 0)
		{
			if(index == device_index)
			{
				cam = &Camera[i];
				break;
			}

			index++;
		}


		//see the error code to return
		if((i+1) == (MAX_CAM_DEVICE))
		{
			if(index		==    0)return CAM_ERROR_NODEVICE;
			if(index < device_index)return CAM_ERROR_BADRANGE;
			return CAM_ERROR_UNKNOWN;
		}
	}



	//find a free device handle slot for the device
	for(i=0;i<MAX_CAM_DEVICE_HANDLE;i++)
	{
		if(CamHandle[i].status == 0)
		{
			handle = &CamHandle[i];
			break;
		}


		//see error to return
		if((i+1)== MAX_CAM_DEVICE_HANDLE)
		{
			return CAM_ERROR_MAXHANDLE;
		}
	}



	//now we point handle attribute to the camera
	handle->status		= 1;
	handle->cam			= cam;

	return handle->fd;
}



/** close the device if it is no longer needed or disconnected */
int PS2CamCloseDevice(int handle)
{
	int				i;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	//find device handle and clear the status
	for(i=0;i<MAX_CAM_DEVICE_HANDLE;i++)
	{
		if(CamHandle[i].fd == handle)
		{
			CamHandle[i].status=0;
			break;
		}


		//see error to return
		if((i+1)== MAX_CAM_DEVICE_HANDLE)
		{
			return CAM_ERROR_BADHANDLE;
		}
	}

	return 0;
}



/** get the status of the compatible camera */
int PS2CamGetDeviceStatus(int handle)
{
	CAMERA_DEVICE	*dev;



	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;

	dev = CamHandle[handle-1].cam;


	return dev->status;
}



/** get information	about the compatible device */
int PS2CamGetDeviceInfo(int handle,int *info)
{
	static unsigned int		size;
	UsbDeviceDescriptor		*dev;
	PS2CAM_DEVICE_INFO		inf;
	PS2CAM_DEVICE_INFO		*ptr;
	CAMERA_DEVICE			*cam;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;

	ptr = (PS2CAM_DEVICE_INFO *)info;
	size = ptr->ssize;



	if(size > sizeof(PS2CAM_DEVICE_INFO))
	{
		size = sizeof(PS2CAM_DEVICE_INFO);
	}



	cam = CamHandle[handle-1].cam;

	//get descripters
	dev  = sceUsbdScanStaticDescriptor(cam->device_id, NULL, USB_DT_DEVICE);


	// now collect inf
	inf.ssize		= size;
	inf.vendor_id	= dev->idVendor;
	inf.product_id	= dev->idProduct;

	PS2CamGetDeviceSring(cam, dev->iManufacturer, (char *)&inf.vendor_name[0] , 32);
	PS2CamGetDeviceSring(cam, dev->iProduct,      (char *)&inf.product_name[0], 32);


	if(dev->idProduct == PS2CAM_PROD_EYETOY)
	{
		strcpy((char *)&inf.model[0],"SLEH-00030\0");
	}
	else if(dev->idProduct == PS2CAM_PROD_EYETOY2)
	{
		strcpy((char *)&inf.model[0],"SLEH-00031\0");
	}
	else
	{
		strcpy((char *)&inf.model[0],"UNKNOWN\0");
	}
	/**/
	memcpy(info, &inf, size);

	return 0;
}



/** before reading you must set the camera's bandwidth */
int PS2CamSetDeviceBandwidth(int handle, char bandwidth)
{
	int						i;
	CAMERA_DEVICE			*cam;

	UsbDeviceDescriptor		*dev;
	UsbInterfaceDescriptor	*intf;
	UsbEndpointDescriptor	*endp;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;


	cam = CamHandle[handle-1].cam;

	cam->bandwidth = bandwidth;



	//search for enpoint to open
	dev  = sceUsbdScanStaticDescriptor(cam->device_id, NULL, USB_DT_DEVICE);
	intf = sceUsbdScanStaticDescriptor(cam->device_id, dev,  USB_DT_INTERFACE);

	for(i=0;i<cam->bandwidth;i++)
	{
		intf = sceUsbdScanStaticDescriptor(cam->device_id, intf,  USB_DT_INTERFACE);
	}

	endp= (UsbEndpointDescriptor  *) ((char *) intf + intf->bLength);


	// close old endpoint and open new one
					sceUsbdClosePipe(cam->stream);
	cam->stream =	sceUsbdOpenPipe(cam->device_id, endp);

	cam->stream_pocket_size	= (endp->wMaxPacketSizeHB * 256 + endp->wMaxPacketSizeLB);
	printf("bandwidth =%d\n",cam->stream_pocket_size);



	camStopStream(cam);

	switch(bandwidth)
	{
	case 0:
		PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_0);
	break;
	case 1:
		PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_384);
	break;
	case 2:
		PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_512);
	break;
	case 3:
		PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_768);
	break;
	case 4:
		PS2CamSelectInterface(cam,0,EYETOY_ALTERNATE_SIZE_896);
	break;

	}

	setReg8(cam, EYETOY_CREG_RESET1, 0x00);

	return 0;
}



/** read some data from the camera based on bandwidth */
int PS2CamReadPacket(int handle)
{

	CAMERA_DEVICE	*cam;
	int				ret;



	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;


	cam = CamHandle[handle-1].cam;

	if(cam			<= 0) return CAM_ERROR_INVALIDDEVICE;
	if(cam->status	<  2) return CAM_ERROR_DEVNOTREADY;




	switch(cam->bandwidth)
	{
	case EYETOY_ALTERNATE_SIZE_0:
		ret =0;
	break;
	case EYETOY_ALTERNATE_SIZE_384:
		ret = PS2CamReadData(cam, &campacket[0], 384);
	break;
	case EYETOY_ALTERNATE_SIZE_512:
		ret = PS2CamReadData(cam, &campacket[0], 512);
	break;
	case EYETOY_ALTERNATE_SIZE_768:
		ret = PS2CamReadData(cam, &campacket[0], 768);
	break;
	case EYETOY_ALTERNATE_SIZE_896:
		ret = PS2CamReadData(cam, &campacket[0], 896);
	break;
	default :
		ret = PS2CamReadData(cam, &campacket[0], 384);
	break;
	}

	return ret;
}



/** set the mode for the red led */
int PS2CamSetLEDMode(int handle, int mode)
{
	CAMERA_DEVICE	*cam;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;


	cam = CamHandle[handle-1].cam;

	if(cam			<= 0) return CAM_ERROR_INVALIDDEVICE;
	if(cam->status	<  2) return CAM_ERROR_DEVNOTREADY;



	switch(mode)
	{
		case 0://off
			camTurnOffRedLed(cam);
		break;
		case 1://on
			camTurnOnRedLed(cam);
		break;
//		case 2://flash
//
//		break;
		default:
			camTurnOffRedLed(cam);
		break;
	}


	return 0;
}




/** activate setting that is in config struct */
int PS2CamSetDeviceConfig(int handle, void *config)
{
	CAMERA_DEVICE			*cam;
	PS2CAM_DEVICE_CONFIG	*cfg;

	unsigned char			*p;

	if(! irx_initialized)
		return CAM_ERROR_NOTINIT;

	if(handle > MAX_CAM_DEVICE_HANDLE || handle <= 0 || CamHandle[handle-1].status == 0)
		return CAM_ERROR_BADHANDLE;

	if(CamHandle[handle-1].status == -1)
		return CAM_ERROR_DISCONNECTED;


	cam = CamHandle[handle-1].cam;

	if(cam			<= 0) return CAM_ERROR_INVALIDDEVICE;
	if(cam->status	<  2) return CAM_ERROR_DEVNOTREADY;



	cfg = (PS2CAM_DEVICE_CONFIG *)config;

	// stop data
	camStopStream(cam);


	//set rez
	if(cfg->mask & CAM_CONFIG_MASK_DIMENSION)
	{

		setReg8(cam, EYETOY_IREG_H_SIZE,		cfg->width >>4);
		setReg8(cam, EYETOY_IREG_V_SIZE,		cfg->height >>3);
	}

	//set offset
	if(cfg->mask & CAM_CONFIG_MASK_OFFSET)
	{

		p = (unsigned char *)&cfg->x_offset;
		setReg8(cam, EYETOY_IREG_X_OFFSETL,		p[0]);
		setReg8(cam, EYETOY_IREG_X_OFFSETH,		p[1]);

		p = (unsigned char *)&cfg->y_offset;
		setReg8(cam, EYETOY_IREG_Y_OFFSETL,		p[0]);
		setReg8(cam, EYETOY_IREG_Y_OFFSETH,		p[1]);
	}

	//set divider
	if(cfg->mask & CAM_CONFIG_MASK_DIVIDER)
	{
		if(cfg->h_divider > 4)cfg->h_divider = 4;
		if(cfg->v_divider > 4)cfg->v_divider = 4;

		camSetDIVIDER(cam, cfg->h_divider, 0, cfg->v_divider, 0);
	}

	//set framerate
	if(cfg->mask & CAM_CONFIG_MASK_FRAMERATE)
	{

		switch(cfg->framerate)
		{
		case 5:
			setReg8(cam, 0xa4, 0x04);
			setReg8(cam, 0x23, 0x1b);
		break;
		case 10:
			setReg8(cam, 0xa4, 0x04);
			setReg8(cam, 0x23, 0x1f);
		break;
		case 15:
			setReg8(cam, 0xa4, 0x04);
			setReg8(cam, 0x23, 0xff);
		break;
		case 25:
			setReg8(cam, 0xa4, 0x0c);
			setReg8(cam, 0x23, 0x1f);
		break;
		case 30:
			setReg8(cam, 0xa4, 0x0c);
			setReg8(cam, 0x23, 0xff);
		break;
		default:
			setReg8(cam, 0xa4, 0x0c);
			setReg8(cam, 0x23, 0xff);
		break;
		}
	}


	// restart data
	camStartStream(cam);

	return 0;
}


void *rpcCommandHandler(u32 command, void *buffer, int size)
{
	int*	buf = (int*) buffer;
	//char*	ptr = (char*) buffer;
	int		ret = 0;

	switch (command)
	{
		case PS2CAM_RPC_GETIRXVERSION:
			ret = PS2CamGetIRXVersion();

		break;
		case PS2CAM_RPC_INITIALIZE:
			ret = PS2CamInit(buf[0]);

		break;
		case PS2CAM_RPC_GETDEVCOUNT:
			ret = PS2CamGetDeviceCount();

		break;
		case PS2CAM_RPC_OPENDEVICE:
			ret = PS2CamOpenDevice(buf[0]);

		break;
		case PS2CAM_RPC_CLOSEDEVICE:
			ret = PS2CamCloseDevice(buf[0]);

		break;
		case PS2CAM_RPC_GETDEVSTATUS:
			ret = PS2CamGetDeviceStatus(buf[0]);

		break;
		case PS2CAM_RPC_GETDEVINFO:
			ret = PS2CamGetDeviceInfo(buf[0],&buf[1]);

		break;
		case PS2CAM_RPC_SETDEVBANDWIDTH:
			ret = PS2CamSetDeviceBandwidth(buf[0], buf[1]);

		break;
		case PS2CAM_RPC_READPACKET:
			ret = PS2CamReadPacket(buf[0]);

			//get mem address so i can access it from ee
			buf[1] = (int )&campacket[0];

		break;

		case PS2CAM_RPC_SETLEDMODE:
			ret = PS2CamSetLEDMode(buf[0], buf[1]);

		break;
		case PS2CAM_RPC_SETDEVCONFIG:
			ret = PS2CamSetDeviceConfig(buf[0], &buf[1]);

		break;
		default:
			ret = CAM_ERROR_COMMUNKNOWN;

		break;
	}


	buf[0] = ret;


	return buffer;
}






/*EOF*/
