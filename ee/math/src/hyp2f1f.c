/*							hyp2f1f.c
 *
 *	Gauss hypergeometric function   F
 *	                               2 1
 *
 *
 * SYNOPSIS:
 *
 * float a, b, c, x, y, hyp2f1f();
 *
 * y = hyp2f1f( a, b, c, x );
 *
 *
 * DESCRIPTION:
 *
 *
 *  hyp2f1( a, b, c, x )  =   F ( a, b; c; x )
 *                           2 1
 *
 *           inf.
 *            -   a(a+1)...(a+k) b(b+1)...(b+k)   k+1
 *   =  1 +   >   -----------------------------  x   .
 *            -         c(c+1)...(c+k) (k+1)!
 *          k = 0
 *
 *  Cases addressed are
 *	Tests and escapes for negative integer a, b, or c
 *	Linear transformation if c - a or c - b negative integer
 *	Special case c = a or c = b
 *	Linear transformation for  x near +1
 *	Transformation for x < -0.5
 *	Psi function expansion if x > 0.5 and c - a - b integer
 *      Conditionally, a recurrence on c to make c-a-b > 0
 *
 * |x| > 1 is rejected.
 *
 * The parameters a, b, c are considered to be integer
 * valued if they are within 1.0e-6 of the nearest integer.
 *
 * ACCURACY:
 *
 *                      Relative error (-1 < x < 1):
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,3         30000       5.8e-4      4.3e-6
 */

/*							hyp2f1	*/


/*
Cephes Math Library Release 2.2:  November, 1992
Copyright 1984, 1987, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"

#define EPS 1.0e-5
#define EPS2 1.0e-5
#define ETHRESH 1.0e-5

extern float MAXNUMF, MACHEPF;

#define fabsf(x) ( (x) < 0 ? -(x) : (x) )

#ifdef ANSIC
float powf(float, float);
static float hys2f1f(float, float, float, float, float *);
static float hyt2f1f(float, float, float, float, float *);
float gammaf(float), logf(float), expf(float), psif(float);
float floorf(float);
#else
float powf(), gammaf(), logf(), expf(), psif();
float floorf();
static float hyt2f1f(), hys2f1f();
#endif

#define roundf(x) (floorf((x)+(float )0.5))




#ifdef ANSIC
float hyp2f1f( float aa, float bb, float cc, float xx )
#else
float hyp2f1f( aa, bb, cc, xx )
double aa, bb, cc, xx;
#endif
{
float a, b, c, x;
float d, d1, d2, e;
float p, q, r, s, y, ax;
float ia, ib, ic, id, err;
int flag, i, aid;

a = aa;
b = bb;
c = cc;
x = xx;
err = 0.0;
ax = fabsf(x);
s = 1.0 - x;
flag = 0;
ia = roundf(a); /* nearest integer to a */
ib = roundf(b);

if( a <= 0 )
	{
	if( fabsf(a-ia) < EPS )		/* a is a negative integer */
		flag |= 1;
	}

if( b <= 0 )
	{
	if( fabsf(b-ib) < EPS )		/* b is a negative integer */
		flag |= 2;
	}

if( ax < 1.0 )
	{
	if( fabsf(b-c) < EPS )		/* b = c */
		{
		y = powf( s, -a );	/* s to the -a power */
		goto hypdon;
		}
	if( fabsf(a-c) < EPS )		/* a = c */
		{
		y = powf( s, -b );	/* s to the -b power */
		goto hypdon;
		}
	}



if( c <= 0.0 )
	{
	ic = roundf(c); 	/* nearest integer to c */
	if( fabsf(c-ic) < EPS )		/* c is a negative integer */
		{
		/* check if termination before explosion */
		if( (flag & 1) && (ia > ic) )
			goto hypok;
		if( (flag & 2) && (ib > ic) )
			goto hypok;
		goto hypdiv;
		}
	}

if( flag )			/* function is a polynomial */
	goto hypok;

if( ax > 1.0 )			/* series diverges	*/
	goto hypdiv;

p = c - a;
ia = roundf(p);
if( (ia <= 0.0) && (fabsf(p-ia) < EPS) )	/* negative int c - a */
	flag |= 4;

r = c - b;
ib = roundf(r); /* nearest integer to r */
if( (ib <= 0.0) && (fabsf(r-ib) < EPS) )	/* negative int c - b */
	flag |= 8;

