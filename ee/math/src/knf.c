/*							knf.c
 *
 *	Modified Bessel function, third kind, integer order
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, knf();
 * int n;
 *
 * y = knf( n, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns modified Bessel function of the third kind
 * of order n of the argument.
 *
 * The range is partitioned into the two intervals [0,9.55] and
 * (9.55, infinity).  An ascending power series is used in the
 * low range, and an asymptotic expansion in the high range.
 *
 *
 *
 * ACCURACY:
 *
 *          Absolute error, relative when function > 1:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,30        10000       2.0e-4      3.8e-6
 *
 *  Error is high only near the crossover point x = 9.55
 * between the two expansions used.
 */


/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1988, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140

*/


/*
Algorithm for Kn.
                       n-1 
                   -n   -  (n-k-1)!    2   k
K (x)  =  0.5 (x/2)     >  -------- (-x /4)
 n                      -     k!
                       k=0

                    inf.                                   2   k
       n         n   -                                   (x /4)
 + (-1)  0.5(x/2)    >  {p(k+1) + p(n+k+1) - 2log(x/2)} ---------
                     -                                  k! (n+k)!
                    k=0

where  p(m) is the psi function: p(1) = -EUL and

                      m-1
                       -
      p(m)  =  -EUL +  >  1/k
                       -
                      k=1

For large x,
                                         2        2     2
                                      u-1     (u-1 )(u-3 )
K (z)  =  sqrt(pi/2z) exp(-z) { 1 + ------- + ------------ + ...}
 v                                        1            2
                                    1! (8z)     2! (8z)
asymptotically, where

           2
    u = 4 v .

*/

#include "mconf.h"

#define EUL 5.772156649015328606065e-1
#define MAXFAC 31
extern float MACHEPF, MAXNUMF, MAXLOGF, PIF;

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#ifdef ANSIC
float expf(float), logf(float), sqrtf(float);

float knf( int nnn, float xx )
#else
float expf(), logf(), sqrtf();

float knf( nnn, xx )
int nnn;
double xx;
#endif
{
float x, k, kf, nk1f, nkf, zn, t, s, z0, z;
float ans, fn, pn, pk, zmn, tlg, tox;
int i, n, nn;

nn = nnn;
x = xx;
if( nn < 0 )
	n = -nn;
else
	n = nn;

if( n > MAXFAC )
	{
overf:
	mtherr( "knf", OVERFLOW );
	return( MAXNUMF );
	}

if( x <= 0.0 )
	{
	if( x < 0.0 )
		mtherr( "knf", DOMAIN );
	else
		mtherr( "knf", SING );
	return( MAXNUMF );
	}


if( x > 9.55 )
	goto asymp;

ans = 0.0;
z0 = 0.25 * x * x;
fn = 1.0;
pn = 0.0;
zmn = 1.0;
tox = 2.0/x;

if( n > 0 )
	{
	/* compute factorial of n and psi(n) */
	pn = -EUL;
	k = 1.0;
	for( i=1; i<n; i++ )
		{
		pn += 1.0/k;
		k += 1.0;
		fn *= k;
		}

	zmn = tox;

	if( n == 1 )
		{
		ans = 1.0/x;
		}
	else
		{
		nk1f = fn/n;
		kf = 1.0;
		s = nk1f;
		z = -z0;
		zn = 1.0;
		for( i=1; i<n; i++ )
			{
			nk1f = nk1f/(n-i);
			kf = kf * i;
			zn *= z;
			t = nk1f * zn / kf;
			s += t;   
			if( (MAXNUMF - fabsf(t)) < fabsf(s) )
				goto overf;
			if( (tox > 1.0) && ((MAXNUMF/tox) < zmn) )
				goto overf;
			zmn *= tox;
			}
		s *= 0.5;
		t = fabsf(s);
		if( (zmn > 1.0) && ((MAXNUMF/zmn) < t) )
			goto overf;
		if( (t > 1.0) && ((MAXNUMF/t) < zmn) )
			goto overf;
		ans = s * zmn;
		}
	}


tlg = 2.0 * logf( 0.5 * x );
pk = -EUL;
if( n == 0 )
	{
	pn = pk;
	t = 1.0;
	}
else
	{
	pn = pn + 1.0/n;
	t = 1.0/fn;
	}
s = (pk+pn-tlg)*t;
k = 1.0;
do
	{
	t *= z0 / (k * (k+n));
	pk += 1.0/k;
	pn += 1.0/(k+n);
	s += (pk+pn-tlg)*t;
	k += 1.0;
	}
while( fabsf(t/s) > MACHEPF );

s = 0.5 * s / zmn;
if( n & 1 )
	s = -s;
ans += s;

return(ans);



/* Asymptotic expansion for Kn(x) */
/* Converges to 1.4e-17 for x > 18.4 */

asymp:

if( x > MAXLOGF )
	{
	mtherr( "knf", UNDERFLOW );
	return(0.0);
	}
k = n;
pn = 4.0 * k * k;
pk = 1.0;
z0 = 8.0 * x;
fn = 1.0;
t = 1.0;
s = t;
nkf = MAXNUMF;
i = 0;
do
	{
	z = pn - pk * pk;
	t = t * z /(fn * z0);
	nk1f = fabsf(t);
	if( (i >= n) && (nk1f > nkf) )
		{
		goto adone;
		}
	nkf = nk1f;
	s += t;
	fn += 1.0;
	pk += 2.0;
	i += 1;
	}
while( fabsf(t/s) > MACHEPF );

adone:
ans = expf(-x) * sqrtf( PIF/(2.0*x) ) * s;
return(ans);
}
