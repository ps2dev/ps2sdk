/*							incbet.c
 *
 *	Incomplete beta integral
 *
 *
 * SYNOPSIS:
 *
 * double a, b, x, y, incbet();
 *
 * y = incbet( a, b, x );
 *
 *
 * DESCRIPTION:
 *
 * Returns incomplete beta integral of the arguments, evaluated
 * from zero to x.  The function is defined as
 *
 *                  x
 *     -            -
 *    | (a+b)      | |  a-1     b-1
 *  -----------    |   t   (1-t)   dt.
 *   -     -     | |
 *  | (a) | (b)   -
 *                 0
 *
 * The domain of definition is 0 <= x <= 1.  In this
 * implementation a and b are restricted to positive values.
 * The integral from x to 1 may be obtained by the symmetry
 * relation
 *
 *    1 - incbet( a, b, x )  =  incbet( b, a, 1-x ).
 *
 * The integral is evaluated by a continued fraction expansion.
 * If a < 1, the function calls itself recursively after a
 * transformation to increase a to a+1.
 *
 * ACCURACY:
 *
 * Tested at random points (a,b,x) with a and b between 0
 * and 100 and x between 0 and 1.
 *          Relative error (x ranges from 0 to 1):
 * arithmetic   domain     # trials      peak         rms
 *    DEC       0,100        3300       3.5e-14     5.0e-15
 *    IEEE      0,100       10000       3.9e-13     5.2e-14
 *
 * Larger errors may occur for extreme ratios of a and b.
 *
 * ERROR MESSAGES:
 *   message         condition      value returned
 * incbet domain      x<0, x>1          0.0
 */


/*
Cephes Math Library, Release 2.0:  April, 1987
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

#define BIG  1.44115188075855872E+17
extern double MACHEP, MINLOG, MAXLOG;

double incbet( aa, bb, xx )
double aa, bb, xx;
{
double ans, a, b, t, x, onemx;
double lgam(), exp(), log(), fabs();
double incbd(), incbcf();
short flag;

if( (xx <= 0.0) || ( xx >= 1.0) )
	{
	if( xx == 0.0 )
		return(0.0);
	if( xx == 1.0 )
		return( 1.0 );
	mtherr( "incbet", DOMAIN );
	return( 0.0 );
	}

onemx = 1.0 - xx;

/* transformation for small aa */

if( aa <= 1.0 )
	{
	ans = incbet( aa+1.0, bb, xx );
	t = aa*log(xx) + bb*log( 1.0-xx )
		+ lgam(aa+bb) - lgam(aa+1.0) - lgam(bb);
	if( t > MINLOG )
		ans += exp(t);
	return( ans );
	}

/* see if x is greater than the mean */

if( xx > (aa/(aa+bb)) )
	{
	flag = 1;
	a = bb;
	b = aa;
	t = xx;
	x = onemx;
	}
else
	{
	flag = 0;
	a = aa;
	b = bb;
	t = onemx;
	x = xx;
	}

/* transformation for small aa */
/*
if( a <= 1.0 )
	{
 	t = incbet( a+1.0, b, x );

	ans = a*log(x) + b*log( 1.0-x )
		+ lgam(a+b) - lgam(a+1.0) - lgam(b);
	if( ans > MINLOG )
		t += exp(ans);
	goto bdone;
	}
*/
/* Choose expansion for optimal convergence */

ans = x * (a+b-2.0)/(a-1.0);
if( ans < 1.0 )
	{
	ans = incbcf( a, b, x );
	t = b * log( t );
	}
else
	{
	ans = incbd( a, b, x );
	t = (b-1.0) * log(t);
	}

adone:
t += a*log(x) + lgam(a+b) - lgam(a) - lgam(b);
t += log( ans/a );

if( t < MINLOG )
	{
	if( flag == 0 )
		{
		mtherr( "incbet", UNDERFLOW );
		return( 0.0 );
		}
	else
		return(1.0);
	}

t = exp(t);

bdone:

if( flag == 1 )
	t = 1.0 - t;

return( t );
}

/* Continued fraction expansion #1
 * for incomplete beta integral
 */

static double incbcf( a, b, x )
double a, b, x;
{
double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
double k1, k2, k3, k4, k5, k6, k7, k8;
double r, t, ans;
static double big = BIG;
double fabs();
int n;

k1 = a;
k2 = a + b;
k3 = a;
k4 = a + 1.0;
k5 = 1.0;
k6 = b - 1.0;
k7 = k4;
k8 = a + 2.0;

pkm2 = 0.0;
qkm2 = 1.0;
pkm1 = 1.0;
qkm1 = 1.0;
ans = 1.0;

n = 0;
do
	{
	
	xk = -( x * k1 * k2 )/( k3 * k4 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	xk = ( x * k5 * k6 )/( k7 * k8 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	if( qk != 0 )
		r = pk/qk;
	if( r != 0 )
		{
		t = fabs( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;

	if( t < MACHEP )
		goto cdone;

	k1 += 1.0;
	k2 += 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 -= 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabs(qk) + fabs(pk)) > big )
		{
		pkm2 /= big;
		pkm1 /= big;
		qkm2 /= big;
		qkm1 /= big;
		}
	if( (fabs(qk) < MACHEP) || (fabs(pk) < MACHEP) )
		{
		pkm2 *= big;
		pkm1 *= big;
		qkm2 *= big;
		qkm1 *= big;
		}
	}
while( ++n < 100 );

cdone:
return(ans);
}


/* Continued fraction expansion #2
 * for incomplete beta integral
 */

static double incbd( a, b, x )
double a, b, x;
{
double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
double k1, k2, k3, k4, k5, k6, k7, k8;
double r, t, ans, z;
static double big = BIG;
double fabs();
int n;

k1 = a;
k2 = b - 1.0;
k3 = a;
k4 = a + 1.0;
k5 = 1.0;
k6 = a + b;
k7 = a + 1.0;;
k8 = a + 2.0;

pkm2 = 0.0;
qkm2 = 1.0;
pkm1 = 1.0;
qkm1 = 1.0;
z = x / (1.0-x);
ans = 1.0;

n = 0;
do
	{
	
	xk = -( z * k1 * k2 )/( k3 * k4 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	xk = ( z * k5 * k6 )/( k7 * k8 );
	pk = pkm1 +  pkm2 * xk;
	qk = qkm1 +  qkm2 * xk;
	pkm2 = pkm1;
	pkm1 = pk;
	qkm2 = qkm1;
	qkm1 = qk;

	if( qk != 0 )
		r = pk/qk;
	if( r != 0 )
		{
		t = fabs( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;

	if( t < MACHEP )
		goto cdone;

	k1 += 1.0;
	k2 -= 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 += 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabs(qk) + fabs(pk)) > big )
		{
		pkm2 /= big;
		pkm1 /= big;
		qkm2 /= big;
		qkm1 /= big;
		}
	if( (fabs(qk) < MACHEP) || (fabs(pk) < MACHEP) )
		{
		pkm2 *= big;
		pkm1 *= big;
		qkm2 *= big;
		qkm1 *= big;
		}
	}
while( ++n < 100 );

cdone:
return(ans);
}
