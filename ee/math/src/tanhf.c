/*							tanhf.c
 *
 *	Hyperbolic tangent
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, tanhf();
 *
 * y = tanhf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns hyperbolic tangent of argument in the range MINLOG to
 * MAXLOG.
 *
 * A polynomial approximation is used for |x| < 0.625.
 * Otherwise,
 *
 *    tanh(x) = sinh(x)/cosh(x) = 1  -  2/(exp(2x) + 1).
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -2,2        100000      1.3e-7      2.6e-8
 *
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1989, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* Single precision hyperbolic tangent
 * test interval: [-0.625, +0.625]
 * trials: 10000
 * peak relative error: 7.2e-8
 * rms relative error: 2.6e-8
 */
#include "mconf.h"

extern float MAXLOGF;

#ifdef ANSIC
float expf( float );

float tanhf( float xx )
#else
float expf();

float tanhf(xx)
double xx;
#endif
{
float x, z;

if( xx < 0 )
	x = -xx;
else
	x = xx;

if( x > 0.5 * MAXLOGF )
	{
	if( xx > 0 )
		return( 1.0 );
	else
		return( -1.0 );
	}
if( x >= 0.625 )
	{
	x = expf(x+x);
	z =  1.0  - 2.0/(x + 1.0);
	if( xx < 0 )
		z = -z;
	}
else
	{
	z = x * x;
	z =
	(((( -5.70498872745E-3 * z
	  + 2.06390887954E-2) * z
	  - 5.37397155531E-2) * z
	  + 1.33314422036E-1) * z
	  - 3.33332819422E-1) * z * xx
	  + xx;
	}
return( z );
}
