/*							atanf.c
 *
 *	Inverse circular tangent
 *      (arctangent)
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, atanf();
 *
 * y = atanf( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns radian angle between -pi/2 and +pi/2 whose tangent
 * is x.
 *
 * Range reduction is from four intervals into the interval
 * from zero to  tan( pi/8 ).  A polynomial approximates
 * the function in this basic interval.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10, 10     100000      1.9e-7      4.1e-8
 *
 */
/*							atan2f()
 *
 *	Quadrant correct inverse circular tangent
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y, z, atan2f();
 *
 * z = atan2f( y, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns radian angle whose tangent is y/x.
 * Define compile time symbol ANSIC = 1 for ANSI standard,
 * range -PI < z <= +PI, args (y,x); else ANSIC = 0 for range
 * 0 to 2PI, args (x,y).
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10, 10     100000      1.9e-7      4.1e-8
 * See atan.c.
 *
 */

/*							atan.c */


/*
Cephes Math Library Release 2.2:  June, 1992
Copyright 1984, 1987, 1989, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* Single precision circular arcsine
 * test interval: [-tan(pi/8), +tan(pi/8)]
 * trials: 10000
 * peak relative error: 7.7e-8
 * rms relative error: 2.9e-8
 */
#include "mconf.h"
extern float PIF, PIO2F, PIO4F;

#ifdef ANSIC
float atanf( float xx )
#else
float atanf(xx)
double xx;
#endif
{
float x, y, z;
int sign;

x = xx;

/* make argument positive and save the sign */
if( xx < 0.0 )
	{
	sign = -1;
	x = -xx;
	}
else
	{
	sign = 1;
	x = xx;
	}
/* range reduction */
if( x > 2.414213562373095 )  /* tan 3pi/8 */
	{
	y = PIO2F;
	x = -( 1.0/x );
	}

else if( x > 0.4142135623730950 ) /* tan pi/8 */
	{
	y = PIO4F;
	x = (x-1.0)/(x+1.0);
	}
else
	y = 0.0;

z = x * x;
y +=
((( 8.05374449538e-2 * z
  - 1.38776856032E-1) * z
  + 1.99777106478E-1) * z
  - 3.33329491539E-1) * z * x
  + x;

if( sign < 0 )
	y = -y;

return( y );
}




#if ANSIC
float atan2f( float y, float x )
#else
float atan2f( x, y )
double x, y;
#endif
{
float z, w;
int code;


code = 0;

if( x < 0.0 )
	code = 2;
if( y < 0.0 )
	code |= 1;

if( x == 0.0 )
	{
	if( code & 1 )
		{
#if ANSIC
		return( -PIO2F );
#else
		return( 3.0*PIO2F );
#endif
		}
	if( y == 0.0 )
		return( 0.0 );
	return( PIO2F );
	}

if( y == 0.0 )
	{
	if( code & 2 )
		return( PIF );
	return( 0.0 );
	}


switch( code )
	{
	default:
#if ANSIC
	case 0:
	case 1: w = 0.0; break;
	case 2: w = PIF; break;
	case 3: w = -PIF; break;
#else
	case 0: w = 0.0; break;
	case 1: w = 2.0 * PIF; break;
	case 2:
	case 3: w = PIF; break;
#endif
	}

z = atanf( y/x );

return( w + z );
}

