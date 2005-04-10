/*							tandgf.c
 *
 *	Circular tangent of angle in degrees
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, tandgf();
 *
 * y = tandgf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the circular tangent of the radian argument x.
 *
 * Range reduction is into intervals of 45 degrees.
 *
 *
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain	   # trials 	 peak		  rms
 *	  IEEE	   +-2^24		50000		2.4e-7		4.8e-8
 *
 * ERROR MESSAGES:
 *
 *	 message		 condition			value returned
 * tanf total loss	 x > 2^24			   0.0
 *
 */
/* 						cotdgf.c
 *
 *	Circular cotangent of angle in degrees
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, cotdgf();
 *
 * y = cotdgf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Range reduction is into intervals of 45 degrees.
 * A common routine computes either the tangent or cotangent.
 *
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain	   # trials 	 peak		  rms
 *	  IEEE	   +-2^24		50000		2.4e-7		4.8e-8
 *
 *
 * ERROR MESSAGES:
 *
 *	 message		 condition			value returned
 * cot total loss	x > 2^24				0.0
 * cot singularity	x = 0				   MAXNUMF
 *
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1989, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* Single precision circular tangent
 * test interval: [-pi/4, +pi/4]
 * trials: 10000
 * peak relative error: 8.7e-8
 * rms relative error: 2.8e-8
 */
#include "mconf.h"

extern float MAXNUMF;

static float T24M1 = 16777215.;
static float PI180 = 0.0174532925199432957692; /* pi/180 */

#ifdef ANSIC
static float tancotf( float xx, int cotflg )
#else
static float tancotf(xx,cotflg)
double xx;
int cotflg;
#endif
{
float x, y, z, zz;
int j;
int sign;


/* make argument positive but save the sign */
if( xx < 0.0 )
	{
	x = -xx;
	sign = -1;
	}
else
	{
	x = xx;
	sign = 1;
	}

if( x > T24M1 )
	{
	if( cotflg )
		mtherr( "cotdgf", TLOSS );
	else
		mtherr( "tandgf", TLOSS );
	return(0.0);
	}

/* compute x mod PIO4 */
j = 0.022222222222222222222 * x; /* integer part of x/45 */
y = j;

/* map zeros and singularities to origin */
if( j & 1 )
	{
	j += 1;
	y += 1.0;
	}

z = x - y * 45.0;
z *= PI180; /* multiply by pi/180 to convert to radians */

zz = z * z;

if( x > 1.0e-4 )
	{
/* 1.7e-8 relative error in [-pi/4, +pi/4] */
	y =
	((((( 9.38540185543E-3 * zz
	+ 3.11992232697E-3) * zz
	+ 2.44301354525E-2) * zz
	+ 5.34112807005E-2) * zz
	+ 1.33387994085E-1) * zz
	+ 3.33331568548E-1) * zz * z
	+ z;
	}
else
	{
	y = z;
	}

if( j & 2 )
	{
	if( cotflg )
		y = -y;
	else
		{
		if( y != 0.0 )
			{
			y = -1.0/y;
			}
		else
			{
			mtherr( "tandgf", SING );
			y = MAXNUMF;
			}
		}
	}
else
	{
	if( cotflg )
		{
		if( y != 0.0 )
			y = 1.0/y;
		else
			{
			mtherr( "cotdgf", SING );
			y = MAXNUMF;
			}
		}
	}

if( sign < 0 )
	y = -y;

return( y );
}


#ifdef ANSIC
float tandgf( float x )
#else
float tandgf(x)
double x;
#endif
{

return( tancotf(x,0) );
}

#ifdef ANSIC
float cotdgf( float x )
#else
float cotdgf(x)
double x;
#endif
{

if( x == 0.0 )
	{
	mtherr( "cotdgf", SING );
	return( MAXNUMF );
	}
return( tancotf(x,1) );
}

