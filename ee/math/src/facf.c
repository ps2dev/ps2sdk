/*							facf.c
 *
 *	Factorial function
 *
 *
 *
 * SYNOPSIS:
 *
 * float y, facf();
 * int i;
 *
 * y = facf( i );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns factorial of i  =  1 * 2 * 3 * ... * i.
 * fac(0) = 1.0.
 *
 * Due to machine arithmetic bounds the largest value of
 * i accepted is 33 in single precision arithmetic.
 * Greater values, or negative ones,
 * produce an error message and return MAXNUM.
 *
 *
 *
 * ACCURACY:
 *
 * For i < 34 the values are simply tabulated, and have
 * full machine accuracy.
 *
 */

/*
Cephes Math Library Release 2.0:  April, 1987
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"

/* Factorials of integers from 0 through 33 */
static double factbl[] = {
  1.00000000000000000000E0,
  1.00000000000000000000E0,
  2.00000000000000000000E0,
  6.00000000000000000000E0,
  2.40000000000000000000E1,
  1.20000000000000000000E2,
  7.20000000000000000000E2,
  5.04000000000000000000E3,
  4.03200000000000000000E4,
  3.62880000000000000000E5,
  3.62880000000000000000E6,
  3.99168000000000000000E7,
  4.79001600000000000000E8,
  6.22702080000000000000E9,
  8.71782912000000000000E10,
  1.30767436800000000000E12,
  2.09227898880000000000E13,
  3.55687428096000000000E14,
  6.40237370572800000000E15,
  1.21645100408832000000E17,
  2.43290200817664000000E18,
  5.10909421717094400000E19,
  1.12400072777760768000E21,
  2.58520167388849766400E22,
  6.20448401733239439360E23,
  1.55112100433309859840E25,
  4.03291461126605635584E26,
  1.0888869450418352160768E28,
  3.04888344611713860501504E29,
  8.841761993739701954543616E30,
  2.6525285981219105863630848E32,
  8.22283865417792281772556288E33,
  2.6313083693369353016721801216E35,
  8.68331761881188649551819440128E36
};
#define MAXFACF 33

extern double MAXNUMF;

#ifdef ANSIC
float facf( int i )
#else
float facf(i)
int i;
#endif
{

if( i < 0 )
	{
	mtherr( "facf", SING );
	return( MAXNUMF );
	}

if( i > MAXFACF )
	{
	mtherr( "facf", OVERFLOW );
	return( MAXNUMF );
	}

/* Get answer from table for small i. */
return( factbl[i] );
}
