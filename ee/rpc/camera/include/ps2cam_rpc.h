/*
  PSX2 OpenSource Project
  (C)2004-2005 Lion[PS2Dev]
  (C)2004-2005 PS2Dev.org
------------------------------------------------------------------------
  ps2cam_rpc.h
					PS2Camera EE-IOP RPC CALLS
			
*/
#ifndef _PS2CAM_RPC_H
#define _PS2CAM_RPC_H




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

// led modes
#define PS2CAM_LED_MODE_OFF			0
#define PS2CAM_LED_MODE_ON			1
//#define PS2CAM_LED_MODE_FLASH		2


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



typedef struct
{
	unsigned int	ssize;				// sizeof this struct		
	unsigned short	vendor_id;			// vendor  id extracted from device
	unsigned short	product_id;			// product id extracted from device
	unsigned char	vendor_name[32];	// vender string
	unsigned char	product_name[32];	// product string
	unsigned char	model[16];			// device model
	

}PS2CAM_DEVICE_INFO;



typedef struct
{
	unsigned int	ssize;			// structure size
	unsigned int	mask;			// CAM_CONFIG_MASK.....
	unsigned short	width;			// camera horizontal resolution
	unsigned short	height;			// camera vertical   resolution
	unsigned short	x_offset;		// x offset of camera image
	unsigned short	y_offset;		// y offset of camera image
	unsigned char	h_divider;		// Horizontal Divider
	unsigned char	v_divider;		// Vertical Divider
	unsigned short	framerate;		// framerate

}PS2CAM_DEVICE_CONFIG;






extern char		campacket[];		//data is stored here when PS2CamReadPacket(...) is called


#ifdef __cplusplus
extern "C" {
#endif

//ee funtions
int PS2CamInit(int mode);
int PS2CamGetIRXVersion(void);
int PS2CamGetDeviceCount(void);
int PS2CamOpenDevice(int device_index);
int PS2CamCloseDevice(int handle);
int PS2CamGetDeviceStatus(int handle);
int PS2CamGetDeviceInfo(int handle, PS2CAM_DEVICE_INFO *info);
int PS2CamSetDeviceBandwidth(int handle, char bandwidth);
int PS2CamReadPacket(int handle);
int PS2CamSetLEDMode(int handle, int mode);
int PS2CamSetDeviceConfig(int handle, PS2CAM_DEVICE_CONFIG *cfg);

int PS2CamExtractFrame(int handle, char *buffer, int bufsize);


#ifdef __cplusplus
}
#endif
























#endif /*_PS2CAM_RPC_H*/
