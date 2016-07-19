#ifndef _APA_OPT_H
#define _APA_OPT_H

#define APA_PRINTF(format,...)	printf(format, ##__VA_ARGS__)
#define APA_DRV_NAME			"hdd"

//Define to build an OSD version
//#define APA_OSD_VER	1

#ifndef APA_OSD_VER
#define APA_ENABLE_PASSWORDS		1
#define APA_FORMAT_MAKE_PARTITIONS	1
#endif

#endif
