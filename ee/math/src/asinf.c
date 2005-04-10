/*							asinf.c
 *
 *	Inverse circular sine
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, asinf();
 *
 * y = asinf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns radian angle between -pi/2 and +pi/2 whose sine is x.
 *
 * A polynomial of the form x + x**3 P(x**2)
 * is used for |x| in the interval [0, 0.5].  If |x| > 0.5 it is
 * transformed by the identity
 *
 *    asin(x) = pi/2 - 2 asin( sqrt( (1-x)/2 ) ).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     -1, 1       100000       2.5e-7       5.0e-8
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * asinf domain        |x| > 1           0.0
 *
 */
/*							acosf()
 *
 *	Inverse circular cosine
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, acosf();
 *
 * y = acosf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns radian angle between -pi/2 and +pi/2 whose cosine
 * is x.
 *
 * Analytically, acos(x) = pi/2 - asin(x).  However if |x| is
 * near 1, there is cancellation error in subtracting asin(x)
 * from pi/2.  Hence if x < -0.5,
 *
 *    acos(x) =	 pi - 2.0 * asin( sqrt((1+x)/2) );
 *
 * or if x > +0.5,
 *
 *    acos(x) =	 2.0 * asin(  sqrt((1-x)/2) ).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -1, 1      100000       1.4e-7      4.2e-8
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * acosf domain        |x| > 1           0.0
 */

/*							asin.c	*/

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* Single precision circular arcsine
 * test interval: [-0.5, +0.5]
 * trials: 10000
 * peak relative error: 6.7e-8
 * rms relative error: 2.5e-8
 */
#include "mconf.h"
extern float PIF, PIO2F;

#ifdef ANSIC
float sqrtf( float );

float asinf( float xx )
#else
float sqrtf();

float asinf(xx)
double xx;
#endif
{
float a, x, z;
int sign, flag;

x = xx;

if( x > 0 )
	{
	sign = 1;
	a = x;
	}
else
	{
	sign = -1;
	a = -x;
	}

if( a > 1.0 )
	{
	mtherr( "asinf", DOMAIN );
	return( 0.0 );
	}

if( a < 1.0e-4 )
	{
	z = a;
	goto done;
	}

if( a > 0.5 )
	{
	z = 0.5 * (1.0 - a);
	x = sqrtf( z );
	flag = 1;
	}
else
	{
	x = a;
	z = x * x;
	flag = 0;
	}

z =
(((( 4.2163199048E-2 * z
  + 2.4181311049E-2) * z
  + 4.5470025998E-2) * z
  + 7.4953002686E-2) * z
  + 1.6666752422E-1) * z * x
  + x;

if( flag != 0 )
	{
	z = z + z;
	z = PIO2F - z;
	}
done:
if( sign < 0 )
	z = -z;
return( z );
}




#ifdef ANSIC
float acosf( float x )
#else
float acosf(x)
double x;
#endif
{

if( x < -1.0 )
	goto domerr;

if( x < -0.5) 
	return( PIF - 2.0 * asinf( sqrtf(0.5*(1.0+x)) ) );

if( x > 1.0 )
	{
domerr:	mtherr( "acosf", DOMAIN );
	return( 0.0 );
	}

if( x > 0.5 )
	return( 2.0 * asinf(  sqrtf(0.5*(1.0-x) ) ) );

return( PIO2F - asinf(x) );
}

