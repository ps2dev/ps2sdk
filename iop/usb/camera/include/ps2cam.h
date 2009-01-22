/*
  PSX2 OpenSource Project
  (C)2004 lion[PS2Dev]
  (C)2004 PS2Dev.org
  ------------------------------------------------------------------------
  ps2cam.h
					PS2Camera driver irx
			
*/
#ifndef __PS2CAM_H__
#define __PS2CAM_H__





//rpc call id
#define PS2CAM_RPC_GETIRXVERSION	40		// get irx version
#define PS2CAM_RPC_INITIALIZE		41		// PS2CamInit
#define PS2CAM_RPC_GETDEVCOUNT		42		// PS2CamGetDeviceCount
#define PS2CAM_RPC_OPENDEVICE		43		// 
#define PS2CAM_RPC_CLOSEDEVICE		44		// 
#define PS2CAM_RPC_GETDEVSTATUS		45		// PS2CamGetStatus
#define PS2CAM_RPC_GETDEVINFO		46		// PS2CamGetInfo
#define PS2CAM_RPC_SETDEVBANDWIDTH	47		// PS2CamSetDeviceBandwidth
#define PS2CAM_RPC_READPACKET		48		// PS2CamReadPacketCOMMAND
#define PS2CAM_RPC_SETLEDMODE		49		// PS2CamSetLEDMode
#define PS2CAM_RPC_SETDEVCONFIG		50		//




/*camera vender(s)*/
#define PS2CAM_VEND_SONY			0x054C	// Sony
#define PS2CAM_VEND_DLINK			0x05A9	// D-Link
#define PS2CAM_VEND_LOGITECH		0x0000	// Logitech

/*product id(s)*/
#define PS2CAM_PROD_EYETOY			0x0154	// SLEH-00030
#define PS2CAM_PROD_EYETOY2			0x0155	// SLEH-00031
#define PS2CAM_PROD_EYETOY3			0x0156	// SLEH-000??	rinco thinks its japan
#define PS2CAM_PROD_EYETOY4			0x0157	// SLEH-000??   ,,  ,,  ,,
		
#define PS2CAM_PROD_VISUALSTREAM	0x8519	// D-Link VisualStream DSB-C310



// eyetoy regs
#define EYETOY_IREG_H_SIZE			0x10
#define EYETOY_IREG_V_SIZE			0x11
#define EYETOY_IREG_X_OFFSETL		0x12
#define EYETOY_IREG_X_OFFSETH		0x13
#define EYETOY_IREG_Y_OFFSETL		0x14
#define EYETOY_IREG_Y_OFFSETH		0x15
#define EYETOY_IREG_DIVIDER			0x16
#define EYETOY_IREG_DFR				0x20
#define EYETOY_IREG_SR				0x21
#define EYETOY_IREG_FRAR			0x22
#define EYETOY_IREG_FORMAT			0x25

#define EYETOY_CREG_RESET0			0x50
#define EYETOY_CREG_RESET1			0x51
#define EYETOY_CREG_EN_CLK0			0x53
#define EYETOY_CREG_EN_CLK1			0x54
#define EYETOY_CREG_AUDIO_CLK		0x55
#define EYETOY_CREG_SNAPSHOT		0x57
#define EYETOY_CREG_PONOFF			0x58
#define EYETOY_CREG_CAMERA_CLK		0x59
#define EYETOY_CREG_CTRL1			0x5A
#define EYETOY_CREG_DEB_CLK			0x5B
#define EYETOY_CREG_CLK				0x5C
#define EYETOY_CREG_PWDN			0x5D
#define EYETOY_CREG_USR_DFN			0x5E
#define EYETOY_CREG_CTRL2			0x5F
#define EYETOY_CREG_INTERRUPT0		0x60
#define EYETOY_CREG_INTERRUPT1		0x61
#define EYETOY_CREG_MASK0			0x62
#define EYETOY_CREG_MASK1			0x63
#define EYETOY_CREG_VCI_R0			0x64
#define EYETOY_CREG_VCI_R1			0x65
#define EYETOY_CREG_ADC_CTRL		0x68
#define EYETOY_CREG_UC_CTRL			0x6D

