/*							igamf.c
 *
 *	Incomplete gamma integral
 *
 *
 *
 * SYNOPSIS:
 *
 * float a, x, y, igamf();
 *
 * y = igamf( a, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *                           x
 *                            -
 *                   1       | |  -t  a-1
 *  igam(a,x)  =   -----     |   e   t   dt.
 *                  -      | |
 *                 | (a)    -
 *                           0
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,30        20000       7.8e-6      5.9e-7
 *
 */
/*							igamcf()
 *
 *	Complemented incomplete gamma integral
 *
 *
 *
 * SYNOPSIS:
 *
 * float a, x, y, igamcf();
 *
 * y = igamcf( a, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * The function is defined by
 *
 *
 *  igamc(a,x)   =   1 - igam(a,x)
 *
 *                            inf.
 *                              -
 *                     1       | |  -t  a-1
 *               =   -----     |   e   t   dt.
 *                    -      | |
 *                   | (a)    -
 *                             x
 *
 *
 * In this implementation both arguments must be positive.
 * The integral is evaluated by either a power series or
 * continued fraction expansion, depending on the relative
 * values of a and x.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,30        30000       7.8e-6      5.9e-7
 *
 */

/*
Cephes Math Library Release 2.2: June, 1992
Copyright 1985, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

/* BIG = 1/MACHEPF */
#define BIG   16777216.

extern float MACHEPF, MAXLOGF;

#ifdef ANSIC
float lgamf(float), expf(float), logf(float), igamf(float, float);
#else
float lgamf(), expf(), logf(), igamf();
#endif

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )



#ifdef ANSIC
float igamcf( float aa, float xx )
#else
float igamcf( aa, xx )
double aa, xx;
#endif
{
float a, x, ans, c, yc, ax, y, z;
float pk, pkm1, pkm2, qk, qkm1, qkm2;
float r, t;
static float big = BIG;

a = aa;
x = xx;
if( (x <= 0) || ( a <= 0) )
	return( 1.0 );

if( (x < 1.0) || (x < a) )
	return( 1.0 - igamf(a,x) );

ax = a * logf(x) - x - lgamf(a);
if( ax < -MAXLOGF )
	{
	mtherr( "igamcf", UNDERFLOW );
	return( 0.0 );
	}
ax = expf(ax);

/* continued fraction */
y = 1.0 - a;
z = x + y + 1.0;
c = 0.0;
pkm2 = 1.0;
qkm2 = x;
pkm1 = x + 1.0;
qkm1 = z * x;
ans = pkm1/qkm1;

do
	{
	c += 1.0;
	y += 1.0;
	z += 2.0;
	yc = y * c;
	pk = pkm1 * z  -  pkm2 * yc;
	qk = qkm1 * z  -  qkm2 * yc;
	if( qk != 0 )
		{
		r = pk/qk;
		t = fabsf( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;
	if( fabsf(pk) > big )
		{
		pkm2 *= MACHEPF;
		pkm1 *= MACHEPF;
		qkm2 *= MACHEPF;
		qkm1 *= MACHEPF;
		}
	}
while( t > MACHEPF );

return( ans * ax );
}



/* left tail of incomplete gamma function:
 *
 *          inf.      k
 *   a  -x   -       x
 *  x  e     >   ----------
 *           -     -
 *          k=0   | (a+k+1)
 *
 */

#ifdef ANSIC
float igamf( float aa, float xx )
#else
float igamf( aa, xx )
double aa, xx;
#endif
{
float a, x, ans, ax, c, r;

a = aa;
x = xx;
if( (x <= 0) || ( a <= 0) )
	return( 0.0 );

if( (x > 1.0) && (x > a ) )
	return( 1.0 - igamcf(a,x) );

/* Compute  x**a * exp(-x) / gamma(a)  */
ax = a * logf(x) - x - lgamf(a);
if( ax < -MAXLOGF )
	{
	mtherr( "igamf", UNDERFLOW );
	return( 0.0 );
	}
ax = expf(ax);

/* power series */
r = a;
c = 1.0;
ans = 1.0;

do
	{
	r += 1.0;
	c *= x/r;
	ans += c;
	}
while( c/ans > MACHEPF );

return( ans * ax/a );
}
