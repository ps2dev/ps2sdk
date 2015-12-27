#ifndef _APA_OPT_H
#define _APA_OPT_H

#define APA_PRINTF(format,...)	printf(format, ##__VA_ARGS__)
#define APA_DRV_NAME			"hdd"

//#define APA_FULL_INPUT_ARGS		1	//Uncomment to enable full input (partitionID,fpswd,rpswd,size,filesystem instead of just partitionID,size).

#ifdef APA_FULL_INPUT_ARGS
#define APA_ENABLE_PASSWORDS	1	//Uncomment to enable passwords. APA_FULL_INPUT_ARGS has to be defined.
#endif

#endif
