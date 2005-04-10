/*							cmplxf.c
 *
 *	Complex number arithmetic
 *
 *
 *
 * SYNOPSIS:
 *
 * typedef struct {
 *      float r;     real part
 *      float i;     imaginary part
 *     }cmplxf;
 *
 * cmplxf *a, *b, *c;
 *
 * caddf( a, b, c );     c = b + a
 * csubf( a, b, c );     c = b - a
 * cmulf( a, b, c );     c = b * a
 * cdivf( a, b, c );     c = b / a
 * cnegf( c );           c = -c
 * cmovf( b, c );        c = b
 *
 *
 *
 * DESCRIPTION:
 *
 * Addition:
 *    c.r  =  b.r + a.r
 *    c.i  =  b.i + a.i
 *
 * Subtraction:
 *    c.r  =  b.r - a.r
 *    c.i  =  b.i - a.i
 *
 * Multiplication:
 *    c.r  =  b.r * a.r  -  b.i * a.i
 *    c.i  =  b.r * a.i  +  b.i * a.r
 *
 * Division:
 *    d    =  a.r * a.r  +  a.i * a.i
 *    c.r  = (b.r * a.r  + b.i * a.i)/d
 *    c.i  = (b.i * a.r  -  b.r * a.i)/d
 * ACCURACY:
 *
 * In DEC arithmetic, the test (1/z) * z = 1 had peak relative
 * error 3.1e-17, rms 1.2e-17.  The test (y/z) * (z/y) = 1 had
 * peak relative error 8.3e-17, rms 2.1e-17.
 *
 * Tests in the rectangle {-10,+10}:
 *                      Relative error:
 * arithmetic   function  # trials      peak         rms
 *    IEEE       cadd       30000       5.9e-8      2.6e-8
 *    IEEE       csub       30000       6.0e-8      2.6e-8
 *    IEEE       cmul       30000       1.1e-7      3.7e-8
 *    IEEE       cdiv       30000       2.1e-7      5.7e-8
 */
/*				cmplx.c
 * complex number arithmetic
 */


/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"
extern float MAXNUMF, MACHEPF, PIF, PIO2F;
#define fabsf(x) ( (x) < 0 ? -(x) : (x) )
#ifdef ANSIC
float sqrtf(float), frexpf(float, int *);
float ldexpf(float, int);
float cabsf(cmplxf *), atan2f(float, float), cosf(float), sinf(float);
#else
float sqrtf(), frexpf(), ldexpf();
float cabsf(), atan2f(), cosf(), sinf();
#endif
/*
typedef struct
	{
	float r;
	float i;
	}cmplxf;
*/
cmplxf czerof = {0.0, 0.0};
extern cmplxf czerof;
cmplxf conef = {1.0, 0.0};
extern cmplxf conef;

/*	c = b + a	*/

void caddf( a, b, c )
register cmplxf *a, *b;
cmplxf *c;
{

c->r = b->r + a->r;
c->i = b->i + a->i;
}


/*	c = b - a	*/

void csubf( a, b, c )
register cmplxf *a, *b;
cmplxf *c;
{

c->r = b->r - a->r;
c->i = b->i - a->i;
}

/*	c = b * a */

void cmulf( a, b, c )
register cmplxf *a, *b;
cmplxf *c;
{
register float y;

y    = b->r * a->r  -  b->i * a->i;
c->i = b->r * a->i  +  b->i * a->r;
c->r = y;
}



/*	c = b / a */

void cdivf( a, b, c )
register cmplxf *a, *b;
cmplxf *c;
{
float y, p, q, w;


y = a->r * a->r  +  a->i * a->i;
p = b->r * a->r  +  b->i * a->i;
q = b->i * a->r  -  b->r * a->i;

if( y < 1.0f )
	{
	w = MAXNUMF * y;
	if( (fabsf(p) > w) || (fabsf(q) > w) || (y == 0.0f) )
		{
		c->r = MAXNUMF;
		c->i = MAXNUMF;
		mtherr( "cdivf", OVERFLOW );
		return;
		}
	}
c->r = p/y;
c->i = q/y;
}


/*	b = a	*/

void cmovf( a, b )
register short *a, *b;
{
int i;


i = 8;
do
	*b++ = *a++;
while( --i );
}


void cnegf( a )
register cmplxf *a;
{

a->r = -a->r;
a->i = -a->i;
}

