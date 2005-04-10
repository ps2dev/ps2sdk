/*						rgammaf.c
 *
 *	Reciprocal gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, rgammaf();
 *
 * y = rgammaf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns one divided by the gamma function of the argument.
 *
 * The function is approximated by a Chebyshev expansion in
 * the interval [0,1].  Range reduction is by recurrence
 * for arguments between -34.034 and +34.84425627277176174.
 * 1/MAXNUMF is returned for positive arguments outside this
 * range.
 *
 * The reciprocal gamma function has no singularities,
 * but overflow and underflow may occur for large arguments.
 * These conditions return either MAXNUMF or 1/MAXNUMF with
 * appropriate sign.
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE     -34,+34      100000      8.9e-7      1.1e-7
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1985, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

/* Chebyshev coefficients for reciprocal gamma function
 * in interval 0 to 1.  Function is 1/(x gamma(x)) - 1
 */

static float R[] = {
 1.08965386454418662084E-9,
-3.33964630686836942556E-8,
 2.68975996440595483619E-7,
 2.96001177518801696639E-6,
-8.04814124978471142852E-5,
 4.16609138709688864714E-4,
 5.06579864028608725080E-3,
-6.41925436109158228810E-2,
-4.98558728684003594785E-3,
 1.27546015610523951063E-1
};


static char name[] = "rgammaf";

extern float PIF, MAXLOGF, MAXNUMF;



#ifdef ANSIC
float chbevlf(float, float *, int);
float expf(float), logf(float), sinf(float), lgamf(float);

float rgammaf(float xx)
#else
float chbevlf(), expf(), logf(), sinf(), lgamf();

float rgammaf(xx)
double xx;
#endif
{
float x, w, y, z;
int sign;

x = xx;
if( x > 34.84425627277176174)
	{
	mtherr( name, UNDERFLOW );
	return(1.0/MAXNUMF);
	}
if( x < -34.034 )
	{
	w = -x;
	z = sinf( PIF*w );
	if( z == 0.0 )
		return(0.0);
	if( z < 0.0 )
		{
		sign = 1;
		z = -z;
		}
	else
		sign = -1;

	y = logf( w * z / PIF ) + lgamf(w);
	if( y < -MAXLOGF )
		{
		mtherr( name, UNDERFLOW );
		return( sign * 1.0 / MAXNUMF );
		}
	if( y > MAXLOGF )
		{
		mtherr( name, OVERFLOW );
		return( sign * MAXNUMF );
		}
	return( sign * expf(y));
	}
z = 1.0;
w = x;

while( w > 1.0 )	/* Downward recurrence */
	{
	w -= 1.0;
	z *= w;
	}
while( w < 0.0 )	/* Upward recurrence */
	{
	z /= w;
	w += 1.0;
	}
if( w == 0.0 )		/* Nonpositive integer */
	return(0.0);
if( w == 1.0 )		/* Other integer */
	return( 1.0/z );

y = w * ( 1.0 + chbevlf( 4.0*w-2.0, R, 10 ) ) / z;
return(y);
}
