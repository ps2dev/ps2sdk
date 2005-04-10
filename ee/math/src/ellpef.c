/*							ellpef.c
 *
 *	Complete elliptic integral of the second kind
 *
 *
 *
 * SYNOPSIS:
 *
 * float m1, y, ellpef();
 *
 * y = ellpef( m1 );
 *
 *
 *
 * DESCRIPTION:
 *
 * Approximates the integral
 *
 *
 *            pi/2
 *             -
 *            | |                 2
 * E(m)  =    |    sqrt( 1 - m sin t ) dt
 *          | |    
 *           -
 *            0
 *
 * Where m = 1 - m1, using the approximation
 *
 *      P(x)  -  x log x Q(x).
 *
 * Though there are no singularities, the argument m1 is used
 * rather than m for compatibility with ellpk().
 *
 * E(1) = 1; E(0) = pi/2.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0, 1       30000       1.1e-7      3.9e-8
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * ellpef domain     x<0, x>1            0.0
 *
 */

/*							ellpe.c		*/

/* Elliptic integral of second kind */

/*
Cephes Math Library, Release 2.1:  February, 1989
Copyright 1984, 1987, 1989 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"


static float P[] = {
  1.53552577301013293365E-4,
  2.50888492163602060990E-3,
  8.68786816565889628429E-3,
  1.07350949056076193403E-2,
  7.77395492516787092951E-3,
  7.58395289413514708519E-3,
  1.15688436810574127319E-2,
  2.18317996015557253103E-2,
  5.68051945617860553470E-2,
  4.43147180560990850618E-1,
  1.00000000000000000299E0
};
static float Q[] = {
  3.27954898576485872656E-5,
  1.00962792679356715133E-3,
  6.50609489976927491433E-3,
  1.68862163993311317300E-2,
  2.61769742454493659583E-2,
  3.34833904888224918614E-2,
  4.27180926518931511717E-2,
  5.85936634471101055642E-2,
  9.37499997197644278445E-2,
  2.49999999999888314361E-1
};

#ifdef ANSIC
float polevlf(float, float *, int), logf(float);
float ellpef( float xx)
#else
float polevlf(), logf();
float ellpef(xx)
double xx;
#endif
{
float x;

x = xx;
if( (x <= 0.0) || (x > 1.0) )
	{
	if( x == 0.0 )
		return( 1.0 );
	mtherr( "ellpef", DOMAIN );
	return( 0.0 );
	}
return( polevlf(x,P,10) - logf(x) * (x * polevlf(x,Q,9)) );
}
