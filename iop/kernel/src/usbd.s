/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# USB Driver Import functions.
*/

	.text
	.set	noreorder

	.local	usbd_stub
usbd_stub:					# Module Import Information
	.word	0x41e00000		# Import Tag
	.word	0x00000000		# Minor Version?
	.word	0x00000101		# Major Version?
	.ascii	"usbd\0\0\0\0"	# Library ID
	.align	2

	/* initialize USBD.IRX
	 * Note: UsbInit is automatically called first whenever USBD.IRX is loaded.
	 * There should never be a need to reinitialize the driver.  In fact, it may
	 * not even work.  But I'm providing the function hook anyhow. <shrug>
	 */
	.globl	UsbInit
UsbInit:
	j	$31
	li	$0, 0x00

	/*
	 * These two functions are used to register and unregister device drivers for
	 * listening for USB bus events.  The events are device probe, connect, and disconnect.
	 */

	 # register a USB device driver
	.globl	sceUsbdRegisterLdd
sceUsbdRegisterLdd:
	j	$31
	li	$0, 0x04

	# unregister a USB device driver
	.globl	sceUsbdUnregisterLdd
sceUsbdUnregisterLdd:
	j	$31
	li	$0, 0x05

	/*
	 * This function is used to get the static descriptors for the specific USB
	 * device.  These descriptors identify the device uniquely and help determine
	 * what type of device we are dealing with, and what its capabilities and
	 * features are.
	 */
	.globl	sceUsbdScanStaticDescriptor
sceUsbdScanStaticDescriptor:
	j	$31
	li	$0, 0x06

	/*
	 * These two functions are used to assign relevant data to a specific device.
	 * The type of data is entirely up to the caller.  For example, a particular
	 * USB device driver may store configuration data for each specific device
	 * under its control.
	 */

	# set the private data pointer for a device
	.globl	sceUsbdSetPrivateData
sceUsbdSetPrivateData:
	j	$31
	li	$0, 0x07

	# get the private data pointer for a device
	.globl	sceUsbdGetPrivateData
sceUsbdGetPrivateData:
	j	$31
	li	$0, 0x08

	/*
	 * This function returns an endpoint ID for the device ID and endpoint descriptor
	 * passed in.  This endpoint ID is then used when transfering data to the device,
	 * and to close the endpoint.
	 */
	.globl	sceUsbdOpenPipe
sceUsbdOpenPipe:
	j	$31
	li	$0, 0x09

	# close an endpoint
	.globl	sceUsbdClosePipe
sceUsbdClosePipe:
	j	$31
	li	$0, 0x0A

	/*
	 * This function is used for all types of USB data transfers.  Which type of
	 * transfer is determined by the parameters that are passed in.  The types are:
	 * control, isochronous, interrupt, and bulk transfers.  More details can be
	 * found in usbd.h.
	 */
	.globl	sceUsbdTransferPipe
sceUsbdTransferPipe:
	j	$31
	li	$0, 0x0B

	.globl	UsbOpenBulkEndpoint
UsbOpenBulkEndpoint:
	j	$31
	li	$0, 0x0C

	.word	0
	.word	0
