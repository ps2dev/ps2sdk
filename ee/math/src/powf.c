/*							powf.c
 *
 *	Power function
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, z, powf();
 *
 * z = powf( x, y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Computes x raised to the yth power.  Analytically,
 *
 *      x**y  =  exp( y log(x) ).
 *
 * Following Cody and Waite, this program uses a lookup table
 * of 2**-i/16 and pseudo extended precision arithmetic to
 * obtain an extra three bits of accuracy in both the logarithm
 * and the exponential.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 *  arithmetic  domain     # trials      peak         rms
 *    IEEE     -10,10      100,000      1.4e-7      3.6e-8
 * 1/10 < x < 10, x uniformly distributed.
 * -10 < y < 10, y uniformly distributed.
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * powf overflow     x**y > MAXNUMF     MAXNUMF
 * powf underflow   x**y < 1/MAXNUMF      0.0
 * powf domain      x<0 and y noninteger  0.0
 *
 */

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"
static char fname[] = {"powf"};


/* 2^(-i/16)
 * The decimal values are rounded to 24-bit precision
 */
static float A[] = {
  1.00000000000000000000E0,
 9.57603275775909423828125E-1,
 9.17004048824310302734375E-1,
 8.78126084804534912109375E-1,
 8.40896427631378173828125E-1,
 8.05245161056518554687500E-1,
 7.71105408668518066406250E-1,
 7.38413095474243164062500E-1,
 7.07106769084930419921875E-1,
 6.77127778530120849609375E-1,
 6.48419797420501708984375E-1,
 6.20928883552551269531250E-1,
 5.94603538513183593750000E-1,
 5.69394290447235107421875E-1,
 5.45253872871398925781250E-1,
 5.22136867046356201171875E-1,
  5.00000000000000000000E-1
};
/* continuation, for even i only
 * 2^(i/16)  =  A[i] + B[i/2]
 */
static float B[] = {
 0.00000000000000000000E0,
-5.61963907099083340520586E-9,
-1.23776636307969995237668E-8,
 4.03545234539989593104537E-9,
 1.21016171044789693621048E-8,
-2.00949968760174979411038E-8,
 1.89881769396087499852802E-8,
-6.53877009617774467211965E-9,
 0.00000000000000000000E0
};

/* 1 / A[i]
 * The decimal values are full precision
 */
static float Ainv[] = {
 1.00000000000000000000000E0,
 1.04427378242741384032197E0,
 1.09050773266525765920701E0,
 1.13878863475669165370383E0,
 1.18920711500272106671750E0,
 1.24185781207348404859368E0,
 1.29683955465100966593375E0,
 1.35425554693689272829801E0,
 1.41421356237309504880169E0,
 1.47682614593949931138691E0,
 1.54221082540794082361229E0,
 1.61049033194925430817952E0,
 1.68179283050742908606225E0,
 1.75625216037329948311216E0,
 1.83400808640934246348708E0,
 1.91520656139714729387261E0,
 2.00000000000000000000000E0
};

#ifdef DEC
#define MEXP 2032.0
#define MNEXP -2032.0
#else
#define MEXP 2048.0
#define MNEXP -2400.0
#endif

/* log2(e) - 1 */
#define LOG2EA 0.44269504088896340736F
extern float MAXNUMF;

#define F W
#define Fa Wa
#define Fb Wb
#define G W
#define Ga Wa
#define Gb u
#define H W
#define Ha Wb
#define Hb Wb


#ifdef ANSIC
float floorf( float );
float frexpf( float, int *);
float ldexpf( float, int );
float powif( float, int );
#else
float floorf(), frexpf(), ldexpf(), powif();
#endif

/* Find a multiple of 1/16 that is within 1/16 of x. */
#define reduc(x)  0.0625 * floorf( 16 * (x) )

