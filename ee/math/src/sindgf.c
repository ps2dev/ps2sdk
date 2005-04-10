/*							sindgf.c
 *
 *	Circular sine of angle in degrees
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, sindgf();
 *
 * y = sindgf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Range reduction is into intervals of 45 degrees.
 *
 * Two polynomial approximating functions are employed.
 * Between 0 and pi/4 the sine is approximated by
 *		x  +  x**3 P(x**2).
 * Between pi/4 and pi/2 the cosine is represented as
 *		1  -  x**2 Q(x**2).
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain		# trials	  peak		 rms
 *	  IEEE		+-3600		100,000 	 1.2e-7 	3.0e-8
 * 
 * ERROR MESSAGES:
 *
 *	 message		   condition		value returned
 * sin total loss	   x > 2^24 			 0.0
 *
 */

/*							cosdgf.c
 *
 *	Circular cosine of angle in degrees
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, cosdgf();
 *
 * y = cosdgf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Range reduction is into intervals of 45 degrees.
 *
 * Two polynomial approximating functions are employed.
 * Between 0 and pi/4 the cosine is approximated by
 *		1  -  x**2 Q(x**2).
 * Between pi/4 and pi/2 the sine is represented as
 *		x  +  x**3 P(x**2).
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain		# trials	  peak		   rms
 *	  IEEE	  -8192,+8192	100,000 	 3.0e-7 	3.0e-8
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1985, 1987, 1988, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


/* Single precision circular sine
 * test interval: [-pi/4, +pi/4]
 * trials: 10000
 * peak relative error: 6.8e-8
 * rms relative error: 2.6e-8
 */
#include "mconf.h"


/*static float FOPI = 1.27323954473516;*/

extern float PIO4F;

/* These are for a 24-bit significand: */
static float T24M1 = 16777215.;

static float PI180 = 0.0174532925199432957692; /* pi/180 */

#ifdef ANSIC
float sindgf( float xx )
#else
float sindgf(xx)
double xx;
#endif
{
float x, y, z;
int j;
int sign;

sign = 1;
x = xx;
if( xx < 0 )
	{
	sign = -1;
	x = -xx;
	}
if( x > T24M1 )
	{
	mtherr( "sindgf", TLOSS );
	return(0.0);
	}
j = 0.022222222222222222222 * x; /* integer part of x/45 */
y = j;
/* map zeros to origin */
if( j & 1 )
	{
	j += 1;
	y += 1.0;
	}
j &= 7; /* octant modulo 360 degrees */
/* reflect in x axis */
if( j > 3)
	{
	sign = -sign;
	j -= 4;
	}

x = x - y * 45.0;
x *= PI180; /* multiply by pi/180 to convert to radians */

z = x * x;
if( (j==1) || (j==2) )
	{
/*
	y = ((( 2.4462803166E-5 * z
	  - 1.3887580023E-3) * z
	  + 4.1666650433E-2) * z
	  - 4.9999999968E-1) * z
	  + 1.0;
*/

/* measured relative error in +/- pi/4 is 7.8e-8 */
	y = ((	2.443315711809948E-005 * z
	  - 1.388731625493765E-003) * z
	  + 4.166664568298827E-002) * z * z;
	y -= 0.5 * z;
	y += 1.0;
	}
else
	{
/* Theoretical relative error = 3.8e-9 in [-pi/4, +pi/4] */
	y = ((-1.9515295891E-4 * z
		 + 8.3321608736E-3) * z
		 - 1.6666654611E-1) * z * x;
	y += x;
	}

if(sign < 0)
	y = -y;
return( y);
}


/* Single precision circular cosine
 * test interval: [-pi/4, +pi/4]
 * trials: 10000
 * peak relative error: 8.3e-8
 * rms relative error: 2.2e-8
 */

#ifdef ANSIC
float cosdgf( float xx )
#else
float cosdgf(xx)
double xx;
#endif
{
register float x, y, z;
int j, sign;

/* make argument positive */
sign = 1;
x = xx;
if( x < 0 )
	x = -x;

if( x > T24M1 )
	{
	mtherr( "cosdgf", TLOSS );
	return(0.0);
	}

j = 0.02222222222222222222222 * x; /* integer part of x/PIO4 */
y = j;
/* integer and fractional part modulo one octant */
if( j & 1 ) /* map zeros to origin */
	{
	j += 1;
	y += 1.0;
	}
j &= 7;
if( j > 3)
	{
	j -=4;
	sign = -sign;
	}

if( j > 1 )
	sign = -sign;

x = x - y * 45.0; /* x mod 45 degrees */
x *= PI180; /* multiply by pi/180 to convert to radians */

z = x * x;

if( (j==1) || (j==2) )
	{
	y = (((-1.9515295891E-4 * z
		 + 8.3321608736E-3) * z
		 - 1.6666654611E-1) * z * x)
		 + x;
	}
else
	{
	y = ((	2.443315711809948E-005 * z
	  - 1.388731625493765E-003) * z
	  + 4.166664568298827E-002) * z * z;
	y -= 0.5 * z;
	y += 1.0;
	}
if(sign < 0)
	y = -y;
return( y );
}

