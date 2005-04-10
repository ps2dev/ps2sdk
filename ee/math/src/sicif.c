/*							sicif.c
 *
 *	Sine and cosine integrals
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, Ci, Si;
 *
 * sicif( x, &Si, &Ci );
 *
 *
 * DESCRIPTION:
 *
 * Evaluates the integrals
 *
 *                          x
 *                          -
 *                         |  cos t - 1
 *   Ci(x) = eul + ln x +  |  --------- dt,
 *                         |      t
 *                        -
 *                         0
 *             x
 *             -
 *            |  sin t
 *   Si(x) =  |  ----- dt
 *            |    t
 *           -
 *            0
 *
 * where eul = 0.57721566490153286061 is Euler's constant.
 * The integrals are approximated by rational functions.
 * For x > 8 auxiliary functions f(x) and g(x) are employed
 * such that
 *
 * Ci(x) = f(x) sin(x) - g(x) cos(x)
 * Si(x) = pi/2 - f(x) cos(x) - g(x) sin(x)
 *
 *
 * ACCURACY:
 *    Test interval = [0,50].
 * Absolute error, except relative when > 1:
 * arithmetic   function   # trials      peak         rms
 *    IEEE        Si        30000       2.1e-7      4.3e-8
 *    IEEE        Ci        30000       3.9e-7      2.2e-8
 */

/*
Cephes Math Library Release 2.1:  January, 1989
Copyright 1984, 1987, 1989 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

static float SN[] = {
-8.39167827910303881427E-11,
 4.62591714427012837309E-8,
-9.75759303843632795789E-6,
 9.76945438170435310816E-4,
-4.13470316229406538752E-2,
 1.00000000000000000302E0,
};
static float SD[] = {
  2.03269266195951942049E-12,
  1.27997891179943299903E-9,
  4.41827842801218905784E-7,
  9.96412122043875552487E-5,
  1.42085239326149893930E-2,
  9.99999999999999996984E-1,
};

static float CN[] = {
 2.02524002389102268789E-11,
-1.35249504915790756375E-8,
 3.59325051419993077021E-6,
-4.74007206873407909465E-4,
 2.89159652607555242092E-2,
-1.00000000000000000080E0,
};
static float CD[] = {
  4.07746040061880559506E-12,
  3.06780997581887812692E-9,
  1.23210355685883423679E-6,
  3.17442024775032769882E-4,
  5.10028056236446052392E-2,
  4.00000000000000000080E0,
};


static float FN4[] = {
  4.23612862892216586994E0,
  5.45937717161812843388E0,
  1.62083287701538329132E0,
  1.67006611831323023771E-1,
  6.81020132472518137426E-3,
  1.08936580650328664411E-4,
  5.48900223421373614008E-7,
};
static float FD4[] = {
/*  1.00000000000000000000E0,*/
  8.16496634205391016773E0,
  7.30828822505564552187E0,
  1.86792257950184183883E0,
  1.78792052963149907262E-1,
  7.01710668322789753610E-3,
  1.10034357153915731354E-4,
  5.48900252756255700982E-7,
};


static float FN8[] = {
  4.55880873470465315206E-1,
  7.13715274100146711374E-1,
  1.60300158222319456320E-1,
  1.16064229408124407915E-2,
  3.49556442447859055605E-4,
  4.86215430826454749482E-6,
  3.20092790091004902806E-8,
  9.41779576128512936592E-11,
  9.70507110881952024631E-14,
};
static float FD8[] = {
/*  1.00000000000000000000E0,*/
  9.17463611873684053703E-1,
  1.78685545332074536321E-1,
  1.22253594771971293032E-2,
  3.58696481881851580297E-4,
  4.92435064317881464393E-6,
  3.21956939101046018377E-8,
  9.43720590350276732376E-11,
  9.70507110881952025725E-14,
};

static float GN4[] = {
  8.71001698973114191777E-2,
  6.11379109952219284151E-1,
  3.97180296392337498885E-1,
  7.48527737628469092119E-2,
  5.38868681462177273157E-3,
  1.61999794598934024525E-4,
  1.97963874140963632189E-6,
  7.82579040744090311069E-9,
};
static float GD4[] = {
/*  1.00000000000000000000E0,*/
  1.64402202413355338886E0,
  6.66296701268987968381E-1,
  9.88771761277688796203E-2,
  6.22396345441768420760E-3,
  1.73221081474177119497E-4,
  2.02659182086343991969E-6,
  7.82579218933534490868E-9,
};

static float GN8[] = {
  6.97359953443276214934E-1,
  3.30410979305632063225E-1,
  3.84878767649974295920E-2,
  1.71718239052347903558E-3,
  3.48941165502279436777E-5,
  3.47131167084116673800E-7,
  1.70404452782044526189E-9,
  3.85945925430276600453E-12,
  3.14040098946363334640E-15,
};
static float GD8[] = {
/*  1.00000000000000000000E0,*/
  1.68548898811011640017E0,
  4.87852258695304967486E-1,
  4.67913194259625806320E-2,
  1.90284426674399523638E-3,
  3.68475504442561108162E-5,
  3.57043223443740838771E-7,
  1.72693748966316146736E-9,
  3.87830166023954706752E-12,
  3.14040098946363335242E-15,
};

#define EUL 0.57721566490153286061
extern float MAXNUMF, PIO2F, MACHEPF;



#ifdef ANSIC
float logf(float), sinf(float), cosf(float);
float polevlf(float, float *, int);
float p1evlf(float, float *, int);
#else
float logf(), sinf(), cosf(), polevlf(), p1evlf();
#endif


#ifdef ANSIC
int sicif( float xx, float *si, float *ci )
#else
int sicif( xx, si, ci )
double xx;
float *si, *ci;
#endif
{
float x, z, c, s, f, g;
int sign;

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


if( x > 1.0e9 )
	{
	*si = PIO2F - cosf(x)/x;
	*ci = sinf(x)/x;
	return( 0 );
	}



if( x > 4.0 )
	goto asympt;

z = x * x;
s = x * polevlf( z, SN, 5 ) / polevlf( z, SD, 5 );
c = z * polevlf( z, CN, 5 ) / polevlf( z, CD, 5 );

if( sign )
	s = -s;
*si = s;
*ci = EUL + logf(x) + c;	/* real part if x < 0 */
return(0);



/* The auxiliary functions are:
 *
 *
 * *si = *si - PIO2;
 * c = cos(x);
 * s = sin(x);
 *
 * t = *ci * s - *si * c;
 * a = *ci * c + *si * s;
 *
 * *si = t;
 * *ci = -a;
 */


asympt:

s = sinf(x);
c = cosf(x);
z = 1.0/(x*x);
if( x < 8.0 )
	{
	f = polevlf( z, FN4, 6 ) / (x * p1evlf( z, FD4, 7 ));
	g = z * polevlf( z, GN4, 7 ) / p1evlf( z, GD4, 7 );
	}
else
	{
	f = polevlf( z, FN8, 8 ) / (x * p1evlf( z, FD8, 8 ));
	g = z * polevlf( z, GN8, 8 ) / p1evlf( z, GD8, 9 );
	}
*si = PIO2F - f * c - g * s;
if( sign )
	*si = -( *si );
*ci = f * s - g * c;

return(0);
}
