/*							acoshf.c
 *
 *	Inverse hyperbolic cosine
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, acoshf();
 *
 * y = acoshf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns inverse hyperbolic cosine of argument.
 *
 * If 1 <= x < 1.5, a polynomial approximation
 *
 *	sqrt(z) * P(z)
 *
 * where z = x-1, is used.  Otherwise,
 *
 * acosh(x)  =  log( x + sqrt( (x-1)(x+1) ).
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      1,3         100000      1.8e-7       3.9e-8
 *    IEEE      1,2000      100000                   3.0e-8
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition      value returned
 * acoshf domain      |x| < 1            0.0
 *
 */

/*							acosh.c	*/

/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1988, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* Single precision inverse hyperbolic cosine
 * test interval: [1.0, 1.5]
 * trials: 10000
 * peak relative error: 1.7e-7
 * rms relative error: 5.0e-8
 *
 * Copyright (C) 1989 by Stephen L. Moshier.  All rights reserved.
 */
#include "mconf.h"
extern float LOGE2F;

#ifdef ANSIC
float sqrtf( float );
float logf( float );

float acoshf( float xx )
#else
float sqrtf(), logf();

float acoshf(xx)
double xx;
#endif
{
float x, z;

x = xx;
if( x < 1.0 )
	{
	mtherr( "acoshf", DOMAIN );
	return(0.0);
	}

if( x > 1500.0 )
	return( logf(x) + LOGE2F );

z = x - 1.0;

if( z < 0.5 )
	{
	z =
	(((( 1.7596881071E-3 * z
	  - 7.5272886713E-3) * z
	  + 2.6454905019E-2) * z
	  - 1.1784741703E-1) * z
	  + 1.4142135263E0) * sqrtf( z );
	}
else
	{
	z = sqrtf( z*(x+1.0) );
	z = logf(x + z);
	}
return( z );
}