#ifdef ANSIC
float powf( float x, float y )
#else
float powf( x, y )
float x, y;
#endif
{
float u, w, z, W, Wa, Wb, ya, yb;
/* float F, Fa, Fb, G, Ga, Gb, H, Ha, Hb */
int e, i, nflg;


nflg = 0;	/* flag = 1 if x<0 raised to integer power */
w = floorf(y);
if( w < 0 )
	z = -w;
else
	z = w;
if( (w == y) && (z < 32768.0) )
	{
	i = w;
	w = powif( x, i );
	return( w );
	}


if( x <= 0.0F )
	{
	if( x == 0.0 )
		{
		if( y == 0.0 )
			return( 1.0 );  /*   0**0   */
		else  
			return( 0.0 );  /*   0**y   */
		}
	else
		{
		if( w != y )
			{ /* noninteger power of negative number */
			mtherr( fname, DOMAIN );
			return(0.0);
			}
		nflg = 1;
		if( x < 0 )
			x = -x;
		}
	}

/* separate significand from exponent */
x = frexpf( x, &e );

/* find significand in antilog table A[] */
i = 1;
if( x <= A[9] )
	i = 9;
if( x <= A[i+4] )
	i += 4;
if( x <= A[i+2] )
	i += 2;
if( x >= A[1] )
	i = -1;
i += 1;


/* Find (x - A[i])/A[i]
 * in order to compute log(x/A[i]):
 *
 * log(x) = log( a x/a ) = log(a) + log(x/a)
 *
 * log(x/a) = log(1+v),  v = x/a - 1 = (x-a)/a
 */
x -= A[i];
x -= B[ i >> 1 ];
x *= Ainv[i];


/* rational approximation for log(1+v):
 *
 * log(1+v)  =  v  -  0.5 v^2  +  v^3 P(v)
 * Theoretical relative error of the approximation is 3.5e-11
 * on the interval 2^(1/16) - 1  > v > 2^(-1/16) - 1
 */
z = x*x;
w = (((-0.1663883081054895  * x
      + 0.2003770364206271) * x
      - 0.2500006373383951) * x
      + 0.3333331095506474) * x * z;
w -= 0.5 * z;

/* Convert to base 2 logarithm:
 * multiply by log2(e)
 */
w = w + LOG2EA * w;
/* Note x was not yet added in
 * to above rational approximation,
 * so do it now, while multiplying
 * by log2(e).
 */
z = w + LOG2EA * x;
z = z + x;

/* Compute exponent term of the base 2 logarithm. */
w = -i;
w *= 0.0625;  /* divide by 16 */
w += e;
/* Now base 2 log of x is w + z. */

/* Multiply base 2 log by y, in extended precision. */

/* separate y into large part ya
 * and small part yb less than 1/16
 */
ya = reduc(y);
yb = y - ya;


F = z * y  +  w * yb;
Fa = reduc(F);
Fb = F - Fa;

G = Fa + w * ya;
Ga = reduc(G);
Gb = G - Ga;

H = Fb + Gb;
Ha = reduc(H);
w = 16 * (Ga + Ha);

/* Test the power of 2 for overflow */
if( w > MEXP )
	{
	mtherr( fname, OVERFLOW );
	return( MAXNUMF );
	}

if( w < MNEXP )
	{
	mtherr( fname, UNDERFLOW );
	return( 0.0 );
	}

e = w;
Hb = H - Ha;

if( Hb > 0.0 )
	{
	e += 1;
	Hb -= 0.0625;
	}

/* Now the product y * log2(x)  =  Hb + e/16.0.
 *
 * Compute base 2 exponential of Hb,
 * where -0.0625 <= Hb <= 0.
 * Theoretical relative error of the approximation is 2.8e-12.
 */
/*  z  =  2**Hb - 1    */
z = ((( 9.416993633606397E-003 * Hb
      + 5.549356188719141E-002) * Hb
      + 2.402262883964191E-001) * Hb
      + 6.931471791490764E-001) * Hb;

/* Express e/16 as an integer plus a negative number of 16ths.
 * Find lookup table entry for the fractional power of 2.
 */
if( e < 0 )
	i = -( -e >> 4 );
else
	i = (e >> 4) + 1;
e = (i << 4) - e;
w = A[e];
z = w + w * z;      /*    2**-e * ( 1 + (2**Hb-1) )    */
z = ldexpf( z, i );  /* multiply by integer power of 2 */

if( nflg )
	{
/* For negative x,
 * find out if the integer exponent
 * is odd or even.
 */
	w = 2 * floorf( (float) 0.5 * w );
	if( w != y )
		z = -z; /* odd exponent */
	}

return( z );
}
