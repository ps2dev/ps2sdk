#include "../include/ps2cam_rpc.h"
#include "libjpg.h"
#include "fileio.h"
#include "Kernel.h"


char jpg_buffer[500512] __attribute__((aligned(64)));



jpgData		*info;

PS2CAM_DEVICE_INFO		caminfo;

PS2CAM_DEVICE_CONFIG	cfg;


int *iopval;


int main (int argc, char *argv[])
{
	static int i;
	static int ret;
	static int devid;
	static int fh;

	SifInitRpc(0);
	fioInit();



	SifLoadModule("rom0:SIO2MAN", 0, 0);
	SifLoadModule("host:usbd.irx", 0, 0);
	SifLoadModule("host:ps2cam.irx", 0, 0);


	printf("binding...\n");

	PS2CamInit(0);

	

	
	while(1)
	{
	
		ret = PS2CamGetDeviceCount();
		printf("there is %d camera connected\n",ret);
		if(ret > 0)break;
	}
	
	devid = PS2CamOpenDevice(0);
	printf("device %d ID == %d\n",0,devid);

	



	while(1)
	{
		ret = PS2CamGetDeviceStatus(devid);
		if(ret==2)break;
	}
	


	printf("status =%d\n",ret);
	
	ret = PS2CamSetDeviceBandwidth(devid, 4);
	printf("set Bandwidth = %d\n",ret);

	cfg.ssize	= sizeof(cfg);
	cfg.mask	= CAM_CONFIG_MASK_DIMENSION|CAM_CONFIG_MASK_DIVIDER|CAM_CONFIG_MASK_OFFSET|CAM_CONFIG_MASK_FRAMERATE;
	cfg.width	= 640;
	cfg.height	= 480;
	cfg.x_offset =0x00;
	cfg.y_offset =0x00;
	cfg.h_divider =0;
	cfg.v_divider =0;
	cfg.framerate =30;

	PS2CamSetDeviceConfig(devid, &cfg);





	caminfo.ssize = sizeof(caminfo);
	PS2CamGetDeviceInfo(devid, &caminfo);

	printf("info = %s (%d)\n",(char *)&caminfo.product_name[0],caminfo.ssize);

	


	// capture and save jpg to host

	// loop and capture a valid frame
	while(1)
	{
		ret = PS2CamExtractFrame(devid, jpg_buffer, 500512);
		if(ret>0)break; // valid frame
		if(ret<0)break;	// error

		printf("frame skiped\n");
	}




	
	
	if(ret>0)
	{
		printf("frame captured\n");

		fh = fioOpen("host:image.jpg", O_WRONLY|O_CREAT|O_TRUNC);


		fioWrite(fh, jpg_buffer, ret);
	


		fioClose(fh);
	}
	else
	{
		printf("error capturing image (%d)\n",ret);
	}
	






	while(1)
	{
		//do nothing
	}




	return 0;
}



int __iob;


int fprintf(int f, const char *s)
{

}
