/*							incbetf.c
 *
 *	Incomplete beta integral
 *
 *
 * SYNOPSIS:
 *
 * float a, b, x, y, incbetf();
 *
 * y = incbetf( a, b, x );
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
 * Tested at random points (a,b,x) with a and b in the indicated
 * interval and x between 0 and 1.
 *
 * arithmetic   domain     # trials      peak         rms
 * Relative error:
 *    IEEE       0,30       10000       3.7e-5      5.1e-6
 *    IEEE       0,100      10000       1.7e-4      2.5e-5
 * The useful domain for relative error is limited by underflow
 * of the single precision exponential function.
 * Absolute error:
 *    IEEE       0,30      100000       2.2e-5      9.6e-7
 *    IEEE       0,100      10000       6.5e-5      3.7e-6
 *
 * Larger errors may occur for extreme ratios of a and b.
 *
 * ERROR MESSAGES:
 *   message         condition      value returned
 * incbetf domain     x<0, x>1          0.0
 */


/*
Cephes Math Library, Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

#ifdef ANSIC
float lgamf(float), expf(float), logf(float);
static float incbdf(float, float, float);
static float incbcff(float, float, float);
float incbpsf(float, float, float);
#else
float lgamf(), expf(), logf();
float incbpsf();
static float incbcff(), incbdf();
#endif

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

/* BIG = 1/MACHEPF */
#define BIG   16777216.
extern float MACHEPF, MAXLOGF;
#define MINLOGF (-MAXLOGF)

#ifdef ANSIC
float incbetf( float aaa, float bbb, float xxx )
#else
float incbetf( aaa, bbb, xxx )
double aaa, bbb, xxx;
#endif
{
float aa, bb, xx, ans, a, b, t, x, onemx;
int flag;

aa = aaa;
bb = bbb;
xx = xxx;
if( (xx <= 0.0) || ( xx >= 1.0) )
	{
	if( xx == 0.0 )
		return(0.0);
	if( xx == 1.0 )
		return( 1.0 );
	mtherr( "incbetf", DOMAIN );
	return( 0.0 );
	}

onemx = 1.0 - xx;


/* transformation for small aa */

if( aa <= 1.0 )
	{
	ans = incbetf( aa+1.0, bb, xx );
	t = aa*logf(xx) + bb*logf( 1.0-xx )
		+ lgamf(aa+bb) - lgamf(aa+1.0) - lgamf(bb);
	if( t > MINLOGF )
		ans += expf(t);
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
	ans = a*logf(x) + b*logf( onemx )
		+ lgamf(a+b) - lgamf(a+1.0) - lgamf(b);
 	t = incbetf( a+1.0, b, x );
	if( ans > MINLOGF )
		t += expf(ans);
	goto bdone;
	}
*/
/* Choose expansion for optimal convergence */


if( b > 10.0 )
	{
if( fabsf(b*x/a) < 0.3 )
	{
	t = incbpsf( a, b, x );
	goto bdone;
	}
	}

ans = x * (a+b-2.0)/(a-1.0);
if( ans < 1.0 )
	{
	ans = incbcff( a, b, x );
	t = b * logf( t );
	}
else
	{
	ans = incbdf( a, b, x );
	t = (b-1.0) * logf(t);
	}

t += a*logf(x) + lgamf(a+b) - lgamf(a) - lgamf(b);
t += logf( ans/a );

if( t < MINLOGF )
	{
	t = 0.0;
	if( flag == 0 )
		{
		mtherr( "incbetf", UNDERFLOW );
		}
	}
else
	{
	t = expf(t);
	}
bdone:

if( flag )
	t = 1.0 - t;

return( t );
}

/* Continued fraction expansion #1
 * for incomplete beta integral
 */

#ifdef ANSIC
static float incbcff( float aa, float bb, float xx )
#else
static float incbcff( aa, bb, xx )
double aa, bb, xx;
#endif
{
float a, b, x, xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
float k1, k2, k3, k4, k5, k6, k7, k8;
float r, t, ans;
static float big = BIG;
int n;

a = aa;
b = bb;
x = xx;
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
r = 0.0;
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
		t = fabsf( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;

	if( t < MACHEPF )
		goto cdone;

	k1 += 1.0;
	k2 += 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 -= 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabsf(qk) + fabsf(pk)) > big )
		{
		pkm2 *= MACHEPF;
		pkm1 *= MACHEPF;
		qkm2 *= MACHEPF;
		qkm1 *= MACHEPF;
		}
	if( (fabsf(qk) < MACHEPF) || (fabsf(pk) < MACHEPF) )
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

#ifdef ANSIC
static float incbdf( float aa, float bb, float xx )
#else
static float incbdf( aa, bb, xx )
double aa, bb, xx;
#endif
{
float a, b, x, xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
float k1, k2, k3, k4, k5, k6, k7, k8;
float r, t, ans, z;
static float big = BIG;
int n;

a = aa;
b = bb;
x = xx;
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
r = 0.0;
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
		t = fabsf( (ans - r)/r );
		ans = r;
		}
	else
		t = 1.0;

	if( t < MACHEPF )
		goto cdone;

	k1 += 1.0;
	k2 -= 1.0;
	k3 += 2.0;
	k4 += 2.0;
	k5 += 1.0;
	k6 += 1.0;
	k7 += 2.0;
	k8 += 2.0;

	if( (fabsf(qk) + fabsf(pk)) > big )
		{
		pkm2 *= MACHEPF;
		pkm1 *= MACHEPF;
		qkm2 *= MACHEPF;
		qkm1 *= MACHEPF;
		}
	if( (fabsf(qk) < MACHEPF) || (fabsf(pk) < MACHEPF) )
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


/* power series */
#ifdef ANSIC
float incbpsf( float aa, float bb, float xx )
#else
float incbpsf( aa, bb, xx )
double aa, bb, xx;
#endif
{
float a, b, x, t, u, y, s;

a = aa;
b = bb;
x = xx;

y = a * logf(x) + (b-1.0)*logf(1.0-x) - logf(a);
y -= lgamf(a) + lgamf(b);
y += lgamf(a+b);


t = x / (1.0 - x);
s = 0.0;
u = 1.0;
do
	{
	b -= 1.0;
	if( b == 0.0 )
		break;
	a += 1.0;
	u *= t*b/a;
	s += u;
	}
while( fabsf(u) > MACHEPF );

if( y < MINLOGF )
	{
	mtherr( "incbetf", UNDERFLOW );
	s = 0.0;
	}
else
	s = expf(y) * (1.0 + s);
/*printf( "incbpsf: %.4e\n", s );*/
return(s);
}
