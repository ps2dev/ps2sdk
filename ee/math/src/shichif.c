/*							shichif.c
 *
 *	Hyperbolic sine and cosine integrals
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, Chi, Shi;
 *
 * shichi( x, &Chi, &Shi );
 *
 *
 * DESCRIPTION:
 *
 * Approximates the integrals
 *
 *                            x
 *                            -
 *                           | |   cosh t - 1
 *   Chi(x) = eul + ln x +   |    -----------  dt,
 *                         | |          t
 *                          -
 *                          0
 *
 *               x
 *               -
 *              | |  sinh t
 *   Shi(x) =   |    ------  dt
 *            | |       t
 *             -
 *             0
 *
 * where eul = 0.57721566490153286061 is Euler's constant.
 * The integrals are evaluated by power series for x < 8
 * and by Chebyshev expansions for x between 8 and 88.
 * For large x, both functions approach exp(x)/2x.
 * Arguments greater than 88 in magnitude return MAXNUM.
 *
 *
 * ACCURACY:
 *
 * Test interval 0 to 88.
 *                      Relative error:
 * arithmetic   function  # trials      peak         rms
 *    IEEE         Shi      20000       3.5e-7      7.0e-8
 *        Absolute error, except relative when |Chi| > 1:
 *    IEEE         Chi      20000       3.8e-7      7.6e-8
 */

/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"

/* x exp(-x) shi(x), inverted interval 8 to 18 */
static float S1[] = {
-3.56699611114982536845E-8,
 1.44818877384267342057E-7,
 7.82018215184051295296E-7,
-5.39919118403805073710E-6,
-3.12458202168959833422E-5,
 8.90136741950727517826E-5,
 2.02558474743846862168E-3,
 2.96064440855633256972E-2,
 1.11847751047257036625E0
};

/* x exp(-x) shi(x), inverted interval 18 to 88 */
static float S2[] = {
 1.69050228879421288846E-8,
 1.25391771228487041649E-7,
 1.16229947068677338732E-6,
 1.61038260117376323993E-5,
 3.49810375601053973070E-4,
 1.28478065259647610779E-2,
 1.03665722588798326712E0
};


/* x exp(-x) chin(x), inverted interval 8 to 18 */
static float C1[] = {
 1.31458150989474594064E-8,
-4.75513930924765465590E-8,
-2.21775018801848880741E-7,
 1.94635531373272490962E-6,
 4.33505889257316408893E-6,
-6.13387001076494349496E-5,
-3.13085477492997465138E-4,
 4.97164789823116062801E-4,
 2.64347496031374526641E-2,
 1.11446150876699213025E0
};

/* x exp(-x) chin(x), inverted interval 18 to 88 */
static float C2[] = {
-3.00095178028681682282E-9,
 7.79387474390914922337E-8,
 1.06942765566401507066E-6,
 1.59503164802313196374E-5,
 3.49592575153777996871E-4,
 1.28475387530065247392E-2,
 1.03665693917934275131E0
};



/* Sine and cosine integrals */

#define EUL 0.57721566490153286061
extern float MACHEPF, MAXNUMF;

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#ifdef ANSIC
float logf(float ), expf(float), chbevlf(float, float *, int);
#else
float logf(), expf(), chbevlf();
#endif



#ifdef ANSIC
int shichif( float xx, float *si, float *ci )
#else
int shichif( xx, si, ci )
double xx;
float *si, *ci;
#endif
{
float x, k, z, c, s, a;
short sign;

x = xx;
if( x < 0.0 )
	{
	sign = -1;
	x = -x;
	}
else
	sign = 0;


if( x == 0.0 )
	{
	*si = 0.0;
	*ci = -MAXNUMF;
	return( 0 );
	}

if( x >= 8.0 )
	goto chb;

z = x * x;

/*	Direct power series expansion	*/

a = 1.0;
s = 1.0;
c = 0.0;
k = 2.0;

do
	{
	a *= z/k;
	c += a/k;
	k += 1.0;
	a /= k;
	s += a/k;
	k += 1.0;
	}
while( fabsf(a/s) > MACHEPF );

s *= x;
goto done;


chb:

if( x < 18.0 )
	{
	a = (576.0/x - 52.0)/10.0;
	k = expf(x) / x;
	s = k * chbevlf( a, S1, 9 );
	c = k * chbevlf( a, C1, 10 );
	goto done;
	}

if( x <= 88.0 )
	{
	a = (6336.0/x - 212.0)/70.0;
	k = expf(x) / x;
	s = k * chbevlf( a, S2, 7 );
	c = k * chbevlf( a, C2, 7 );
	goto done;
	}
else
	{
	if( sign )
		*si = -MAXNUMF;
	else
		*si = MAXNUMF;
	*ci = MAXNUMF;
	return(0);
	}
done:
if( sign )
	s = -s;

*si = s;

*ci = EUL + logf(x) + c;
return(0);
}
