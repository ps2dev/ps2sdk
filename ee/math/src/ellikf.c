/*							ellikf.c
 *
 *	Incomplete elliptic integral of the first kind
 *
 *
 *
 * SYNOPSIS:
 *
 * float phi, m, y, ellikf();
 *
 * y = ellikf( phi, m );
 *
 *
 *
 * DESCRIPTION:
 *
 * Approximates the integral
 *
 *
 *
 *                phi
 *                 -
 *                | |
 *                |           dt
 * F(phi\m)  =    |    ------------------
 *                |                   2
 *              | |    sqrt( 1 - m sin t )
 *               -
 *                0
 *
 * of amplitude phi and modulus m, using the arithmetic -
 * geometric mean algorithm.
 *
 *
 *
 *
 * ACCURACY:
 *
 * Tested at random points with phi in [0, 2] and m in
 * [0, 1].
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,2         10000       2.9e-7      5.8e-8
 *
 *
 */


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/*	Incomplete elliptic integral of first kind	*/

#include "mconf.h"
extern float PIF, PIO2F, MACHEPF;

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#ifdef ANSIC
float sqrtf(float), logf(float), sinf(float), tanf(float), atanf(float);
#else
float sqrtf(), logf(), sinf(), tanf(), atanf();
#endif


#ifdef ANSIC
float ellikf( float phia, float ma )
#else
float ellikf( phia, ma )
double phia, ma;
#endif
{
float phi, m, a, b, c, temp;
float t;
int d, mod, sign;

phi = phia;
m = ma;
if( m == 0.0 )
	return( phi );
if( phi < 0.0 )
	{
	phi = -phi;
	sign = -1;
	}
else
	sign = 0;
a = 1.0;
b = 1.0 - m;
if( b == 0.0 )
	return(  logf(  tanf( 0.5*(PIO2F + phi) )  )   );
b = sqrtf(b);
c = sqrtf(m);
d = 1;
t = tanf( phi );
mod = (phi + PIO2F)/PIF;

while( fabsf(c/a) > MACHEPF )
	{
	temp = b/a;
	phi = phi + atanf(t*temp) + mod * PIF;
	mod = (phi + PIO2F)/PIF;
	t = t * ( 1.0 + temp )/( 1.0 - temp * t * t );
	c = ( a - b )/2.0;
	temp = sqrtf( a * b );
	a = ( a + b )/2.0;
	b = temp;
	d += d;
	}

temp = (atanf(t) + mod * PIF)/(d * a);
if( sign < 0 )
	temp = -temp;
return( temp );
}
