/*   mtst.c
 Consistency tests for math functions.
 To get strict rounding rules on a 386 or 68000 computer,
 define SETPREC to 1.

 With NTRIALS=10000, the following are typical results for
 IEEE double precision arithmetic.

Consistency test of math functions.
Max and rms relative errors for 10000 random arguments.
x =   cbrt(   cube(x) ):  max = 0.00E+00   rms = 0.00E+00
x =   atan(    tan(x) ):  max = 2.21E-16   rms = 3.27E-17
x =    sin(   asin(x) ):  max = 2.13E-16   rms = 2.95E-17
x =   sqrt( square(x) ):  max = 0.00E+00   rms = 0.00E+00
x =    log(    exp(x) ):  max = 1.11E-16 A rms = 4.35E-18 A
x =   tanh(  atanh(x) ):  max = 2.22E-16   rms = 2.43E-17
x =  asinh(   sinh(x) ):  max = 2.05E-16   rms = 3.49E-18
x =  acosh(   cosh(x) ):  max = 1.43E-15 A rms = 1.54E-17 A
x =  log10(  exp10(x) ):  max = 5.55E-17 A rms = 1.27E-18 A
x = pow( pow(x,a),1/a ):  max = 7.60E-14   rms = 1.05E-15
x =    cos(   acos(x) ):  max = 2.22E-16 A rms = 6.90E-17 A
*/

/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
*/


#include "mconf.h"

#define SETPREC 1
#define NTRIALS 10000
#define STRTST 0

#define WTRIALS (NTRIALS/5)

#ifndef ANSIC
float sqrtf(), cbrtf(), expf(), logf();
float exp10f(), log10f(), tanf(), atanf();
float sinf(), asinf(), cosf(), acosf(), powf();
float tanhf(), atanhf(), sinhf(), asinhf(), coshf(), acoshf();
#endif

#define fabsf(x) ((x) < 0 ? -(x) : (x))

#if SETPREC
int sprec();
#endif

int drand();
void exit();
int printf();


/* Provide inverses for square root and cube root: */
#ifdef ANSIC
float square(float x)
#else
float square(x)
float x;
#endif
{
return( x * x );
}

#ifdef ANSIC
float cube(float x)
#else
float cube(x)
float x;
#endif
{
return( x * x * x );
}

/* lookup table for each function */
struct oneargument
  {
    char *nam1;		/* the function */
#if ANSIC
    float (*name) (float);
#else
    float (*name) ();
#endif
    char *nam2;		/* its inverse  */
#if ANSIC
    float (*inv )(float);
#else
    float (*inv )();
#endif
    int tstyp;		/* type code of the function */
    long ctrl;		/* relative error flag */
    float arg1w;	/* width of domain for 1st arg */
    float arg1l;	/* lower bound domain 1st arg */
    long arg1f;		/* flags, e.g. integer arg */
  };

struct twoarguments
  {
    char *nam1;		/* the function */
#if ANSIC
    float (*name) (float, float);
#else
    float (*name) ();
#endif
    char *nam2;		/* its inverse  */
#if ANSIC
    float (*inv )(float, float);
#else
    float (*inv )();
#endif
    int tstyp;		/* type code of the function */
    long ctrl;		/* relative error flag */
    float arg1w;	/* width of domain for 1st arg */
    float arg1l;	/* lower bound domain 1st arg */
    long arg1f;		/* flags, e.g. integer arg */
    float arg2w;	/* same info for args 2, 3, 4 */
    float arg2l;
    long arg2f;
  };

/* def.ctrl bits: */
#define RELERR 1

/* fundef.tstyp  test types: */
#define POWER 1 
#define ELLIP 2 
#define GAMMA 3
#define WRONK1 4
#define WRONK2 5
#define WRONK3 6

/* fundef.argNf  argument flag bits: */
#define INT 2
#define EXPSCAL 4

extern float MINLOGF;
extern float MAXLOGF;
extern float PIF;
extern float PIO2F;
/*
define MINLOGF -170.0
define MAXLOGF +170.0
define PIF 3.14159265358979323846
define PIO2F 1.570796326794896619
*/

#define N1TESTS 10
struct oneargument defs1arg[N1TESTS] = {
{"  cube",    cube,   "  cbrt",   cbrtf, 0, 1, 2002.0, -1001.0, 0},
{"   tan",    tanf,   "  atan",   atanf, 0, 1,    0.0,     0.0,  0},
{"  asin",   asinf,   "   sin",    sinf, 0, 1,   2.0,      -1.0,  0},
{"square",  square,   "  sqrt",   sqrtf, 0, 1, 87.0,    -43.5, EXPSCAL},
{"   exp",    expf,   "   log",    logf, 0, 0, 174.0,    -87.0,  0},
{" atanh",  atanhf,   "  tanh",   tanhf, 0, 1,    2.0,    -1.0,  0},
{"  sinh",   sinhf,   " asinh",  asinhf, 0, 1, 174.0,   0.0,  0},
{"  cosh",   coshf,   " acosh",  acoshf, 0, 0, 174.0,      0.0,  0},
{" exp10",  exp10f,   " log10",  log10f, 0, 0,  76.0,    -38.0,  0},
{"  acos",   acosf,   "   cos",    cosf, 0, 0,   2.0,      -1.0,  0},
};