d = c - a - b;
id = roundf(d); /* nearest integer to d */
q = fabsf(d-id);

if( fabsf(ax-1.0) < EPS )			/* |x| == 1.0	*/
	{
	if( x > 0.0 )
		{
		if( flag & 12 ) /* negative int c-a or c-b */
			{
			if( d >= 0.0 )
				goto hypf;
			else
				goto hypdiv;
			}
		if( d <= 0.0 )
			goto hypdiv;
		y = gammaf(c)*gammaf(d)/(gammaf(p)*gammaf(r));
		goto hypdon;
		}

	if( d <= -1.0 )
		goto hypdiv;
	}

/* Conditionally make d > 0 by recurrence on c
 * AMS55 #15.2.27
 */
if( d < 0.0 )
	{
/* Try the power series first */
	y = hyt2f1f( a, b, c, x, &err );
	if( err < ETHRESH )
		goto hypdon;
/* Apply the recurrence if power series fails */
	err = 0.0;
	aid = 2 - id;
	e = c + aid;
	d2 = hyp2f1f(a,b,e,x);
	d1 = hyp2f1f(a,b,e+1.0,x);
	q = a + b + 1.0;
	for( i=0; i<aid; i++ )
		{
		r = e - 1.0;
		y = (e*(r-(2.0*e-q)*x)*d2 + (e-a)*(e-b)*x*d1)/(e*r*s);
		e = r;
		d1 = d2;
		d2 = y;
		}
	goto hypdon;
	}


if( flag & 12 )
	goto hypf; /* negative integer c-a or c-b */

hypok:
y = hyt2f1f( a, b, c, x, &err );

hypdon:
if( err > ETHRESH )
	{
	mtherr( "hyp2f1", PLOSS );
/*	printf( "Estimated err = %.2e\n", err );*/
	}
return(y);

/* The transformation for c-a or c-b negative integer
 * AMS55 #15.3.3
 */
hypf:
y = powf( s, d ) * hys2f1f( c-a, c-b, c, x, &err );
goto hypdon;

/* The alarm exit */
hypdiv:
mtherr( "hyp2f1f", OVERFLOW );
return( MAXNUMF );
}




/* Apply transformations for |x| near 1
 * then call the power series
 */
#ifdef ANSIC
static float hyt2f1f( float aa, float bb, float cc, float xx, float *loss )
#else
static float hyt2f1f( aa, bb, cc, xx, loss )
double aa, bb, cc, xx;
float *loss;
#endif
{
float a, b, c, x;
float p, q, r, s, t, y, d, err, err1;
float ax, id, d1, d2, e, y1;
int i, aid;

a = aa;
b = bb;
c = cc;
x = xx;
err = 0.0;
s = 1.0 - x;
if( x < -0.5 )
	{
	if( b > a )
		y = powf( s, -a ) * hys2f1f( a, c-b, c, -x/s, &err );

	else
		y = powf( s, -b ) * hys2f1f( c-a, b, c, -x/s, &err );

	goto done;
	}



d = c - a - b;
id = roundf(d);	/* nearest integer to d */

if( x > 0.8 )
{

if( fabsf(d-id) > EPS2 ) /* test for integer c-a-b */
	{
/* Try the power series first */
	y = hys2f1f( a, b, c, x, &err );
	if( err < ETHRESH )
		goto done;
/* If power series fails, then apply AMS55 #15.3.6 */
	q = hys2f1f( a, b, 1.0-d, s, &err );	
	q *= gammaf(d) /(gammaf(c-a) * gammaf(c-b));
	r = powf(s,d) * hys2f1f( c-a, c-b, d+1.0, s, &err1 );
	r *= gammaf(-d)/(gammaf(a) * gammaf(b));
	y = q + r;

	q = fabsf(q); /* estimate cancellation error */
	r = fabsf(r);
	if( q > r )
		r = q;
	err += err1 + (MACHEPF*r)/y;

	y *= gammaf(c);
	goto done;
	}	
else
	{
/* Psi function expansion, AMS55 #15.3.10, #15.3.11, #15.3.12 */
	if( id >= 0.0 )
		{
		e = d;
		d1 = d;
		d2 = 0.0;
		aid = id;
		}
	else
		{
		e = -d;
		d1 = 0.0;
		d2 = d;
		aid = -id;
		}

	ax = logf(s);

	/* sum for t = 0 */
	y = psif(1.0) + psif(1.0+e) - psif(a+d1) - psif(b+d1) - ax;
	y /= gammaf(e+1.0);

	p = (a+d1) * (b+d1) * s / gammaf(e+2.0);	/* Poch for t=1 */
	t = 1.0;
	do
		{
		r = psif(1.0+t) + psif(1.0+t+e) - psif(a+t+d1)
			- psif(b+t+d1) - ax;
		q = p * r;
		y += q;
		p *= s * (a+t+d1) / (t+1.0);
		p *= (b+t+d1) / (t+1.0+e);
		t += 1.0;
		}
	while( fabsf(q/y) > EPS );


	if( id == 0.0 )
		{
		y *= gammaf(c)/(gammaf(a)*gammaf(b));
		goto psidon;
		}

	y1 = 1.0;

	if( aid == 1 )
		goto nosum;

	t = 0.0;
	p = 1.0;
	for( i=1; i<aid; i++ )
		{
		r = 1.0-e+t;
		p *= s * (a+t+d2) * (b+t+d2) / r;
		t += 1.0;
		p /= t;
		y1 += p;
		}


nosum:
	p = gammaf(c);
	y1 *= gammaf(e) * p / (gammaf(a+d1) * gammaf(b+d1));
	y *= p / (gammaf(a+d2) * gammaf(b+d2));
	if( (aid & 1) != 0 )
		y = -y;

	q = powf( s, id );	/* s to the id power */
	if( id > 0.0 )
		y *= q;
	else
		y1 *= q;

	y += y1;
psidon:
	goto done;
	}
}


/* Use defining power series if no special cases */
y = hys2f1f( a, b, c, x, &err );

done:
*loss = err;
return(y);
}





