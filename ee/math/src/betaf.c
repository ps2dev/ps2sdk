/*							betaf.c
 *
 *	Beta function
 *
 *
 *
 * SYNOPSIS:
 *
 * float a, b, y, betaf();
 *
 * y = betaf( a, b );
 *
 *
 *
 * DESCRIPTION:
 *
 *                   -     -
 *                  | (a) | (b)
 * beta( a, b )  =  -----------.
 *                     -
 *                    | (a+b)
 *
 * For large arguments the logarithm of the function is
 * evaluated using lgam(), then exponentiated.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE       0,30       10000       4.0e-5      6.0e-6
 *    IEEE       -20,0      10000       4.9e-3      5.4e-5
 *
 * ERROR MESSAGES:
 *
 *   message         condition          value returned
 * betaf overflow   log(beta) > MAXLOG       0.0
 *                  a or b <0 integer        0.0
 *
 */

/*							beta.c	*/


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#define MAXGAM 34.84425627277176174


extern float MAXLOGF, MAXNUMF;
extern int sgngamf;

#ifdef ANSIC
float gammaf(float), lgamf(float), expf(float), floorf(float);
#else
float gammaf(), lgamf(), expf(), floorf();
#endif

#ifdef ANSIC
float betaf( float aa, float bb )
#else
float betaf( aa, bb )
double aa, bb;
#endif
{
float a, b, y;
int sign;

sign = 1;
a = aa;
b = bb;
if( a <= 0.0 )
	{
	if( a == floorf(a) )
		goto over;
	}
if( b <= 0.0 )
	{
	if( b == floorf(b) )
		goto over;
	}


y = a + b;
if( fabsf(y) > MAXGAM )
	{
	y = lgamf(y);
	sign *= sgngamf; /* keep track of the sign */
	y = lgamf(b) - y;
	sign *= sgngamf;
	y = lgamf(a) + y;
	sign *= sgngamf;
	if( y > MAXLOGF )
		{
over:
		mtherr( "betaf", OVERFLOW );
		return( sign * MAXNUMF );
		}
	return( sign * expf(y) );
	}

y = gammaf(y);
if( y == 0.0 )
	goto over;

if( a > b )
	{
	y = gammaf(a)/y;
	y *= gammaf(b);
	}
else
	{
	y = gammaf(b)/y;
	y *= gammaf(a);
	}

return(y);
}
