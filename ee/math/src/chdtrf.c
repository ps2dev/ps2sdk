/*							chdtrf.c
 *
 *	Chi-square distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * float df, x, y, chdtrf();
 *
 * y = chdtrf( df, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the area under the left hand tail (from 0 to x)
 * of the Chi square probability density function with
 * v degrees of freedom.
 *
 *
 *                                  inf.
 *                                    -
 *                        1          | |  v/2-1  -t/2
 *  P( x | v )   =   -----------     |   t      e     dt
 *                    v/2  -       | |
 *                   2    | (v/2)   -
 *                                   x
 *
 * where x is the Chi-square variable.
 *
 * The incomplete gamma integral is used, according to the
 * formula
 *
 *	y = chdtr( v, x ) = igam( v/2.0, x/2.0 ).
 *
 *
 * The arguments must both be positive.
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       5000       3.2e-5      5.0e-6
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * chdtrf domain  x < 0 or v < 1        0.0
 */
/*							chdtrcf()
 *
 *	Complemented Chi-square distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * float v, x, y, chdtrcf();
 *
 * y = chdtrcf( v, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the area under the right hand tail (from x to
 * infinity) of the Chi square probability density function
 * with v degrees of freedom:
 *
 *
 *                                  inf.
 *                                    -
 *                        1          | |  v/2-1  -t/2
 *  P( x | v )   =   -----------     |   t      e     dt
 *                    v/2  -       | |
 *                   2    | (v/2)   -
 *                                   x
 *
 * where x is the Chi-square variable.
 *
 * The incomplete gamma integral is used, according to the
 * formula
 *
 *	y = chdtr( v, x ) = igamc( v/2.0, x/2.0 ).
 *
 *
 * The arguments must both be positive.
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       5000       2.7e-5      3.2e-6
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * chdtrc domain  x < 0 or v < 1        0.0
 */
/*							chdtrif()
 *
 *	Inverse of complemented Chi-square distribution
 *
 *
 *
 * SYNOPSIS:
 *
 * float df, x, y, chdtrif();
 *
 * x = chdtrif( df, y );
 *
 *
 *
 *
 * DESCRIPTION:
 *
 * Finds the Chi-square argument x such that the integral
 * from x to infinity of the Chi-square density is equal
 * to the given cumulative probability y.
 *
 * This is accomplished using the inverse gamma integral
 * function and the relation
 *
 *    x/2 = igami( df/2, y );
 *
 *
 *
 *
 * ACCURACY:
 *
 *        Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,100       10000      2.2e-5      8.5e-7
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * chdtri domain   y < 0 or y > 1        0.0
 *                     v < 1
 *
 */

/*								chdtr() */


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

#ifdef ANSIC
float igamcf(float, float), igamf(float, float), igamif(float, float);
#else
float igamcf(), igamf(), igamif();
#endif

#ifdef ANSIC
float chdtrcf(float dff, float xx)
#else
float chdtrcf(dff,xx)
double dff, xx;
#endif
{
float df, x;

df = dff;
x = xx;

if( (x < 0.0) || (df < 1.0) )
	{
	mtherr( "chdtrcf", DOMAIN );
	return(0.0);
	}
return( igamcf( 0.5*df, 0.5*x ) );
}


#ifdef ANSIC
float chdtrf(float dff, float xx)
#else
float chdtrf(dff,xx)
double dff, xx;
#endif
{
float df, x;

df = dff;
x = xx;
if( (x < 0.0) || (df < 1.0) )
	{
	mtherr( "chdtrf", DOMAIN );
	return(0.0);
	}
return( igamf( 0.5*df, 0.5*x ) );
}


#ifdef ANSIC
float chdtrif( float dff, float yy )
#else
float chdtrif( dff, yy )
double dff, yy;
#endif
{
float y, df, x;

y = yy;
df = dff;
if( (y < 0.0) || (y > 1.0) || (df < 1.0) )
	{
	mtherr( "chdtrif", DOMAIN );
	return(0.0);
	}

x = igamif( 0.5 * df, y );
return( 2.0 * x );
}
