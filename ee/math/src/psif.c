/*							psif.c
 *
 *	Psi (digamma) function
 *
 *
 * SYNOPSIS:
 *
 * float x, y, psif();
 *
 * y = psif( x );
 *
 *
 * DESCRIPTION:
 *
 *              d      -
 *   psi(x)  =  -- ln | (x)
 *              dx
 *
 * is the logarithmic derivative of the gamma function.
 * For integer x,
 *                   n-1
 *                    -
 * psi(n) = -EUL  +   >  1/k.
 *                    -
 *                   k=1
 *
 * This formula is used for 0 < n <= 10.  If x is negative, it
 * is transformed to a positive argument by the reflection
 * formula  psi(1-x) = psi(x) + pi cot(pi x).
 * For general positive x, the argument is made greater than 10
 * using the recurrence  psi(x+1) = psi(x) + 1/x.
 * Then the following asymptotic expansion is applied:
 *
 *                           inf.   B
 *                            -      2k
 * psi(x) = log(x) - 1/2x -   >   -------
 *                            -        2k
 *                           k=1   2k x
 *
 * where the B2k are Bernoulli numbers.
 *
 * ACCURACY:
 *    Absolute error,  relative when |psi| > 1 :
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -33,0        30000      8.2e-7      1.2e-7
 *    IEEE      0,33        100000      7.3e-7      7.7e-8
 *
 * ERROR MESSAGES:
 *     message         condition      value returned
 * psi singularity    x integer <=0      MAXNUMF
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"


static float A[] = {
-4.16666666666666666667E-3,
 3.96825396825396825397E-3,
-8.33333333333333333333E-3,
 8.33333333333333333333E-2
};


#define EUL 0.57721566490153286061

extern float PIF, MAXNUMF;



#ifdef ANSIC
float floorf(float), logf(float), tanf(float);
float polevlf(float, float *, int);

float psif(float xx)
#else
float floorf(), logf(), tanf(), polevlf();

float psif(xx)
double xx;
#endif
{
float p, q, nz, x, s, w, y, z;
int i, n, negative;


x = xx;
nz = 0.0;
negative = 0;
if( x <= 0.0 )
	{
	negative = 1;
	q = x;
	p = floorf(q);
	if( p == q )
		{
		mtherr( "psif", SING );
		return( MAXNUMF );
		}
	nz = q - p;
	if( nz != 0.5 )
		{
		if( nz > 0.5 )
			{
			p += 1.0;
			nz = q - p;
			}
		nz = PIF/tanf(PIF*nz);
		}
	else
		{
		nz = 0.0;
		}
	x = 1.0 - x;
	}

/* check for positive integer up to 10 */
if( (x <= 10.0) && (x == floorf(x)) )
	{
	y = 0.0;
	n = x;
	for( i=1; i<n; i++ )
		{
		w = i;
		y += 1.0/w;
		}
	y -= EUL;
	goto done;
	}

s = x;
w = 0.0;
while( s < 10.0 )
	{
	w += 1.0/s;
	s += 1.0;
	}

if( s < 1.0e8 )
	{
	z = 1.0/(s * s);
	y = z * polevlf( z, A, 3 );
	}
else
	y = 0.0;

y = logf(s)  -  (0.5/s)  -  y  -  w;

done:
if( negative )
	{
	y -= nz;
	}
return(y);
}
