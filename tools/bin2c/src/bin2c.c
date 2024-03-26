/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2024, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    size_t fd_size, i;
	FILE *source,*dest;
    unsigned char *buffer;

	if(argc != 4) {
		printf("bin2c - from bin2s By Sjeep\n"
			   "Usage: bin2c infile outfile label\n\n");
		return 1;
	}

	if((source=fopen( argv[1], "rb")) == NULL) {
		printf("Error opening %s for reading.\n",argv[1]);
		return 1;
	}

	fseek(source,0,SEEK_END);
	fd_size = ftell(source);
	fseek(source,0,SEEK_SET);

	buffer = malloc(fd_size);
	if(buffer == NULL) {
		printf("Failed to allocate memory.\n");
		fclose(source);
		return 1;
	}

	if(fread(buffer,1,fd_size,source) != fd_size) {
		printf("Failed to read file.\n");
		fclose(source);
        free(buffer);
		return 1;
	}
	fclose(source);

	if((dest = fopen(argv[2],"w+")) == NULL) {
		printf("Failed to open/create %s.\n",argv[2]);
        free(buffer);
		return 1;
	}

	fprintf(dest, "#ifndef __%s__\n", argv[3]);
	fprintf(dest, "#define __%s__\n\n", argv[3]);
	fprintf(dest, "unsigned int size_%s = %zu;\n", argv[3], fd_size);
	fprintf(dest, "unsigned char %s[] __attribute__((aligned(16))) = {", argv[3]);

	for(i=0;i<fd_size;i+=1) {
		if((i % 16) == 0) fprintf(dest, "\n\t");
		fprintf(dest, "0x%02x, ", buffer[i]);
	}

	fprintf(dest, "\n};\n\n#endif\n");

    free(buffer);
	fclose(dest);

	return 0;
}
