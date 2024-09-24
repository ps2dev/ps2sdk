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

void usbdRebootInner(void)
{
	memPool->m_ohciRegs->HcRhPortStatus[0] = BIT(PORT_RESET);
	memPool->m_ohciRegs->HcRhPortStatus[1] = BIT(PORT_RESET);
}

void hubResetDevice(UsbdDevice_t *dev)
{
	if ( memPool->m_delayResets )
	{
		dev->m_deviceStatus = DEVICE_RESETDELAYED;
	}
	else
	{
		memPool->m_delayResets = 1;
		dev->m_deviceStatus = DEVICE_RESETPENDING;
		dev->m_resetFlag = 1;
		if ( dev->m_parent == memPool->m_deviceTreeRoot )  // root hub port
			memPool->m_ohciRegs->HcRhPortStatus[dev->m_attachedToPortNo - 1] = BIT(PORT_RESET);
		else  // normal hub port
			hubResetDevicePort(dev);
	}
}

static int checkDelayedResetsTree(UsbdDevice_t *tree)
{
	UsbdDevice_t *dev;

	for ( dev = tree->m_childListStart; dev; dev = dev->m_next )
	{
		if ( dev->m_deviceStatus == DEVICE_RESETDELAYED )
		{
			hubResetDevice(dev);
			return 1;
		}
		if ( dev->m_childListStart )
		{
			if ( (u8)((int)dev->m_deviceStatus - 7) < 2u )
			{
				if ( checkDelayedResetsTree(dev) != 0 )
					return 1;
			}
		}
	}
	return 0;
}

int checkDelayedResets(UsbdDevice_t *dev)
{
	memPool->m_delayResets = 0;
	dev->m_resetFlag = 0;
	checkDelayedResetsTree(memPool->m_deviceTreeRoot);
	return 0;
}

void handleRhsc(void)
{
	u32 portNum;
	UsbdDevice_t *port;
	UsbdDevice_t *next;

	for ( portNum = 0, port = memPool->m_deviceTreeRoot->m_childListStart; port; portNum += 1, port = next )
	{
		u32 status;

		next = port->m_next;
		status = memPool->m_ohciRegs->HcRhPortStatus[portNum];
		memPool->m_ohciRegs->HcRhPortStatus[portNum] = C_PORT_FLAGS;  // reset all flags
		if ( (status & BIT(PORT_CONNECTION)) != 0 )
		{
			if ( port->m_deviceStatus >= (unsigned int)DEVICE_CONNECTED && ((status & BIT(C_PORT_CONNECTION)) != 0) )
				flushPort(port);
			if ( port->m_deviceStatus == DEVICE_NOTCONNECTED )
			{
				port->m_deviceStatus = DEVICE_CONNECTED;
				addTimerCallback(&port->m_timer, (TimerCallback)hubResetDevice, port, 501);
			}
			else if ( port->m_deviceStatus == DEVICE_RESETPENDING )
			{
				if ( (status & BIT(PORT_RESET)) == 0 )
				{
					port->m_deviceStatus = DEVICE_RESETCOMPLETE;
					port->m_isLowSpeedDevice = (status & BIT(PORT_LOW_SPEED)) != 0;
					if ( openDeviceEndpoint(port, NULL, 0) )
						hubTimedSetFuncAddress(port);
				}
			}
		}
		else
		{
			flushPort(port);
		}
	}
}
