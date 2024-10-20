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

int handleStaticDeviceDescriptor(UsbdDevice_t *dev, UsbDeviceDescriptor *devDescStart, UsbDeviceDescriptor *devDescEnd)
{
	UsbDeviceDescriptor *devDescCur;
	u32 ifNum;
	unsigned int bLength;
	int i;
	u32 wItemLength;
	UsbdReportDescriptor_t *hidDescriptor;
	int state;
	u32 cfgNum;
	UsbHidDescriptor *usbHidDescriptor;
	const UsbConfigDescriptor *usbConfigDescriptor;
	const UsbInterfaceDescriptor *usbInterfaceDescriptor;

	dev->m_reportDescriptorStart = NULL;
	dev->m_reportDescriptorEnd = NULL;
	cfgNum = 0;
	ifNum = 0;
	for ( devDescCur = devDescStart; devDescCur < devDescEnd;
				devDescCur = (UsbDeviceDescriptor *)((u8 *)devDescCur + bLength) )
	{
		bLength = devDescCur->bLength;
		if ( bLength < 2 )
			break;
		if ( (u8 *)devDescEnd - (u8 *)devDescCur < (int)bLength )
			break;
		switch ( devDescCur->bDescriptorType )
		{
			case 1u:
			case 3u:
			case 5u:
			case 6u:
			case 7u:
			case 8u:
			case 9u:
			case 0xAu:
			case 0xBu:
			case 0xCu:
			case 0xDu:
			case 0xEu:
			case 0xFu:
			case 0x10u:
			case 0x11u:
			case 0x12u:
			case 0x13u:
			case 0x14u:
			case 0x15u:
			case 0x16u:
			case 0x17u:
			case 0x18u:
			case 0x19u:
			case 0x1Au:
			case 0x1Bu:
			case 0x1Cu:
			case 0x1Du:
			case 0x1Eu:
			case 0x1Fu:
			case 0x20u:
				break;
			case 2u:
				if ( (int)bLength >= 9 )
				{
					usbConfigDescriptor = (UsbConfigDescriptor *)devDescCur;
					cfgNum = usbConfigDescriptor->bConfigurationValue;
				}
				break;
			case 4u:
				if ( (int)bLength >= 9 )
				{
					usbInterfaceDescriptor = (UsbInterfaceDescriptor *)devDescCur;
					ifNum = usbInterfaceDescriptor->bInterfaceNumber;
				}
				break;
			case 0x21u:
				usbHidDescriptor = (UsbHidDescriptor *)devDescCur;
				for ( i = 0; i < usbHidDescriptor->bNumDescriptors; i += 1 )
				{
					if ( usbHidDescriptor->items[i].bDescriptorType == 0x22 )
					{
						wItemLength =
							usbHidDescriptor->items[i].wDescriptorLengthLb + (usbHidDescriptor->items[i].wDescriptorLengthHb << 8);
						CpuSuspendIntr(&state);
						hidDescriptor =
							(UsbdReportDescriptor_t *)AllocSysMemory(ALLOC_FIRST, wItemLength + sizeof(UsbdReportDescriptor_t), NULL);
						CpuResumeIntr(state);
						hidDescriptor->m_ifNum = ifNum;
						hidDescriptor->m_length = wItemLength;
						hidDescriptor->m_cfgNum = cfgNum;
						hidDescriptor->m_prev = dev->m_reportDescriptorEnd;
						if ( dev->m_reportDescriptorEnd )
							dev->m_reportDescriptorEnd->m_next = hidDescriptor;
						else
							dev->m_reportDescriptorStart = hidDescriptor;
						hidDescriptor->m_next = NULL;
						dev->m_reportDescriptorEnd = hidDescriptor;
					}
				}
				break;
			default:
				break;
		}
	}
	return 0;
}
