/*							expnf.c
 *
 *		Exponential integral En
 *
 *
 *
 * SYNOPSIS:
 *
 * int n;
 * float x, y, expnf();
 *
 * y = expnf( n, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Evaluates the exponential integral
 *
 *                 inf.
 *                   -
 *                  | |   -xt
 *                  |    e
 *      E (x)  =    |    ----  dt.
 *       n          |      n
 *                | |     t
 *                 -
 *                  1
 *
 *
 * Both n and x must be nonnegative.
 *
 * The routine employs either a power series, a continued
 * fraction, or an asymptotic formula depending on the
 * relative values of n and x.
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0, 30       10000       5.6e-7      1.2e-7
 *
 */

/*							expn.c	*/

/* Cephes Math Library Release 2.2:  July, 1992
 * Copyright 1985, 1992 by Stephen L. Moshier
 * Direct inquiries to 30 Frost Street, Cambridge, MA 02140 */

#include "mconf.h"

#define EUL 0.57721566490153286060
#define BIG   16777216.
extern float MAXNUMF, MACHEPF, MAXLOGF;
#ifdef ANSIC
float powf(float, float), gammaf(float), logf(float), expf(float);
#else
float powf(), gammaf(), logf(), expf();
#endif
#define fabsf(x) ( (x) < 0 ? -(x) : (x) )


#ifdef ANSIC
float expnf( int n, float xx )
#else
float expnf( n, xx )
int n;
double xx;
#endif
{
float x, ans, r, t, yk, xk;
float pk, pkm1, pkm2, qk, qkm1, qkm2;
float psi, z;
int i, k;
static float big = BIG;


x = xx;
if( n < 0 )
	goto domerr;

if( x < 0 )
	{
domerr:	mtherr( "expnf", DOMAIN );
	return( MAXNUMF );
	}

if( x > MAXLOGF )
	return( 0.0 );

if( x == 0.0 )
	{
	if( n < 2 )
		{
		mtherr( "expnf", SING );
		return( MAXNUMF );
		}
	else
		return( 1.0/(n-1.0) );
	}

if( n == 0 )
	return( expf(-x)/x );

/*							expn.c	*/
/*		Expansion for large n		*/

if( n > 5000 )
	{
	xk = x + n;
	yk = 1.0 / (xk * xk);
	t = n;
	ans = yk * t * (6.0 * x * x  -  8.0 * t * x  +  t * t);
	ans = yk * (ans + t * (t  -  2.0 * x));
	ans = yk * (ans + t);
	ans = (ans + 1.0) * expf( -x ) / xk;
	goto done;
	}

if( x > 1.0 )
	goto cfrac;

/*							expn.c	*/

/*		Power series expansion		*/

psi = -EUL - logf(x);
for( i=1; i<n; i++ )
	psi = psi + 1.0/i;

z = -x;
xk = 0.0;
yk = 1.0;
pk = 1.0 - n;
if( n == 1 )
	ans = 0.0;
else
	ans = 1.0/pk;
do
	{
	xk += 1.0;
	yk *= z/xk;
	pk += 1.0;
	if( pk != 0.0 )
		{
		ans += yk/pk;
		}
	if( ans != 0.0 )
		t = fabsf(yk/ans);
	else
		t = 1.0;
	}
while( t > MACHEPF );
k = xk;
t = n;
r = n - 1;
ans = (powf(z, r) * psi / gammaf(t)) - ans;
goto done;

/*							expn.c	*/
/*		continued fraction		*/
cfrac:
k = 1;
pkm2 = 1.0;
qkm2 = x;
pkm1 = 1.0;
qkm1 = x + n;
ans = pkm1/qkm1;

do
	{
	k += 1;
	if( k & 1 )
		{
		yk = 1.0;
		xk = n + (k-1)/2;
		}
	else
		{
		yk = x;
		xk = k/2;
		}
	pk = pkm1 * yk  +  pkm2 * xk;
	qk = qkm1 * yk  +  qkm2 * xk;
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

ans *= expf( -x );

done:
return( ans );
}

