/*							bdtrf.c
 *
 *	Binomial distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * int k, n;
 * float p, y, bdtrf();
 *
 * y = bdtrf( k, n, p );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the sum of the terms 0 through k of the Binomial
 * probability density:
 *
 *   k
 *   --  ( n )   j      n-j
 *   >   (   )  p  (1-p)
 *   --  ( j )
 *  j=0
 *
 * The terms are not summed directly; instead the incomplete
 * beta integral is employed, according to the formula
 *
 * y = bdtr( k, n, p ) = incbet( n-k, k+1, 1-p ).
 *
 * The arguments must be positive, with p ranging from 0 to 1.
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error (p varies from 0 to 1):
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       2000       6.9e-5      1.1e-5
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * bdtrf domain        k < 0            0.0
 *                     n < k
 *                     x < 0, x > 1
 *
 */
/*							bdtrcf()
 *
 *	Complemented binomial distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * int k, n;
 * float p, y, bdtrcf();
 *
 * y = bdtrcf( k, n, p );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the sum of the terms k+1 through n of the Binomial
 * probability density:
 *
 *   n
 *   --  ( n )   j      n-j
 *   >   (   )  p  (1-p)
 *   --  ( j )
 *  j=k+1
 *
 * The terms are not summed directly; instead the incomplete
 * beta integral is employed, according to the formula
 *
 * y = bdtrc( k, n, p ) = incbet( k+1, n-k, p ).
 *
 * The arguments must be positive, with p ranging from 0 to 1.
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error (p varies from 0 to 1):
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       2000       6.0e-5      1.2e-5
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * bdtrcf domain     x<0, x>1, n<k       0.0
 */
/*							bdtrif()
 *
 *	Inverse binomial distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * int k, n;
 * float p, y, bdtrif();
 *
 * p = bdtrf( k, n, y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Finds the event probability p such that the sum of the
 * terms 0 through k of the Binomial probability density
 * is equal to the given cumulative probability y.
 *
 * This is accomplished using the inverse beta integral
 * function and the relation
 *
 * 1 - p = incbi( n-k, k+1, y ).
 *
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error (p varies from 0 to 1):
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       2000       3.5e-5      3.3e-6
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * bdtrif domain    k < 0, n <= k         0.0
 *                  x < 0, x > 1
 *
 */

/*								bdtr() */


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

#ifdef ANSIC
float incbetf(float, float, float), powf(float, float);
float incbif( float, float, float );
#else
float incbetf(), powf(), incbif();
#endif

#ifdef ANSIC
float bdtrcf( int k, int n, float pp )
#else
float bdtrcf( k, n, pp )
int k, n;
double pp;
#endif
{
float p, dk, dn;

p = pp;
if( (p < 0.0) || (p > 1.0) )
	goto domerr;
if( k < 0 )
	return( 1.0 );

if( n < k )
	{
domerr:
	mtherr( "bdtrcf", DOMAIN );
	return( 0.0 );
	}

if( k == n )
	return( 0.0 );
dn = n - k;
if( k == 0 )
	{
	dk = 1.0 - powf( 1.0-p, dn );
	}
else
	{
	dk = k + 1;
	dk = incbetf( dk, dn, p );
	}
return( dk );
}



#ifdef ANSIC
float bdtrf( int k, int n, float pp )
#else
float bdtrf( k, n, pp )
int k, n;
double pp;
#endif
{
float p, dk, dn;

p = pp;
if( (p < 0.0) || (p > 1.0) )
	goto domerr;
if( (k < 0) || (n < k) )
	{
domerr:
	mtherr( "bdtrf", DOMAIN );
	return( 0.0 );
	}

if( k == n )
	return( 1.0 );

dn = n - k;
if( k == 0 )
	{
	dk = powf( 1.0-p, dn );
	}
else
	{
	dk = k + 1;
	dk = incbetf( dn, dk, 1.0 - p );
	}
return( dk );
}


#ifdef ANSIC
float bdtrif( int k, int n, float yy )
#else
float bdtrif( k, n, yy )
int k, n;
double yy;
#endif
{
float y, dk, dn, p;

y = yy;
if( (y < 0.0) || (y > 1.0) )
	goto domerr;
if( (k < 0) || (n <= k) )
	{
domerr:
	mtherr( "bdtrif", DOMAIN );
	return( 0.0 );
	}

dn = n - k;
if( k == 0 )
	{
	p = 1.0 - powf( y, 1.0/dn );
	}
else
	{
	dk = k + 1;
	p = 1.0 - incbif( dn, dk, y );
	}
return( p );
}
