#ifndef __LIBJPG_H__
#define __LIBJPG_H__

#include <tamtypes.h>

typedef struct {
	int width;
	int height;
	int bpp;

	void *priv;
} jpgData;

jpgData *jpgOpen(char *filename);
jpgData *jpgOpenRAW(u8 *data, int size);
jpgData *jpgCreate(char *filename, u8 *data, int width, int height, int bpp);
jpgData *jpgCreateRAW(u8 *data, int width, int height, int bpp);
int      jpgCompressImage(jpgData *jpg);
int      jpgCompressImageRAW(jpgData *jpg, u8 **dest);
int      jpgReadImage(jpgData *jpg, u8 *dest);
void     jpgClose(jpgData *jpg);
int      jpgScreenshot(const char* pFilename,unsigned int VramAdress,
		       unsigned int Width, unsigned int Height, unsigned int Psm);

#endif /* __LIBJPG_H__ */
