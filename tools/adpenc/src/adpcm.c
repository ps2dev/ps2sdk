/*

    PSX VAG-Packer, hacked by bITmASTER@bigfoot.com
    v0.1
	
	* Support for looping, better support for wave files,
		support for stereo samples and some other minor 
		changes and fixes.
	  Lukasz Bruun (mail@lukasz.dk) 2005


VAG Info by bITmASTER:

VAG-Sample
One sample is 16 bits long.

typedef struct {
	unsigned char pack_info;
	unsigned char flags;
	unsigned char packed[14];
}

pack_info:
high nibble: Predictor-Number
low nibble : Shiftfactor

flags:
0x07: Sample-End (Last sample)
0x02: Sample belongs to Repeat-Part

0x06: Repeat-Start (First sample)
0x04: Repeat-Point
0x03: Loop Sample-End, begin playing Repeat-Start (2nd last sample)
0x01: Sample-End (2nd last sample)

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define BUFFER_SIZE 128*28

short wave[BUFFER_SIZE];

void find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor );
void pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor );
void fputi( int d, FILE *fp );


void adpcm_encode(FILE* fp, FILE* sad, int offset, int sample_len, int flag_loop)
{
	short *ptr;
	double d_samples[28];
	short four_bit[28];
	int predict_nr;
	int shift_factor;
	int flags;
	int i, j, k;    
	unsigned char d;
	int size;
    
	flags = 0;  
	
	// sample_len is number of 16 bit samples
  while( sample_len > 0 ) 
	{		
		// Size is sample_len
		size = ( sample_len >= BUFFER_SIZE ) ? BUFFER_SIZE : sample_len; 
    
		if(offset)
		{
			for(i = 0; i < size; i++)
			{
				fread( wave+i, sizeof( short ), 1, fp );
				fseek(fp, offset, SEEK_CUR);
			}
		}  
		else
		{
			fread(wave, sizeof( short ), size, fp);
		}	   

		// i = num of samples with size 28
		i = size / 28;

		// Add blanks
		if ( size % 28 ) 
		{
			for ( j = size % 28; j < 28; j++ ) wave[28*i+j] = 0;
			i++;
		}
      
		// pack 28 samples
		for ( j = 0; j < i; j++ ) 
		{                                     
			ptr = wave + j * 28;
		
			find_predict( ptr, d_samples, &predict_nr, &shift_factor );
        
			// pack samples
			pack( d_samples, four_bit, predict_nr, shift_factor );
        
			// correctly format predict_nr and shift_factor and write to file
			d = ( predict_nr << 4 ) | shift_factor;
      fputc( d, sad );
			
			if(flag_loop == 1)
			{
				fputc( 6, sad );; // loop value
				flag_loop = 2;
			}
			else
			{
				fputc( flags, sad );
			}
				
			// Write the 28 samples to file
			for ( k = 0; k < 28; k += 2 ) 
			{
				d = ( ( four_bit[k+1] >> 8 ) & 0xf0 ) | ( ( four_bit[k] >> 12 ) & 0xf );
				fputc( d, sad );
			}
			
			// Decrease sample_len by 28 samples
			sample_len -= 28;
           
			if ( sample_len < 28 )
			{
				if(flag_loop == 2)
					flags = 3;
				else
					flags = 1;
			}
		}
	}
    
  fputc( ( predict_nr << 4 ) | shift_factor, sad );
  fputc( 7, sad );            // end flag
  
	for ( i = 0; i < 14; i++ )
			fputc( 0, sad ); 
}


static double f[5][2] = { { 0.0, 0.0 },
                            {  -60.0 / 64.0, 0.0 },
                            { -115.0 / 64.0, 52.0 / 64.0 },
                            {  -98.0 / 64.0, 55.0 / 64.0 },
                            { -122.0 / 64.0, 60.0 / 64.0 } };
                  


void find_predict( short *samples, double *d_samples, int *predict_nr, int *shift_factor )
{
    int i, j;
    double buffer[28][5];
    double min = 1e10;
    double max[5];
    double ds;
    int min2;
    int shift_mask;
    static double _s_1 = 0.0;                            // s[t-1]
    static double _s_2 = 0.0;                            // s[t-2]
    double s_0, s_1, s_2;

    for ( i = 0; i < 5; i++ ) {
        max[i] = 0.0;
        s_1 = _s_1;
        s_2 = _s_2;
        for ( j = 0; j < 28; j ++ ) {
            s_0 = (double) samples[j];                      // s[t-0]
            if ( s_0 > 30719.0 )
                s_0 = 30719.0;
            if ( s_0 < - 30720.0 )
                s_0 = -30720.0;
            ds = s_0 + s_1 * f[i][0] + s_2 * f[i][1];
            buffer[j][i] = ds;
            if ( fabs( ds ) > max[i] )
                max[i] = fabs( ds );
//                printf( "%+5.2f\n", s2 );
                s_2 = s_1;                                  // new s[t-2]
                s_1 = s_0;                                  // new s[t-1]
        }
        
        if ( max[i] < min ) {
            min = max[i];
            *predict_nr = i;
        }
        if ( min <= 7 ) {
            *predict_nr = 0;
            break;
        }
        
    }

// store s[t-2] and s[t-1] in a static variable
// these than used in the next function call

    _s_1 = s_1;
    _s_2 = s_2;
    
    for ( i = 0; i < 28; i++ )
        d_samples[i] = buffer[i][*predict_nr];

//  if ( min > 32767.0 )
//      min = 32767.0;
        
    min2 = ( int ) min;
    shift_mask = 0x4000;
    *shift_factor = 0;
    
    while( *shift_factor < 12 ) {
        if ( shift_mask  & ( min2 + ( shift_mask >> 3 ) ) )
            break;
        (*shift_factor)++;
        shift_mask = shift_mask >> 1;
    }
      
}

void pack( double *d_samples, short *four_bit, int predict_nr, int shift_factor )
{
    double ds;
    int di;
    double s_0;
    static double s_1 = 0.0;
    static double s_2 = 0.0;
    int i;

    for ( i = 0; i < 28; i++ ) {
        s_0 = d_samples[i] + s_1 * f[predict_nr][0] + s_2 * f[predict_nr][1];
        ds = s_0 * (double) ( 1 << shift_factor );

        di = ( (int) ds + 0x800 ) & 0xfffff000;

        if ( di > 32767 )
            di = 32767;
        if ( di < -32768 )
            di = -32768;
            
        four_bit[i] = (short) di;

        di = di >> shift_factor;
        s_2 = s_1;
        s_1 = (double) di - s_0;

    }
}

void fputi( int d, FILE *fp )
{
	fputc( d,       fp );
	fputc( d >> 8,  fp );
	fputc( d >> 16, fp );   
	fputc( d >> 24, fp );
}
