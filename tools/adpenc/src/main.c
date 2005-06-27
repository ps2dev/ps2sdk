#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define VERSION	"1.0"

extern void adpcm_encode(FILE* fp, FILE* sad, int offset, int sample_len, int flag_loop);
extern void fputi( int d, FILE *fp );

enum{ monoF = (1 << 0), stereo = (1 << 1), loopF = (1 << 2) } sad_flag;

int main( int argc, char *argv[] )
{
	FILE *fp, *sad;
	char flag_loop;
	int finput, foutput;
	int sample_freq, sample_len;
	char s[4];
	int chunk_data;
	short e;
	char channels;

	if ( argc < 3 ) 
	{
		printf("ADPCM Encoder %s\n", VERSION);
		printf("Usage: sadenc [-L] <input wave> <output sad>\n" );
		printf("Options:\n");
		printf("  -L  Loop\n");
		return( -1 );
	}
            
	if( argc == 4)
	{
		if( strncmp( argv[1], "-L", 2 ) )
		{
			printf("Error: Option '%s' not recognized\n", argv[1]);
			return (-1);
		}
		else
		{	
			finput = 2;
			foutput = 3;
			flag_loop = 1;
		}
	}
	else
	{
		if(argc == 3)
		{
			finput = 1;
			foutput = 2;
			flag_loop = 0;
		}
		else
		{		
			printf("Error: Too many arguments\n");
			return (-1);
		}
	}

	fp = fopen( argv[finput], "rb" );
  
	if ( fp == NULL ) 
	{
		printf( "Error: Can't open %s\n", argv[finput] );
		return( -2 );
	}
	
	fread( s, 1, 4, fp );
    
	if ( strncmp( s, "RIFF", 4 ) ) 
	{	
		printf( "Error: Not a WAVE-file (\"RIFF\" identifier not found)\n" );
		return( -3 );
	}
	
	fseek( fp, 8, SEEK_SET );
	fread( s, 1, 4, fp );
  
	if ( strncmp( s, "WAVE", 4 ) ) 
	{	
		printf( "Error: Not a WAVE-file (\"WAVE\" identifier not found)\n" );
		return( -3 );
	}

	fseek( fp, 8 + 4, SEEK_SET );
	fread( s, 1, 4, fp );
  
	if ( strncmp( s, "fmt", 3 ) ) 
	{
		printf( "Error: Not a WAVE-file (\"fmt\" chunk not found)\n" );
		return( -3 );
	}
    
	fread( &chunk_data, 4, 1, fp ); 
	chunk_data += ftell( fp );
  
	fread( &e, 2, 1, fp );
    
	if ( e != 1 ) 
	{	
		printf( "Error: No PCM data in WAVE-file\n" );
		return( -4 );
  }   

	fread( &e, 2, 1, fp );
    
	if ( (e != 1) && (e != 2) ) 
	{
		printf( "Error: WAVE file must be MONO or STEREO (max 2 channels)\n" );
    return( -5 );
	}

	channels = e;

	fread( &sample_freq, 4, 1, fp );
  fseek( fp, 4 + 2, SEEK_CUR );

  fread( &e, 2, 1, fp );
  
	if ( e != 16 ) 
	{
		printf( "Error: WAVE-file must 16 bit\n" );
		return( -6 );
	}       
        
	fseek( fp, chunk_data, SEEK_SET );
	fread( s, 1, 4, fp );
    
	// Skip 'fact' and possibly other chunks
	while(strncmp( s, "data", 4 ))
	{	
		fread( &chunk_data, 4, 1, fp );
		chunk_data += ftell( fp );
		fseek( fp, chunk_data, SEEK_SET );
		fread( s, 1, 4, fp );
	}
	
	fread( &sample_len, 4, 1, fp );

	sample_len /= (channels+1);

  sad = fopen( argv[foutput], "wb" );
    
	if ( sad == NULL ) 
	{	
		printf( "Error: Can't write output file %s\n", argv[foutput] );
		return( -8 );
	}

	/*
	header: (16 bytes)
	u32 id "APCM"
	u8	version;	
	u8	channels;
	u8	loop;	
	u8	reserved;
	u32 pitch;
	u32 reserved
	*/

	fprintf( sad, "APCM" );		// ID
	fputc( 1, sad ); // version
	fputc( channels, sad);		// channels		
	fputc( flag_loop, sad);		// loop flag
	fputc( 0, sad);
	fputi((sample_freq*4096)/48000, sad );  // pitch, to encode for PS1 change 48000 to 44100
	fputi( 0, sad); // resverd
		

	if(channels == 1)
	{
		adpcm_encode(fp, sad, 0, sample_len, flag_loop);
	}
	else
	{
		int data_offset = ftell(fp);
		
		// Encode left
		adpcm_encode(fp, sad, 2, sample_len, flag_loop);
		
		fseek(fp, SEEK_SET, data_offset+2);
		// Encode right
		adpcm_encode(fp, sad, 2, sample_len, flag_loop);
	}

	fclose(fp);
	fclose(sad);

	return 0;
}

