/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include "usbdpriv.h"

int callUsbDriverFunc(int (*func)(int devId), int devId, void *gpSeg)
{
	int res;

	if ( !func )
		return 0;
	usbdUnlock();
#if USE_GP_REGISTER
	SetGP(gpSeg);
#endif
	res = func(devId);
#if USE_GP_REGISTER
	SetGP(_gp);
#endif
	usbdLock();
	return res;
}

static void probeDeviceTree(UsbdDevice_t *tree, sceUsbdLddOps *drv)
{
	UsbdDevice_t *curDevice;

	for ( curDevice = tree->m_childListStart; curDevice; curDevice = curDevice->m_next )
	{
		if ( curDevice->m_deviceStatus == DEVICE_READY )
		{
			if ( curDevice->m_devDriver )
			{
				if ( curDevice->m_childListStart )
					probeDeviceTree(curDevice, drv);
			}
			else
			{
				curDevice->m_privDataField = NULL;
				if ( callUsbDriverFunc(drv->probe, curDevice->m_id, drv->gp) )
				{
					curDevice->m_devDriver = drv;
					callUsbDriverFunc(drv->connect, curDevice->m_id, drv->gp);
				}
			}
		}
	}
}

int doRegisterDriver(sceUsbdLddOps *drv, void *drvGpSeg)
{
	if ( drv->next || drv->prev || !drv->name || drv->reserved1 || drv->reserved2 )
	{
		return USB_RC_BADDRIVER;
	}
	if ( drvListStart == drv )
	{
		return USB_RC_BUSY;
	}
	drv->gp = drvGpSeg;
	drv->prev = drvListEnd;
	if ( drvListEnd )
		drvListEnd->next = drv;
	else
		drvListStart = drv;
	drv->next = NULL;
	drvListEnd = drv;
	if ( drv->probe )
	{
		probeDeviceTree(getDeviceTreeRoot(), drv);
	}
	return USB_RC_OK;
}

int doRegisterAutoLoader(sceUsbdLddOps *drv, void *drvGpSeg)
{
	if ( drv->next || drv->prev || !drv->name || drv->reserved1 || drv->reserved2 )
	{
		return USB_RC_BADDRIVER;
	}
	if ( drvAutoLoader )
	{
		return USB_RC_BUSY;
	}
	drv->gp = drvGpSeg;
	drvAutoLoader = drv;
	if ( drv->probe )
	{
		probeDeviceTree(getDeviceTreeRoot(), drv);
	}
	return USB_RC_OK;
}

static void disconnectDriver(UsbdDevice_t *tree, sceUsbdLddOps *drv)
{
	UsbdEndpoint_t *ep;
	UsbdEndpoint_t *nextEp;
	UsbdDevice_t *tree_tmp1;

	if ( drv == tree->m_devDriver )
	{
		if ( tree->m_endpointListStart )
		{
			ep = tree->m_endpointListStart->m_next;
			for ( nextEp = ep; nextEp; ep = nextEp )
			{
				nextEp = ep->m_next;
				removeEndpointFromDevice(tree, ep);
			}
		}
		tree->m_devDriver = NULL;
		tree->m_privDataField = NULL;
	}
	for ( tree_tmp1 = tree->m_childListStart; tree_tmp1; tree_tmp1 = tree_tmp1->m_next )
	{
		disconnectDriver(tree_tmp1, drv);
	}
}

int doUnregisterDriver(sceUsbdLddOps *drv)
{
	sceUsbdLddOps *pos;

	for ( pos = drvListStart; pos && pos != drv; pos = pos->next )
	{
	}
	if ( !pos )
		return USB_RC_BADDRIVER;
	if ( drv->next )
		drv->next->prev = drv->prev;
	else
		drvListEnd = drv->prev;
	if ( drv->prev )
		drv->prev->next = drv->next;
	else
		drvListStart = drv->next;
	disconnectDriver(getDeviceTreeRoot(), drv);
	return USB_RC_OK;
}

int doUnregisterAutoLoader(void)
{
	drvAutoLoader = NULL;
	return USB_RC_OK;
}