/*							cabsf()
 *
 *	Complex absolute value
 *
 *
 *
 * SYNOPSIS:
 *
 * float cabsf();
 * cmplxf z;
 * float a;
 *
 * a = cabsf( &z );
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * If z = x + iy
 *
 * then
 *
 *       a = sqrt( x**2 + y**2 ).
 * 
 * Overflow and underflow are avoided by testing the magnitudes
 * of x and y before squaring.  If either is outside half of
 * the floating point full scale range, both are rescaled.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10,+10     30000       1.2e-7      3.4e-8
 */


/*
Cephes Math Library Release 2.1:  January, 1989
Copyright 1984, 1987, 1989 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


/*
typedef struct
	{
	float r;
	float i;
	}cmplxf;
*/
/* square root of max and min numbers */
#define SMAX  1.3043817825332782216E+19
#define SMIN  7.6664670834168704053E-20
#define PREC 12
#define MAXEXPF 128


#define SMAXT (2.0f * SMAX)
#define SMINT (0.5f * SMIN)

float cabsf( z )
register cmplxf *z;
{
float x, y, b, re, im;
int ex, ey, e;

re = fabsf( z->r );
im = fabsf( z->i );

if( re == 0.0f )
	{
	return( im );
	}
if( im == 0.0f )
	{
	return( re );
	}

/* Get the exponents of the numbers */
x = frexpf( re, &ex );
y = frexpf( im, &ey );

/* Check if one number is tiny compared to the other */
e = ex - ey;
if( e > PREC )
	return( re );
if( e < -PREC )
	return( im );

/* Find approximate exponent e of the geometric mean. */
e = (ex + ey) >> 1;

/* Rescale so mean is about 1 */
x = ldexpf( re, -e );
y = ldexpf( im, -e );
		
/* Hypotenuse of the right triangle */
b = sqrtf( x * x  +  y * y );

/* Compute the exponent of the answer. */
y = frexpf( b, &ey );
ey = e + ey;

/* Check it for overflow and underflow. */
if( ey > MAXEXPF )
	{
	mtherr( "cabsf", OVERFLOW );
	return( MAXNUMF );
	}
if( ey < -MAXEXPF )
	return(0.0f);

/* Undo the scaling */
b = ldexpf( b, e );
return( b );
}
/*							csqrtf()
 *
 *	Complex square root
 *
 *
 *
 * SYNOPSIS:
 *
 * void csqrtf();
 * cmplxf z, w;
 *
 * csqrtf( &z, &w );
 *
 *
 *
 * DESCRIPTION:
 *
 *
 * If z = x + iy,  r = |z|, then
 *
 *                       1/2
 * Im w  =  [ (r - x)/2 ]   ,
 *
 * Re w  =  y / 2 Im w.
 *
 *
 * Note that -w is also a square root of z.  The solution
 * reported is always in the upper half plane.
 *
 * Because of the potential for cancellation error in r - x,
 * the result is sharpened by doing a Heron iteration
 * (see sqrt.c) in complex arithmetic.
 *
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      -10,+10    100000       1.8e-7       4.2e-8
 *
 */


void csqrtf( z, w )
cmplxf *z, *w;
{
cmplxf q, s;
float x, y, r, t;

x = z->r;
y = z->i;

if( y == 0.0f )
	{
	if( x < 0.0f )
		{
		w->r = 0.0f;
		w->i = sqrtf(-x);
		return;
		}
	else
		{
		w->r = sqrtf(x);
		w->i = 0.0f;
		return;
		}
	}

if( x == 0.0f )
	{
	r = fabsf(y);
	r = sqrtf(0.5f*r);
	if( y > 0 )
		w->r = r;
	else
		w->r = -r;
	w->i = r;
	return;
	}

/* Approximate  sqrt(x^2+y^2) - x  =  y^2/2x - y^4/24x^3 + ... .
 * The relative error in the first term is approximately y^2/12x^2 .
 */
if( (fabsf(y) < fabsf(0.015f*x))
   && (x > 0) )
	{
	t = 0.25f*y*(y/x);
	}
else
	{
	r = cabsf(z);
	t = 0.5f*(r - x);
	}

r = sqrtf(t);
q.i = r;
q.r = 0.5f*y/r;

/* Heron iteration in complex arithmetic:
 * q = (q + z/q)/2
 */
cdivf( &q, z, &s );
caddf( &q, &s, w );
w->r *= 0.5f;
w->i *= 0.5f;
}

