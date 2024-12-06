#ifndef _MASS_STOR_H
#define _MASS_STOR_H 1

extern int InitUSB(void);
extern int mass_stor_disconnect(int devId);
extern int mass_stor_connect(int devId);
extern int mass_stor_probe(int devId);
extern int mass_stor_readSector(mass_dev *dev, unsigned int sector, unsigned char *buffer, unsigned short int count);
extern int mass_stor_writeSector(mass_dev *dev, unsigned int sector, const unsigned char *buffer, unsigned short int count);
extern int mass_stor_configureNextDevice(void);
extern int mass_stor_stop_unit(mass_dev *dev);
extern void mass_store_stop_all(void);

#endif
