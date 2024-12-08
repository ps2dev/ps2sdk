#ifndef _PART_DRIVER_H
#define _PART_DRIVER_H 1

#ifdef BUILDING_USBHDFSD
extern int part_connect(mass_dev *dev);
extern void part_disconnect(mass_dev *dev);
#endif /* BUILDING_USBHDFSD */
#ifdef BUILDING_IEEE1394_DISK
extern int part_connect(struct SBP2Device *dev);
extern void part_disconnect(struct SBP2Device *dev);
#endif /* BUILDING_IEEE1394_DISK */

#endif
