/*							ndtrif.c
 *
 *	Inverse of Normal distribution function
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, ndtrif();
 *
 * x = ndtrif( y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the argument, x, for which the area under the
 * Gaussian probability density function (integrated from
 * minus infinity to x) is equal to y.
 *
 *
 * For small arguments 0 < y < exp(-2), the program computes
 * z = sqrt( -2.0 * log(y) );  then the approximation is
 * x = z - log(z)/z  - (1/z) P(1/z) / Q(1/z).
 * There are two rational functions P/Q, one for 0 < y < exp(-32)
 * and the other for y up to exp(-2).  For larger arguments,
 * w = y - 0.5, and  x/sqrt(2pi) = w + w**3 R(w**2)/S(w**2)).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain        # trials      peak         rms
 *    IEEE     1e-38, 1        30000       3.6e-7      5.0e-8
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition    value returned
 * ndtrif domain      x <= 0        -MAXNUM
 * ndtrif domain      x >= 1         MAXNUM
 *
 */


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1989, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"
extern float MAXNUMF;

/* sqrt(2pi) */
static float s2pi = 2.50662827463100050242;

/* approximation for 0 <= |y - 0.5| <= 3/8 */
static float P0[5] = {
-5.99633501014107895267E1,
 9.80010754185999661536E1,
-5.66762857469070293439E1,
 1.39312609387279679503E1,
-1.23916583867381258016E0,
};
static float Q0[8] = {
/* 1.00000000000000000000E0,*/
 1.95448858338141759834E0,
 4.67627912898881538453E0,
 8.63602421390890590575E1,
-2.25462687854119370527E2,
 2.00260212380060660359E2,
-8.20372256168333339912E1,
 1.59056225126211695515E1,
-1.18331621121330003142E0,
};

/* Approximation for interval z = sqrt(-2 log y ) between 2 and 8
 * i.e., y between exp(-2) = .135 and exp(-32) = 1.27e-14.
 */
static float P1[9] = {
 4.05544892305962419923E0,
 3.15251094599893866154E1,
 5.71628192246421288162E1,
 4.40805073893200834700E1,
 1.46849561928858024014E1,
 2.18663306850790267539E0,
-1.40256079171354495875E-1,
-3.50424626827848203418E-2,
-8.57456785154685413611E-4,
};
static float Q1[8] = {
/*  1.00000000000000000000E0,*/
 1.57799883256466749731E1,
 4.53907635128879210584E1,
 4.13172038254672030440E1,
 1.50425385692907503408E1,
 2.50464946208309415979E0,
-1.42182922854787788574E-1,
-3.80806407691578277194E-2,
-9.33259480895457427372E-4,
};


/* Approximation for interval z = sqrt(-2 log y ) between 8 and 64
 * i.e., y between exp(-32) = 1.27e-14 and exp(-2048) = 3.67e-890.
 */

static float P2[9] = {
  3.23774891776946035970E0,
  6.91522889068984211695E0,
  3.93881025292474443415E0,
  1.33303460815807542389E0,
  2.01485389549179081538E-1,
  1.23716634817820021358E-2,
  3.01581553508235416007E-4,
  2.65806974686737550832E-6,
  6.23974539184983293730E-9,
};
static float Q2[8] = {
/*  1.00000000000000000000E0,*/
  6.02427039364742014255E0,
  3.67983563856160859403E0,
  1.37702099489081330271E0,
  2.16236993594496635890E-1,
  1.34204006088543189037E-2,
  3.28014464682127739104E-4,
  2.89247864745380683936E-6,
  6.79019408009981274425E-9,
};

#ifdef ANSIC
float polevlf(float, float *, int);
float p1evlf(float, float *, int);
float logf(float), sqrtf(float);
#else
float polevlf(), p1evlf(), logf(), sqrtf();
#endif


#ifdef ANSIC
float ndtrif(float yy0)
#else
float ndtrif(yy0)
double yy0;
#endif
{
float y0, x, y, z, y2, x0, x1;
int code;

y0 = yy0;
if( y0 <= 0.0 )
	{
	mtherr( "ndtrif", DOMAIN );
	return( -MAXNUMF );
	}
if( y0 >= 1.0 )
	{
	mtherr( "ndtrif", DOMAIN );
	return( MAXNUMF );
	}
code = 1;
y = y0;
if( y > (1.0 - 0.13533528323661269189) ) /* 0.135... = exp(-2) */
	{
	y = 1.0 - y;
	code = 0;
	}

if( y > 0.13533528323661269189 )
	{
	y = y - 0.5;
	y2 = y * y;
	x = y + y * (y2 * polevlf( y2, P0, 4)/p1evlf( y2, Q0, 8 ));
	x = x * s2pi; 
	return(x);
	}

x = sqrtf( -2.0 * logf(y) );
x0 = x - logf(x)/x;

z = 1.0/x;
if( x < 8.0 ) /* y > exp(-32) = 1.2664165549e-14 */
	x1 = z * polevlf( z, P1, 8 )/p1evlf( z, Q1, 8 );
else
	x1 = z * polevlf( z, P2, 8 )/p1evlf( z, Q2, 8 );
x = x0 - x1;
if( code != 0 )
	x = -x;
return( x );
}
