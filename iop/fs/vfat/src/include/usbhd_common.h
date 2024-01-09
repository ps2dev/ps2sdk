#ifndef _USBHD_COMMON_H
#define _USBHD_COMMON_H

#ifdef WIN32
#define USBHD_INLINE
typedef unsigned char u8;
#else
#define USBHD_INLINE inline
void *malloc(int size);
void free(void *ptr);
#endif

#ifdef BUILDING_IEEE1394_DISK
struct _cache_set;
typedef struct _cache_set cache_set;
#endif /* BUILDING_IEEE1394_DISK */

//---------------------------------------------------------------------------
#define getUI32(buf) ((unsigned int)(((u8 *)(buf))[0] +         \
                                     (((u8 *)(buf))[1] << 8) +  \
                                     (((u8 *)(buf))[2] << 16) + \
                                     (((u8 *)(buf))[3] << 24)))
#define getUI32_2(buf1, buf2) ((unsigned int)(((u8 *)(buf1))[0] +         \
                                              (((u8 *)(buf1))[1] << 8) +  \
                                              (((u8 *)(buf2))[0] << 16) + \
                                              (((u8 *)(buf2))[1] << 24)))
#define getUI16(buf) ((unsigned int)(((u8 *)(buf))[0] + (((u8 *)(buf))[1] << 8)))

#endif // _USBHD_COMMON_H
