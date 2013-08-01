#include <errno.h>
#include <sysclib.h>

#include "ipconfig.h"

int ParseNetAddr(const char *address, unsigned char *octlets){
	int result;
	char address_copy[16], *octlet;
	unsigned char i;

	if(strlen(address)<16){
		strcpy(address_copy, address);

		if((octlet=strtok(address_copy, "."))!=NULL){
			result=0;

			octlets[0]=strtoul(octlet, NULL, 10);
			for(i=1; i<4; i++){
				if((octlet=strtok(NULL, "."))==NULL){
					result=EINVAL;
					break;
				}
				else octlets[i]=strtoul(octlet, NULL, 10);
			}
		}
		else result=EINVAL;
	}
	else result=EINVAL;


	return result;
}