#define N2TESTS 1
struct twoarguments defs2arg[N2TESTS] = {
{"pow",       powf,      "pow",    powf, POWER, 1, 20.0, 0.01,   0,
40.0, -20.0, 0},
};

static char *headrs[] = {
"x = %s( %s(x) ): ",
"x = %s( %s(x,a),1/a ): ",	/* power */
"Legendre %s, %s: ",		/* ellip */
"%s(x) = log(%s(x)): ",		/* gamma */
"Wronksian of %s, %s: ",
"Wronksian of %s, %s: ",
"Wronksian of %s, %s: "
};
 
static float yy1;
static float y2;
static float y3;
static float y4;
static float a;
static float x;
static float y;
static float z;
static float  e;
static float max;
static float rmsa;
static float rms;
static float ave;
static double doublea;

int main()
{
#if ANSIC
float (*fun1 )(float);
float (*ifun1 )(float);
float (*fun2 )(float, float);
float (*ifun2 )(float, float);
#else
float (*fun1 )();
float (*ifun1 )();
float (*fun2 )();
float (*ifun2 )();
#endif
char *nam1, *nam2;
int tstyp, nargs;
long arg1f, arg2f, ctrl;
float arg1l, arg2l, arg1w, arg2w;
int i, k, itst, ntsts, iargs;
int m, ntr;

#if SETPREC
sprec();  /* set coprocessor precision */
#endif
ntr = NTRIALS;
printf( "Consistency test of math functions.\n" );
printf( "Max and rms relative errors for %d random arguments.\n",
	ntr );

/* Initialize machine dependent parameters: */

defs1arg[1].arg1w = PIF;
defs1arg[1].arg1l = -PIF/2.0;

/* Microsoft C has trouble with denormal numbers. */
#if 0
defs[3].arg1w = MAXLOGF;
defs[3].arg1l = -MAXLOGF/2.0F;
defs[4].arg1w = 2*MAXLOGF;
defs[4].arg1l = -MAXLOGF;
#endif
defs1arg[6].arg1w = 2.0F*MAXLOGF;
defs1arg[6].arg1l = -MAXLOGF;
defs1arg[7].arg1w = MAXLOGF;
defs1arg[7].arg1l = 0.0;


/* Outer outer loop, on number of function arguments.  */
for( iargs=1; iargs <=2; iargs++)
  {
    switch (iargs)
      {
      case 2:
	ntsts = N2TESTS;
	break;
      default:
	ntsts = N1TESTS;
      }
/* Outer loop, on the test number: */
for( itst=STRTST; itst<ntsts; itst++ )
{
  switch (iargs)
    {
    case 2:
      tstyp = defs2arg[itst].tstyp;
      fun2 = defs2arg[itst].name;
      ifun2 = defs2arg[itst].inv;
      nam1 = defs2arg[itst].nam1;
      nam2 = defs2arg[itst].nam2;
      arg1w = defs2arg[itst].arg1w;
      arg1l = defs2arg[itst].arg1l;
      arg1f = defs2arg[itst].arg1f;
      arg2w = defs2arg[itst].arg2w;
      arg2l = defs2arg[itst].arg2l;
      arg2f = defs2arg[itst].arg2f;
      ctrl = defs2arg[itst].ctrl;
      nargs = 2;
      break;
    default:
      tstyp = defs1arg[itst].tstyp;
      fun1 = defs1arg[itst].name;
      ifun1 = defs1arg[itst].inv;
      nam1 = defs1arg[itst].nam1;
      nam2 = defs1arg[itst].nam2;
      arg1w = defs1arg[itst].arg1w;
      arg1l = defs1arg[itst].arg1l;
      arg1f = defs1arg[itst].arg1f;
      ctrl = defs1arg[itst].ctrl;
      nargs = 1;
    }
k = 0;
m = 0;
max = 0.0F;
rmsa = 0.0F;
ave = 0.0F;

/* Absolute error criterion starts with gamma function
 * (put all such at end of table)
 */
if( tstyp == GAMMA )
	printf( "Absolute error criterion (but relative if >1):\n" );

/* Smaller number of trials for Wronksians
 * (put them at end of list)
 */
if( tstyp == WRONK1 )
	{
	ntr = WTRIALS;
	printf( "Absolute error and only %d trials:\n", ntr );
	}

printf( headrs[tstyp], nam2, nam1 );

for( i=0; i<ntr; i++ )
{
m++;

/* make random number(s) in desired range(s) */
switch( nargs )
{

default:
goto illegn;
	
case 2:
drand( &doublea );
a = arg2w *  ( doublea - 1.0 )  +  arg2l;
if( arg2f & EXPSCAL )
	{
	a = expf(a);
	drand( &doublea );
	y2 = doublea;
	a -= 1.0e-13 * a * y2;
	}
if( arg2f & INT )
	{
	k = a + 0.25;
	a = k;
	}

case 1:
drand( &doublea );
x = arg1w *  ( doublea - 1.0 )  +  arg1l;
if( arg1f & EXPSCAL )
	{
	x = expf(x);
	drand( &doublea );
	a = doublea;
	x += 1.0e-13F * x * a;
	}
}


/* compute function under test */
switch( nargs )
	{
	case 2:
	if( arg2f & INT )
		{
		switch( tstyp )
			{
			case WRONK1:
			yy1 = (*fun2)( k, x ); /* jn */
			y2 = (*fun2)( k+1, x );
			y3 = (*ifun2)( k, x ); /* yn */
			y4 = (*ifun2)( k+1, x );	
			break;

			case WRONK2:
			yy1 = (*fun2)( a, x ); /* iv */
			y2 = (*fun2)( a+1.0F, x );
			y3 = (*ifun2)( k, x ); /* kn */	
			y4 = (*ifun2)( k+1, x );	
			break;

			default:
			z = (*fun2)( k, x );
			y = (*ifun2)( k, z );
			}
		}
	else
		{
		if( tstyp == POWER )
			{
			z = (*fun2)( x, a );
			y = (*ifun2)( z, 1.0F/a );
			}
		else
			{
			z = (*fun2)( a, x );
			y = (*ifun2)( a, z );
			}
		}
	break;

	case 1:
	switch( tstyp )
		{
		case ELLIP:
		yy1 = ( *(fun1) )(x);
		y2 = ( *(fun1) )(1.0F-x);
		y3 = ( *(ifun1) )(x);
		y4 = ( *(ifun1) )(1.0F-x);
		break;

#if 0
		case GAMMA:
		y = lgam(x);
		x = log( gamma(x) );
		break;
#endif
		default:
		z = ( *(fun1) )(x);
		y = ( *(ifun1) )(z);
		}
	break;
	

	default:
illegn:
	printf( "Illegal nargs= %d", nargs );
	exit(1);
	}	

switch( tstyp )
	{
	case WRONK1:
	e = (y2*y3 - yy1*y4) - 2.0F/(PIF*x); /* Jn, Yn */
	break;

	case WRONK2:
	e = (y2*y3 + yy1*y4) - 1.0F/x; /* In, Kn */
	break;
	
	case ELLIP:
	e = (yy1-y3)*y4 + y3*y2 - PIO2F;
	break;

	default:
	e = y - x;
	break;
	}

if( ctrl & RELERR )
	e /= x;
else
	{
	if( fabsf(x) > 1.0F )
		e /= x;
	}

ave += e;
/* absolute value of error */
if( e < 0 )
	e = -e;

/* peak detect the error */
if( e > max )
	{
	max = e;

	if( e > 1.0e-3F )
		{
		printf("x %.6E z %.6E y %.6E max %.4E\n",
		 x, z, y, max);
		if( tstyp == POWER )
			{
			printf( "a %.6E\n", a );
			}
		if( tstyp >= WRONK1 )
			{
		printf( "yy1 %.4E y2 %.4E y3 %.4E y4 %.4E k %d x %.4E\n",
		 yy1, y2, y3, y4, k, x );
			}
		}

/*
	printf("%.8E %.8E %.4E %6ld \n", x, y, max, n);
	printf("%d %.8E %.8E %.4E %6ld \n", k, x, y, max, n);
	printf("%.6E %.6E %.6E %.4E %6ld \n", a, x, y, max, n);
	printf("%.6E %.6E %.6E %.6E %.4E %6ld \n", a, b, x, y, max, n);
	printf("%.4E %.4E %.4E %.4E %.4E %.4E %6ld \n",
		a, b, c, x, y, max, n);
*/
	}

/* accumulate rms error	*/
e *= 1.0e7F;	/* adjust range */
rmsa += e * e;	/* accumulate the square of the error */
}

/* report after NTRIALS trials */
rms = 1.0e-7F * sqrtf( rmsa/m );
if(ctrl & RELERR)
	printf(" max = %.2E   rms = %.2E\n", max, rms );
else
	printf(" max = %.2E A rms = %.2E A\n", max, rms );
} /* loop on itst */
} /* loop on number of args */
return 0;
}
