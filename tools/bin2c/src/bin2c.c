#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

unsigned char *buffer;

int main(int argc, char *argv[])
{
	int fd_size;
	FILE *source,*dest;
	int i;

	if(argc != 4) {
		printf("bin2c - from bin2s By Sjeep\n"
			   "Usage: bin2c infile outfile label %i\n\n", argc);
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
		return 1;
	}

	if(fread(buffer,1,fd_size,source) != fd_size) {
		printf("Failed to read file.\n");
		return 1;
	}
	fclose(source);

	if((dest = fopen(argv[2],"w+")) == NULL) {
		printf("Failed to open/create %s.\n",argv[2]);
		return 1;
	}
	
	fprintf(dest, "#ifndef __%s__\n", argv[3]);
	fprintf(dest, "#define __%s__\n\n", argv[3]);
	fprintf(dest, "static unsigned int size_%s = %d;\n", argv[3], fd_size);
	fprintf(dest, "static unsigned char %s[] __attribute__((aligned(16))) = {", argv[3]);

	for(i=0;i<fd_size;i+=1) {
		if((i % 16) == 0) fprintf(dest, "\n\t");
		fprintf(dest, "0x%02x, ", buffer[i]);
	}

	fprintf(dest, "\n};\n\n#endif\n");

	fclose(dest);

	return 0;
}
