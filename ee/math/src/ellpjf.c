/*							ellpjf.c
 *
 *	Jacobian Elliptic Functions
 *
 *
 *
 * SYNOPSIS:
 *
 * float u, m, sn, cn, dn, phi;
 * int ellpj();
 *
 * ellpj( u, m, _&sn, _&cn, _&dn, _&phi );
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * Evaluates the Jacobian elliptic functions sn(u|m), cn(u|m),
 * and dn(u|m) of parameter m between 0 and 1, and real
 * argument u.
 *
 * These functions are periodic, with quarter-period on the
 * real axis equal to the complete elliptic integral
 * ellpk(1.0-m).
 *
 * Relation to incomplete elliptic integral:
 * If u = ellik(phi,m), then sn(u|m) = sin(phi),
 * and cn(u|m) = cos(phi).  Phi is called the amplitude of u.
 *
 * Computation is by means of the arithmetic-geometric mean
 * algorithm, except when m is within 1e-9 of 0 or 1.  In the
 * latter case with m close to 1, the approximation applies
 * only for phi < pi/2.
 *
 * ACCURACY:
 *
 * Tested at random points with u between 0 and 10, m between
 * 0 and 1.
 *
 *            Absolute error (* = relative error):
 * arithmetic   function   # trials      peak         rms
 *    IEEE      sn          10000       1.7e-6      2.2e-7
 *    IEEE      cn          10000       1.6e-6      2.2e-7
 *    IEEE      dn          10000       1.4e-3      1.9e-5
 *    IEEE      phi         10000       3.9e-7*     6.7e-8*
 *
 *  Peak error observed in consistency check using addition
 * theorem for sn(u+v) was 4e-16 (absolute).  Also tested by
 * the above relation to the incomplete elliptic integral.
 * Accuracy deteriorates when u is large.
 *
 */

/*							ellpj.c		*/


/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include "mconf.h"
extern double PIO2F, MACHEPF;

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#ifdef ANSIC
float sqrtf(float), sinf(float), cosf(float), asinf(float), tanhf(float);
float sinhf(float), coshf(float), atanf(float), expf(float);
#else
float sqrtf(), sinf(), cosf(), asinf(), tanhf();
float sinhf(), coshf(), atanf(), expf();
#endif

#ifdef ANSIC
int ellpjf( float uu, float mm,
   float *sn, float *cn, float *dn, float *ph )
#else
int ellpjf( uu, mm, sn, cn, dn, ph )
double uu, mm;
float *sn, *cn, *dn, *ph;
#endif
{
float u, m, ai, b, phi, t, twon;
float a[10], c[10];
int i;

u = uu;
m = mm;
/* Check for special cases */

if( m < 0.0 || m > 1.0 )
	{
	mtherr( "ellpjf", DOMAIN );
	return(-1);
	}
if( m < 1.0e-5 )
	{
	t = sinf(u);
	b = cosf(u);
	ai = 0.25 * m * (u - t*b);
	*sn = t - ai*b;
	*cn = b + ai*t;
	*ph = u - ai;
	*dn = 1.0 - 0.5*m*t*t;
	return(0);
	}

if( m >= 0.99999 )
	{
	ai = 0.25 * (1.0-m);
	b = coshf(u);
	t = tanhf(u);
	phi = 1.0/b;
	twon = b * sinhf(u);
	*sn = t + ai * (twon - u)/(b*b);
	*ph = 2.0*atanf(expf(u)) - PIO2F + ai*(twon - u)/b;
	ai *= t * phi;
	*cn = phi - ai * (twon - u);
	*dn = phi + ai * (twon + u);
	return(0);
	}


/*	A. G. M. scale		*/
a[0] = 1.0;
b = sqrtf(1.0 - m);
c[0] = sqrtf(m);
twon = 1.0;
i = 0;

while( fabsf( (c[i]/a[i]) ) > MACHEPF )
	{
	if( i > 8 )
		{
/*		mtherr( "ellpjf", OVERFLOW );*/
		break;
		}
	ai = a[i];
	++i;
	c[i] = 0.5 * ( ai - b );
	t = sqrtf( ai * b );
	a[i] = 0.5 * ( ai + b );
	b = t;
	twon += twon;
	}


/* backward recurrence */
phi = twon * a[i] * u;
do
	{
	t = c[i] * sinf(phi) / a[i];
	b = phi;
	phi = 0.5 * (asinf(t) + phi);
	}
while( --i );

*sn = sinf(phi);
t = cosf(phi);
*cn = t;
*dn = t/cosf(phi-b);
*ph = phi;
return(0);
}
