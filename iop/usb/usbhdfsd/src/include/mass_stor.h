#ifndef _MASS_STOR_H
#define _MASS_STOR_H 1

int InitUSB(void);
int mass_stor_disconnect(int devId);
int mass_stor_connect(int devId);
int mass_stor_probe(int devId);
int mass_stor_readSector(mass_dev* dev, unsigned int sector, unsigned char* buffer, unsigned short int count);
int mass_stor_writeSector(mass_dev* dev, unsigned int sector, const unsigned char* buffer, unsigned short int count);
int mass_stor_configureNextDevice(void);
int mass_stor_stop_unit(mass_dev* dev);
void mass_store_stop_all(void);

#endif