/*these reg control extra pins on chip(eg:red led)*/
#define EYETOY_GPIO_DATA_OUT0		0x71
#define EYETOY_GPIO_IO_CTRL0		0x72

//alternate settins(select butter size)
#define EYETOY_ALTERNATE_SIZE_0		0x00	// xfer disable
#define EYETOY_ALTERNATE_SIZE_384	0x01
#define EYETOY_ALTERNATE_SIZE_512	0x02
#define EYETOY_ALTERNATE_SIZE_768	0x03
#define EYETOY_ALTERNATE_SIZE_896	0x04

// device status
#define CAM_STATUS_NOTCONNECTED		0	// device not connected
#define CAM_STATUS_CONNECTED		1	// connected but initializing
#define CAM_STATUS_CONNECTEDREADY	2	// connect & ready for commands


// used with PS2CAM_DEVICE_CONFIG->mask
#define CAM_CONFIG_MASK_DIMENSION	0x00000001
#define CAM_CONFIG_MASK_OFFSET		0x00000002
#define CAM_CONFIG_MASK_DIVIDER		0x00000004
#define CAM_CONFIG_MASK_FRAMERATE	0x00000008



// error codes
#define CAM_ERROR_NONE				 (00)	// ok
#define CAM_ERROR_NOTINIT			-(20)	//
#define CAM_ERROR_INVALIDDEVICE		-(21)	// 
#define CAM_ERROR_COMMUNKNOWN		-(22)	// unknown command
#define CAM_ERROR_DEVNOTREADY		-(23)	// device not ready
#define CAM_ERROR_NODEVICE			-(24)	// no compatible device connected
#define CAM_ERROR_BADRANGE			-(25)	// a value was out of range
#define CAM_ERROR_UNKNOWN			-(26)	// unknown error
#define CAM_ERROR_MAXHANDLE			-(27)	// out of free device handle(s)
#define CAM_ERROR_BADHANDLE			-(28)	// invalid device handle
#define CAM_ERROR_DISCONNECTED		-(29)	// device was removed




typedef struct
{
	unsigned int	ssize;				// sizeof this struct		
	unsigned short	vendor_id;			// 
	unsigned short	product_id;			//
	unsigned char	vendor_name[32];	//
	unsigned char	product_name[32];	//
	unsigned char	model[16];			//
	

}PS2CAM_DEVICE_INFO;





typedef struct
{
	int			device_id;
	int			config_id;
	int			status;
	int			bandwidth;
	int			stream_pocket_size;

	int			controll;				//controll endpoint
	int			stream;					//stream   endpoint

	int			result;					//info extracted from callback
	int			bytes;					//
	void		*arg;					//

	
}CAMERA_DEVICE;




typedef struct
{
	int				fd;
	int				status;				
	CAMERA_DEVICE	*cam;

}PS2CAM_DEVICE_HANDLE;







typedef struct
{
	unsigned int	ssize;			// structure size
	unsigned int	mask;			//
	unsigned short	width;			// camera horizontal resolution
	unsigned short	height;			// camera vertical   resolution
	unsigned short	x_offset;		// x offset of camera image
	unsigned short	y_offset;		// y offset of camera image
	unsigned char	h_divider;		// Horizontal Divider
	unsigned char	v_divider;		// Vertical Divider
	unsigned short	framerate;		// framerate

}PS2CAM_DEVICE_CONFIG;





