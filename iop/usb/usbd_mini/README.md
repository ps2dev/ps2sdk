# usbd_mini — FreeUsbd (pre-rewrite)

This module is built from the **FreeUsbd** sources last known good before
ps2sdk commit `b1f7ff96` (“USBD feature update”, 2024-09-04), i.e. tree
`2dc6b32f`.

The full `iop/usb/usbd` tree remains the SCE rewrite (usbd 1.2 with HID report
descriptors and multi-isochronous). `usbd_mini` is what OPL and other BDM USB
loaders embed; restoring FreeUsbd here fixes Crash Bandicoot: Wrath of Cortex
over USB on hardware while leaving the rewrite available for other consumers.

Export table stays **usbd 1.2** with stubs for `usbdReboot`,
`sceUsbdGetReportDescriptor`, and `sceUsbdMultiIsochronousTransfer`
(`USB_RC_NOSUPPORT` / no-op). Mass storage does not use those entry points.

See: https://github.com/ps2dev/ps2sdk/issues/908
