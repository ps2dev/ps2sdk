/*
  PSX2 OpenSource Project
  (C)2004-2005 Lion[PS2Dev]
  (C)2004-2005 PS2Dev.org
*/

/**
 * @file
 * PS2Camera EE-IOP RPC CALLS
 */

#ifndef __PS2CAM_RPC_H__
#define __PS2CAM_RPC_H__

/** get irx version */
#define PS2CAM_RPC_GETIRXVERSION	40
/** PS2CamInit */
#define PS2CAM_RPC_INITIALIZE		41
/** PS2CamGetDeviceCount */
#define PS2CAM_RPC_GETDEVCOUNT		42
#define PS2CAM_RPC_OPENDEVICE		43
#define PS2CAM_RPC_CLOSEDEVICE		44
/** PS2CamGetStatus */
#define PS2CAM_RPC_GETDEVSTATUS		45
/** PS2CamGetInfo */
#define PS2CAM_RPC_GETDEVINFO		46
/** PS2CamSetDeviceBandwidth */
#define PS2CAM_RPC_SETDEVBANDWIDTH	47
/** PS2CamReadPacketCOMMAND */
#define PS2CAM_RPC_READPACKET		48
/** PS2CamSetLEDMode */
#define PS2CAM_RPC_SETLEDMODE		49

#define PS2CAM_RPC_SETDEVCONFIG		50

/* led modes */
#define PS2CAM_LED_MODE_OFF			0
#define PS2CAM_LED_MODE_ON			1
//#define PS2CAM_LED_MODE_FLASH		2

/* device status */
/** device not connected */
#define CAM_STATUS_NOTCONNECTED		0
/** connected but initializing */
#define CAM_STATUS_CONNECTED		1
/** connect & ready for commands */
#define CAM_STATUS_CONNECTEDREADY	2

/* used with PS2CAM_DEVICE_CONFIG->mask */
#define CAM_CONFIG_MASK_DIMENSION	0x00000001
#define CAM_CONFIG_MASK_OFFSET		0x00000002
#define CAM_CONFIG_MASK_DIVIDER		0x00000004
#define CAM_CONFIG_MASK_FRAMERATE	0x00000008

/* error codes */
/** ok */
#define CAM_ERROR_NONE				 (00)
#define CAM_ERROR_NOTINIT			-(20)
#define CAM_ERROR_INVALIDDEVICE		-(21)
/** unknown command */
#define CAM_ERROR_COMMUNKNOWN		-(22)
/** device not ready */
#define CAM_ERROR_DEVNOTREADY		-(23)
/** no compatible device connected */
#define CAM_ERROR_NODEVICE			-(24)
/** a value was out of range */
#define CAM_ERROR_BADRANGE			-(25)
/** unknown error */
#define CAM_ERROR_UNKNOWN			-(26)
/** out of free device handle(s) */
#define CAM_ERROR_MAXHANDLE			-(27)
/** invalid device handle */
#define CAM_ERROR_BADHANDLE			-(28)
/** device was removed */
#define CAM_ERROR_DISCONNECTED		-(29)

typedef struct
{
	/** 0x01 */
	unsigned char	magic1;
	/** 0xff */
	unsigned char	magic2;
	/** 0xff */
	unsigned char	magic3;
	/** 0x51:EOF     0x50:SOF */
	unsigned char	type;
	unsigned char	uk1;
	unsigned char	uk2;
	unsigned char	uk3;
	unsigned char	uk4;
	unsigned char	uk5;
	/** 0x01:framehead+no data, 0x00:framehead+data */
	unsigned char	frame;
	unsigned char	uk6;
	unsigned char	uk7;
	unsigned char	uk8;
	unsigned char	uk9;
	/** file size Lo byte (only in EOF) */
	unsigned char	Lo;
	/** file size Hi byte (only in EOF) */
	unsigned char	Hi;
}EYETOY_FRAME_HEAD;

typedef struct
{
	/** sizeof this struct */
	unsigned int	ssize;
	/** vendor  id extracted from device */
	unsigned short	vendor_id;
	/** product id extracted from device */
	unsigned short	product_id;
	/** vender string */
	unsigned char	vendor_name[32];
	/** product string */
	unsigned char	product_name[32];
	/** device model */
	unsigned char	model[16];
}PS2CAM_DEVICE_INFO;


typedef struct
{
	/** structure size */
	unsigned int	ssize;
	/** CAM_CONFIG_MASK..... */
	unsigned int	mask;
	/** camera horizontal resolution */
	unsigned short	width;
	/** camera vertical   resolution */
	unsigned short	height;
	/** x offset of camera image */
	unsigned short	x_offset;
	/** y offset of camera image */
	unsigned short	y_offset;
	/** Horizontal Divider */
	unsigned char	h_divider;
	/** Vertical Divider */
	unsigned char	v_divider;
	/** framerate */
	unsigned short	framerate;
}PS2CAM_DEVICE_CONFIG;

//extern char		campacket[];		//data is stored here when PS2CamReadPacket(...) is called. Still not used anywhere

#ifdef __cplusplus
extern "C" {
#endif

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

#endif /* __PS2CAM_RPC_H__ */
