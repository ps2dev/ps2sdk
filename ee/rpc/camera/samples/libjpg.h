#ifndef __LIBJPG_H__
#define __LIBJPG_H__

#include <tamtypes.h>

typedef struct {
	int width;
	int height;
	int bpp;

	void *priv;
} jpgData;

extern jpgData *jpgOpen(char *filename);
extern jpgData *jpgOpenRAW(u8 *data, int size);
extern jpgData *jpgCreate(char *filename, u8 *data, int width, int height, int bpp);
extern jpgData *jpgCreateRAW(u8 *data, int width, int height, int bpp);
extern int      jpgCompressImage(jpgData *jpg);
extern int      jpgCompressImageRAW(jpgData *jpg, u8 **dest);
extern int      jpgReadImage(jpgData *jpg, u8 *dest);
extern void     jpgClose(jpgData *jpg);
extern int      jpgScreenshot(const char* pFilename,unsigned int VramAdress,
               unsigned int Width, unsigned int Height, unsigned int Psm);

#endif /* __LIBJPG_H__ */
