/*      
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (c) 2004-2005 Lion | (psxdever@hotmail.com)
  ------------------------------------------------------------------------
  ps2cam.c
                           PS2CAM.irc rpc client
*/



#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include "../include/ps2cam_rpc.h"



#define PS2_CAM_RPC_ID		0x00FD000 +2



static int					CamInited = 0;
static SifRpcClientData_t	cdata			__attribute__((aligned(64)));
static char					data[1024]		__attribute__((aligned(64)));
static char					campacket[896]	__attribute__((aligned(64)));

ee_sema_t compSema;

int sem;




int PS2CamInit(int mode)
{
	unsigned int	i;
	int				ret=0;
	int				*buf;
	int				timeout;

	if(CamInited)return 0;
	
	SifInitRpc(0);

	timeout = 100000;

	while (((ret = SifBindRpc(&cdata, PS2_CAM_RPC_ID, 0)) >= 0) && (cdata.server == NULL))
		nopdelay();
	
	nopdelay();
		
	
	
	if (ret < 0)return ret;
		


	buf		= (int *)&data[0];
	buf[0]	= mode;

	printf("bind done\n");

	SifCallRpc(&cdata, PS2CAM_RPC_INITIALIZE, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);
	nopdelay();

	CamInited = 1;
	
printf("init done\n");


	compSema.init_count = 1;
	compSema.max_count = 1;
	compSema.option = 0;
	sem = CreateSema(&compSema);

printf("sema done\n");

	return buf[0];
}





int PS2CamGetIRXVersion(void)
{
	int *ret;

	ret = (int *)&data[0];

	SifCallRpc(&cdata, PS2CAM_RPC_GETIRXVERSION, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);
	nopdelay();

	return ret[0];
}





int PS2CamGetDeviceCount(void)
{
	int *ret;

	ret = (int *)&data[0];

	SifCallRpc(&cdata, PS2CAM_RPC_GETDEVCOUNT, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);
	nopdelay();

	return ret[0];
}





int PS2CamOpenDevice(int device_index)
{
	int *ret;

	ret = (int *)&data[0];

	ret[0] = device_index;

	SifCallRpc(&cdata, PS2CAM_RPC_OPENDEVICE, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);

	return ret[0];
}




int PS2CamCloseDevice(int handle)
{
	int *ret;

	ret = (int *)&data[0];

	ret[0] = handle;

	SifCallRpc(&cdata, PS2CAM_RPC_CLOSEDEVICE, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);

	return ret[0];
}





int PS2CamGetDeviceStatus(int handle)
{
	int *ret;
			

	ret = (int *)&data[0];

	ret[0] = handle;


	SifCallRpc(&cdata, PS2CAM_RPC_GETDEVSTATUS, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);
	nopdelay();

	return ret[0];
}




int PS2CamGetDeviceInfo(int handle, PS2CAM_DEVICE_INFO *info)
{
	int					*ret;
	PS2CAM_DEVICE_INFO	*iop_info;

	ret			= (int *)&data[0];
	iop_info	= (PS2CAM_DEVICE_INFO *)&ret[1];

	ret[0] = handle;

	memcpy(iop_info, info, info->ssize);
	
	SifCallRpc(&cdata, PS2CAM_RPC_GETDEVINFO, 0, (void*)(&data[0]),info->ssize+4,(void*)(&data[0]),info->ssize+4,0,0);
	nopdelay();

	memcpy(info, iop_info, iop_info->ssize);

	return ret[0];
}





int PS2CamSetDeviceBandwidth(int handle, char bandwidth)
{
	int *ret;

	ret = (int *)&data[0];

	ret[0] = handle;
	ret[1] = bandwidth;

	

	SifCallRpc(&cdata, PS2CAM_RPC_SETDEVBANDWIDTH, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);
	nopdelay();

	return ret[0];
}






int PS2CamReadPacket(int handle)
{
	int *ret;
	int *iop_addr;

	WaitSema(sem);

	ret = (int *)&data[0];

	ret[0] = handle;

	SifCallRpc(&cdata, PS2CAM_RPC_READPACKET, 0, (void*)(&data[0]),4,(void*)(&data[0]),4*2,0,0);
	

	if(ret[0] < 0) return ret[0];

	
	DI();
	ee_kmode_enter();
	
	iop_addr = (int *)(0xbc000000+ret[1]);

	memcpy(&campacket[0],iop_addr, ret[0]);
		
	ee_kmode_exit();
	EI();

	//if i add a printf here, the ps2 will exit to sony's explorer
	SignalSema(sem);
	return ret[0];
}




int PS2CamSetLEDMode(int handle, int mode)
{
	int *ret;

	ret = (int *)&data[0];

	ret[0] = handle;
	ret[1] = mode;

	SifCallRpc(&cdata, PS2CAM_RPC_SETLEDMODE, 0, (void*)(&data[0]),4,(void*)(&data[0]),4,0,0);

	return ret[0];
}



int PS2CamSetDeviceConfig(int handle, PS2CAM_DEVICE_CONFIG *cfg)
{
	int *ret;

	ret = (int *)&data[0];

	ret[0] = handle;
	
	memcpy(&ret[1], cfg, cfg->ssize);

	SifCallRpc(&cdata, PS2CAM_RPC_SETDEVCONFIG, 0, (void*)(&data[0]), 4+cfg->ssize,(void*)(&data[0]),4+cfg->ssize,0,0);

	return ret[0];
}



int PS2CamExtractFrame(int handle, char *buffer, int bufsize)
{
	static EYETOY_FRAME_HEAD	*head;
	static int			capturing;
	static int			pos;
	static int			ret;
	static int			pic_size;
	
	pos					= 0;
	capturing			= 0;



	while(1)
	{
		ret  = PS2CamReadPacket(handle);

	//	printf("packet =%d\n",ret);
		//if read has a error return it
		if(ret < 0) return ret;

		head = (EYETOY_FRAME_HEAD *)&campacket[0];
		
		if(head->magic2==0xff && head->magic3==0xff && ret!=0)
		{
			if(head->type==0x50 && head->frame==0x00)
			{
				// start of frame
			//	printf("SOF(%d)\n",ret);
				if(capturing == 1)
				{
					return 0;
				}
				
				memcpy(&buffer[pos], &campacket[16], ret-16 );
				pos += (ret-16);
				capturing = 1;
			
			}
			else if(head->type==0x50 && head->frame==0x01)
			{
				// frame head with no data
				// so do nothing

			}
			else if(head->type==0x51 && capturing==1)
			{
				//end of frame
			//	printf("EOF(%d)\n",ret);
				pic_size = (int)(((head->Lo) + ((int)(head->Hi)<<8))<<3);
				

				if(pos != pic_size)
				{
					return 0;
				}
				else
				{
					return pic_size;
				}
			}
			else
			{
				//if it doesnt fit in those pos then it must be data
				if(capturing==1)
				{
					memcpy(&buffer[pos], &campacket[0], ret );
					pos += (ret);
				}
			}
		}
		else
		{
			// it must be data
			if(capturing==1 && ret !=0)
			{
				memcpy(&buffer[pos], &campacket[0], ret );
				pos += (ret);
			}
		}
	}
	
}