/* Defining power series expansion of Gauss hypergeometric function */

#if 1
#ifdef ANSIC
static float hys2f1f( float aa, float bb, float cc, float xx, float *loss )
#else
static float hys2f1f( aa, bb, cc, xx, loss )
double aa, bb, cc, xx;
float *loss;
#endif
{
int i;
float a, b, c, x;
float f, g, h, k, m, s, u, umax;


a = aa;
b = bb;
c = cc;
x = xx;
i = 0;
umax = 0.0;
f = a;
g = b;
h = c;
k = 0.0;
s = 1.0;
u = 1.0;

do
	{
	if( fabsf(h) < EPS )
		return( MAXNUMF );
	m = k + 1.0;
	u = u * ((f+k) * (g+k) * x / ((h+k) * m));
	s += u;
	k = fabsf(u);  /* remember largest term summed */
	if( k > umax )
		umax = k;
	k = m;
	if( ++i > 10000 ) /* should never happen */
		{
		*loss = 1.0;
		return(s);
		}
	}
while( fabsf(u/s) > MACHEPF );

/* return estimated relative error */
*loss = (MACHEPF*umax)/fabsf(s) + (MACHEPF*i);

return(s);
}


#else /* 0 */

extern double MACHEP;

#ifdef ANSIC
static float hys2f1f( float aa, float bb, float cc, float xx, float *loss )
#else
static float hys2f1f( aa, bb, cc, xx, loss )
double aa, bb, cc, xx;
float *loss;
#endif
{
int i;
double a, b, c, x;
double f, g, h, k, m, s, u, umax;


a = aa;
b = bb;
c = cc;
x = xx;
i = 0;
umax = 0.0;
f = a;
g = b;
h = c;
k = 0.0;
s = 1.0;
u = 1.0;

do
	{
	if( fabsf(h) < EPS )
		{
		*loss = 1.0;
		return( MAXNUMF );
		}
	m = k + 1.0;
	u = u * ((f+k) * (g+k) * x / ((h+k) * m));
	s += u;
	k = fabsf(u);  /* remember largest term summed */
	if( k > umax )
		umax = k;
	k = m;
	if( ++i > 10000 ) /* should never happen */
		{
		*loss = 1.0;
		return(s);
		}
	}
while( fabsf(u/s) > MACHEP );

/* return estimated relative error */
*loss = (MACHEPF*umax)/fabsf(s) + (MACHEPF*i);

return(s);
}
#endif