typedef struct
{
	unsigned char	magic1;	//0x01
	unsigned char	magic2;	//0xff
	unsigned char	magic3;	//0xff
	unsigned char	type;	//0x51:EOF     0x50:SOF
	unsigned char	uk1;	//?
	unsigned char	uk2;	//?
	unsigned char	uk3;	//?
	unsigned char	uk4;	//?
	unsigned char	uk5;	//?
	unsigned char	frame;	//0x01:framehead+no data, 0x00:framehead+data
	unsigned char	uk6;	//?
	unsigned char	uk7;	//?
	unsigned char	uk8;	//?
	unsigned char	uk9;	//?
	unsigned char	Lo;		//file size Lo byte (only in EOF)
	unsigned char	Hi;		//file size Hi byte (only in EOF)

}EYETOY_FRAME_HEAD;




#define camSetDIVIDER(p, hdiv, en_lpf, vdiv, en_sa)		 setReg8((p), EYETOY_IREG_DIVIDER,	 \
															((unsigned char)(hdiv)		<<0)|\
															((unsigned char)(en_lpf)	<<3)|\
															((unsigned char)(vdiv)		<<4)|\
															((unsigned char)(en_sa)		<<7));





void rpcMainThread(void* param);
void *rpcCommandHandler(u32 command, void *buffer, int size);

// usbd.irx calls
int  PS2CamInitDriver();
int  PS2CamProbe(int devId);
int  PS2CamConnect(int devId);
int  PS2CamDisconnect(int devId);
void PS2CamGetDeviceSring(CAMERA_DEVICE* dev, int index, char *str, int strmax);

//private funtions
void PS2CamCallback(int resultCode, int bytes, void *arg);
void PS2CamSetDeviceConfiguration(CAMERA_DEVICE *dev,int id);
void PS2CamInitializeNewDevice(CAMERA_DEVICE *argv);
void PS2CamSetDeviceDefaults(CAMERA_DEVICE *dev);
int PS2CamGetIRXVersion(void);
int PS2CamInit(int mode);
int PS2CamGetDeviceCount(void);
int PS2CamOpenDevice(int device_index);
int PS2CamCloseDevice(int handle);
int PS2CamGetDeviceInfo(int handle,int *info);



// low level camera funtions
int  setReg8(CAMERA_DEVICE		*dev, unsigned char reg_id, unsigned char  value);			
int  getReg8(CAMERA_DEVICE		*dev, unsigned char reg_id, unsigned char *value);
int  setReg8Mask(CAMERA_DEVICE	*dev, unsigned char reg_id, unsigned char  value,unsigned char mask);
int  setReg16(CAMERA_DEVICE		*dev, unsigned char reg_id, unsigned short value);
int  PS2CamSelectInterface(CAMERA_DEVICE* dev, int interface, int altSetting);


void camResetDevice(CAMERA_DEVICE *dev);
void camEnableAutoLaunch(CAMERA_DEVICE *dev);
void camDisableAutoLaunch(CAMERA_DEVICE *dev);
void camClearSnapButton(CAMERA_DEVICE *dev);
int  camCheckAutoLaunch(CAMERA_DEVICE *dev);
void camEnableSystem(CAMERA_DEVICE *dev);
void camDisableSystem(CAMERA_DEVICE *dev);
void camResetUsb(CAMERA_DEVICE *dev);
void camSetUsbInit(CAMERA_DEVICE *dev);
void camSetUsbWork(CAMERA_DEVICE *dev);
void camTurnOnRedLed(CAMERA_DEVICE *dev);
void camTurnOffRedLed(CAMERA_DEVICE *dev);
void camBlockStream(CAMERA_DEVICE *dev);
void camStartStream(CAMERA_DEVICE *dev);
void camStopStream(CAMERA_DEVICE *dev);


// PS2Cam rpc/export funtions
int PS2CamReadPacket(int device_id);
int PS2CamGetDeviceStatus(int device_id);
int PS2CamSetDeviceBandwidth(int device_id, char bandwidth);
int PS2CamSetLEDMode(int device_id, int mode);
int PS2CamSetDeviceConfig(int handle, void *config);






#endif	/*__PS2CAM_H__*/

