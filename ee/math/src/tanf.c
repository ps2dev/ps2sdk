/*							tanf.c
 *
 *	Circular tangent
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, tanf();
 *
 * y = tanf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the circular tangent of the radian argument x.
 *
 * Range reduction is modulo pi/4.	A polynomial approximation
 * is employed in the basic interval [0, pi/4].
 *
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain	   # trials 	 peak		  rms
 *	  IEEE	   +-4096		 100000 	3.3e-7		4.5e-8
 *
 * ERROR MESSAGES:
 *
 *	 message		 condition			value returned
 * tanf total loss	 x > 2^24			   0.0
 *
 */
/* 						cotf.c
 *
 *	Circular cotangent
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, cotf();
 *
 * y = cotf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the circular cotangent of the radian argument x.
 * A common routine computes either the tangent or cotangent.
 *
 *
 *
 * ACCURACY:
 *
 *						Relative error:
 * arithmetic	domain	   # trials 	 peak		  rms
 *	  IEEE	   +-4096		 100000 	3.0e-7		4.5e-8
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
Copyright 1984, 1987, 1989 by Stephen L. Moshier
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

static float DP1 = 0.78515625;
static float DP2 = 2.4187564849853515625e-4;
static float DP3 = 3.77489497744594108e-8;
float FOPI = 1.27323954473516;	/* 4/pi */
static float lossth = 8192.;
/*static float T24M1 = 16777215.;*/


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

if( x > lossth )
	{
	if( cotflg )
		mtherr( "cotf", TLOSS );
	else
		mtherr( "tanf", TLOSS );
	return(0.0);
	}

/* compute x mod PIO4 */
j = FOPI * x; /* integer part of x/(PI/4) */
y = j;

/* map zeros and singularities to origin */
if( j & 1 )
	{
	j += 1;
	y += 1.0;
	}

z = ((x - y * DP1) - y * DP2) - y * DP3;

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
		y = -1.0/y;
	}
else
	{
	if( cotflg )
		y = 1.0/y;
	}

if( sign < 0 )
	y = -y;

return( y );
}


#ifdef ANSIC
float tanf( float x )
#else
float tanf(x)
double x;
#endif
{

return( tancotf(x,0) );
}

#ifdef ANSIC
float cotf( float x )
#else
float cotf(x)
double x;
#endif
{

if( x == 0.0 )
	{
	mtherr( "cotf", SING );
	return( MAXNUMF );
	}
return( tancotf(x,1) );
}

