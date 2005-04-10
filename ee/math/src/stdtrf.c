/*							stdtrf.c
 *
 *	Student's t distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * float t, stdtrf();
 * short k;
 *
 * y = stdtrf( k, t );
 *
 *
 * DESCRIPTION:
 *
 * Computes the integral from minus infinity to t of the Student
 * t distribution with integer k > 0 degrees of freedom:
 *
 *                                      t
 *                                      -
 *                                     | |
 *              -                      |         2   -(k+1)/2
 *             | ( (k+1)/2 )           |  (     x   )
 *       ----------------------        |  ( 1 + --- )        dx
 *                     -               |  (      k  )
 *       sqrt( k pi ) | ( k/2 )        |
 *                                   | |
 *                                    -
 *                                   -inf.
 * 
 * Relation to incomplete beta integral:
 *
 *        1 - stdtr(k,t) = 0.5 * incbet( k/2, 1/2, z )
 * where
 *        z = k/(k + t**2).
 *
 * For t < -1, this is the method of computation.  For higher t,
 * a direct method is derived from integration by parts.
 * Since the function is symmetric about t=0, the area under the
 * right tail of the density is found by calling the function
 * with -t instead of t.
 * 
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      +/- 100      5000       2.3e-5      2.9e-6
 */


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

extern float PIF, MACHEPF;

#ifdef ANSIC
float sqrtf(float), atanf(float), incbetf(float, float, float);
#else
float sqrtf(), atanf(), incbetf();
#endif



#ifdef ANSIC
float stdtrf( int k, float tt )
#else
float stdtrf( k, tt )
int k;
double tt;
#endif
{
float t, x, rk, z, f, tz, p, xsqk;
int j;

t = tt;
if( k <= 0 )
	{
	mtherr( "stdtrf", DOMAIN );
	return(0.0);
	}

if( t == 0 )
	return( 0.5 );

if( t < -1.0 )
	{
	rk = k;
	z = rk / (rk + t * t);
	p = 0.5 * incbetf( 0.5*rk, 0.5, z );
	return( p );
	}

/*	compute integral from -t to + t */

if( t < 0 )
	x = -t;
else
	x = t;

rk = k;	/* degrees of freedom */
z = 1.0 + ( x * x )/rk;

/* test if k is odd or even */
if( (k & 1) != 0)
	{

	/*	computation for odd k	*/

	xsqk = x/sqrtf(rk);
	p = atanf( xsqk );
	if( k > 1 )
		{
		f = 1.0;
		tz = 1.0;
		j = 3;
		while(  (j<=(k-2)) && ( (tz/f) > MACHEPF )  )
			{
			tz *= (j-1)/( z * j );
			f += tz;
			j += 2;
			}
		p += f * xsqk/z;
		}
	p *= 2.0/PIF;
	}


else
	{

	/*	computation for even k	*/

	f = 1.0;
	tz = 1.0;
	j = 2;

	while(  ( j <= (k-2) ) && ( (tz/f) > MACHEPF )  )
		{
		tz *= (j - 1)/( z * j );
		f += tz;
		j += 2;
		}
	p = f * x/sqrtf(z*rk);
	}

/*	common exit	*/


if( t < 0 )
	p = -p;	/* note destruction of relative accuracy */

	p = 0.5 + 0.5 * p;
return(p);
}
