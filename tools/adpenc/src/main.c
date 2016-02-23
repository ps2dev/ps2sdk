#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define VERSION	"1.1"

extern int adpcm_encode(FILE* fp, FILE* sad, int offset, int sample_len, int flag_loop);

enum{ monoF = (1 << 0), stereo = (1 << 1), loopF = (1 << 2) } sad_flag;

struct AdpcmHeader{
	char id[4];	//"APCM"
	unsigned char version;
	unsigned char channels;
	unsigned char loop;
	unsigned char reserved;
	unsigned int pitch;
	unsigned int reserved2;
};

static int ConvertFile(const char *InputFile, const char *OutputFile, int flag_loop){
	FILE *fp, *sad;
	int sample_freq, sample_len, result;
	char s[4];
	int chunk_data;
	short e;
	char channels;
	struct AdpcmHeader AdpcmHeader;

	result=0;
	if ( (fp = fopen(InputFile, "rb" )) != NULL )
	{
		if (fread( s, 1, 4, fp )!=4 || strncmp( s, "RIFF", 4 ) )
		{
			printf( "Error: Not a WAVE-file (\"RIFF\" identifier not found)\n" );
			result=-3;
			goto InputFileIOEnd;
		}

		fseek( fp, 8, SEEK_SET );

		if (fread( s, 1, 4, fp )!=4 || strncmp( s, "WAVE", 4 ) )
		{
			printf( "Error: Not a WAVE-file (\"WAVE\" identifier not found)\n" );
			result=-3;
 			goto InputFileIOEnd;
		}

		fseek( fp, 8 + 4, SEEK_SET );

		if (fread( s, 1, 4, fp )!=4 || strncmp( s, "fmt", 3 ) )
		{
			printf( "Error: Not a WAVE-file (\"fmt\" chunk not found)\n" );
			result=-3;
 			goto InputFileIOEnd;
		}

		if(fread( &chunk_data, 4, 1, fp )==1){
			chunk_data += ftell( fp );

			if (fread( &e, 2, 1, fp )!=1 || e != 1 )
			{
				printf( "Error: No PCM data in WAVE-file\n" );
				result=-4;
	 			goto InputFileIOEnd;
			}
		}
		else{
			printf( "Error: can't read CHUNK DATA in WAVE-file\n" );
			result=-4;
 			goto InputFileIOEnd;
		}

		if (fread( &e, 2, 1, fp )!=1 || ((e != 1) && (e != 2)))
		{
			printf( "Error: WAVE file must be MONO or STEREO (max 2 channels)\n" );
			result=-5;
 			goto InputFileIOEnd;
		}

		channels = e;

		if(fread( &sample_freq, 4, 1, fp )==1){
			fseek( fp, 4 + 2, SEEK_CUR );
			if (fread( &e, 2, 1, fp )!=1 || e != 16 )
			{
				printf( "Error: WAVE-file must 16 bit\n" );
				result=-6;
	 			goto InputFileIOEnd;
			}
		}
		else{
			printf("Error: Can't read SAMPLE FREQUENCY in WAVE-file\n");
			result=-6;
 			goto InputFileIOEnd;
		}

		fseek( fp, chunk_data, SEEK_SET );
		if(fread( s, 1, 4, fp )==4){
			// Skip 'fact' and possibly other chunks
			while(strncmp( s, "data", 4 ))
			{
				if(fread( &chunk_data, 4, 1, fp )==1){
					chunk_data += ftell( fp );
					fseek( fp, chunk_data, SEEK_SET );
					if(fread( s, 1, 4, fp )!=4){
						printf("Error: Read error in WAVE-file\n");
						result=-6;
			 			goto InputFileIOEnd;
					}
				}
				else{
					printf("Error: Read error in WAVE-file\n");
					result=-6;
		 			goto InputFileIOEnd;
				}
			}
		}
		else{
			printf("Error: Read error in WAVE-file\n");
			result=-6;
 			goto InputFileIOEnd;
		}

		if(fread( &sample_len, 4, 1, fp )==1){
			sample_len /= (channels*2);
		}
		else{
			printf("Error: Can't read SAMPLE LENGTH in WAVE-file\n");
			result=-6;
 			goto InputFileIOEnd;
		}

		if ( (sad = fopen(OutputFile, "wb" )) != NULL )
		{
			strncpy(AdpcmHeader.id, "APCM", 4);
			AdpcmHeader.version=1;
			AdpcmHeader.channels=channels;
			AdpcmHeader.loop=flag_loop;
			AdpcmHeader.reserved=0;
			AdpcmHeader.pitch=(sample_freq*4096)/48000;	// pitch, to encode for PS1 change 48000 to 44100
			AdpcmHeader.reserved2=0;
			fwrite(&AdpcmHeader, sizeof(AdpcmHeader), 1, sad);

			if(channels == 1)
			{
				result=adpcm_encode(fp, sad, 0, sample_len, flag_loop);
			}
			else
			{
				int data_offset = ftell(fp);

				// Encode left
				if((result=adpcm_encode(fp, sad, 2, sample_len, flag_loop))==0){
					fseek(fp, data_offset+2, SEEK_SET);
					// Encode right
					result=adpcm_encode(fp, sad, 2, sample_len, flag_loop);
				}
			}

			fclose(sad);
		}
		else
		{
			printf( "Error: Can't write output file %s\n", OutputFile);
			result=EIO;
		}

InputFileIOEnd:
		fclose(fp);
	}
	else
	{
		printf( "Error: Can't open %s\n", InputFile);
		result=ENOENT;
	}

	return result;
}

int main( int argc, char *argv[] )
{
	int result;

	if( argc == 4)
	{
		if( strncmp( argv[1], "-L", 2 ) )
		{
			printf("Error: Option '%s' not recognized\n", argv[1]);
			result=EINVAL;
		}
		else
		{
			result=ConvertFile(argv[2], argv[3], 1);
		}
	}
	else if(argc == 3)
	{
		result=ConvertFile(argv[1], argv[2], 0);
	}
	else
	{
		printf(	"ADPCM Encoder %s\n"
			"Usage: sadenc [-L] <input wave> <output sad>\n"
			"Options:\n"
			"  -L  Loop\n", VERSION);
		result=EINVAL;
	}

	return result;
}

