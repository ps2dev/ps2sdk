#ifndef __PS2SCREENSHOT_H__
#define __PS2SCREENSHOT_H__

//#ifdef __cplusplus
//extern "C" {
//#endif

int ps2_screenshot_file( const char* pFilename,unsigned int VramAdress, 
                         unsigned int Width, unsigned int Height, unsigned int Psm );

int ps2_screenshot( void* pTemp, unsigned int VramAdress,unsigned int x,unsigned int y,
                    unsigned int Width, unsigned int Height, unsigned int Psm );

//#ifdef __cplusplus
//}
//#endif

#endif //__PS2SCREENSHOT_H__
