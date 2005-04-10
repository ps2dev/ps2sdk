/*							ceilf()
 *							floorf()
 *							frexpf()
 *							ldexpf()
 *							signbitf()
 *							isnanf()
 *							isfinitef()
 *
 *	Single precision floating point numeric utilities
 *
 *
 *
 * SYNOPSIS:
 *
 * float x, y;
 * float ceilf(), floorf(), frexpf(), ldexpf();
 * int signbit(), isnan(), isfinite();
 * int expnt, n;
 *
 * y = floorf(x);
 * y = ceilf(x);
 * y = frexpf( x, &expnt );
 * y = ldexpf( x, n );
 * n = signbit(x);
 * n = isnan(x);
 * n = isfinite(x);
 *
 *
 *
 * DESCRIPTION:
 *
 * All four routines return a single precision floating point
 * result.
 *
 * sfloor() returns the largest integer less than or equal to x.
 * It truncates toward minus infinity.
 *
 * sceil() returns the smallest integer greater than or equal
 * to x.  It truncates toward plus infinity.
 *
 * sfrexp() extracts the exponent from x.  It returns an integer
 * power of two to expnt and the significand between 0.5 and 1
 * to y.  Thus  x = y * 2**expn.
 *
 * ldexpf() multiplies x by 2**n.
 *
 * signbit(x) returns 1 if the sign bit of x is 1, else 0.
 *
 * These functions are part of the standard C run time library
 * for many but not all C compilers.  The ones supplied are
 * written in C for either DEC or IEEE arithmetic.  They should
 * be used only if your compiler library does not already have
 * them.
 *
 * The IEEE versions assume that denormal numbers are implemented
 * in the arithmetic.  Some modifications will be required if
 * the arithmetic has abrupt rather than gradual underflow.
 */


/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


#include "mconf.h"
#ifdef DEC
#undef DENORMAL
#define DENORMAL 0
#endif

#ifdef UNK
#undef UNK
#if BIGENDIAN
#define MIEEE 1
#else
#define IBMPC 1
#endif
/*
char *unkmsg = "ceil(), floor(), frexp(), ldexp() must be rewritten!\n";
*/
#endif

#define EXPMSK 0x807f
#define MEXP 255
#define NBITS 24


extern float MAXNUMF; /* (2^24 - 1) * 2^103 */
#ifdef ANSIC
float floorf(float);
#else
float floorf();
#endif

#ifdef ANSIC
float ceilf( float x )
#else
float ceilf(x)
double x;
#endif
{
float y;

#ifdef UNK
printf( "%s\n", unkmsg );
return(0.0);
#endif

y = floorf( (float )x );
if( y < x )
	y += 1.0;
return(y);
}




/* Bit clearing masks: */

static unsigned short bmask[] = {
0xffff,
0xfffe,
0xfffc,
0xfff8,
0xfff0,
0xffe0,
0xffc0,
0xff80,
0xff00,
0xfe00,
0xfc00,
0xf800,
0xf000,
0xe000,
0xc000,
0x8000,
0x0000,
};



#ifdef ANSIC
float floorf( float x )
#else
float floorf(x)
double x;
#endif
{
unsigned short *p;
union
  {
    float y;
    unsigned short i[2];
  } u;
int e;

#ifdef UNK
printf( "%s\n", unkmsg );
return(0.0);
#endif

u.y = x;
/* find the exponent (power of 2) */
#ifdef DEC
p = &u.i[0];
e = (( *p  >> 7) & 0377) - 0201;
p += 3;
#endif

#ifdef IBMPC
p = &u.i[1];
e = (( *p >> 7) & 0xff) - 0x7f;
p -= 1;
#endif

#ifdef MIEEE
p = &u.i[0];
e = (( *p >> 7) & 0xff) - 0x7f;
p += 1;
#endif

if( e < 0 )
	{
	if( u.y < 0 )
		return( -1.0 );
	else
		return( 0.0 );
	}

e = (NBITS -1) - e;
/* clean out 16 bits at a time */
while( e >= 16 )
	{
#ifdef IBMPC
	*p++ = 0;
#endif

#ifdef DEC
	*p-- = 0;
#endif

#ifdef MIEEE
	*p-- = 0;
#endif
	e -= 16;
	}

/* clear the remaining bits */
if( e > 0 )
	*p &= bmask[e];

if( (x < 0) && (u.y != x) )
	u.y -= 1.0;

return(u.y);
}



#ifdef ANSIC
float frexpf( float x, int *pw2 )
#else
float frexpf( x, pw2 )
double x;
int *pw2;
#endif
{
union
  {
    float y;
    unsigned short i[2];
  } u;
int i, k;
short *q;

u.y = x;

#ifdef UNK
printf( "%s\n", unkmsg );
return(0.0);
#endif

#ifdef IBMPC
q = &u.i[1];
#endif

#ifdef DEC
q = &u.i[0];
#endif

#ifdef MIEEE
q = &u.i[0];
#endif

/* find the exponent (power of 2) */

i  = ( *q >> 7) & 0xff;
if( i == 0 )
	{
	if( u.y == 0.0 )
		{
		*pw2 = 0;
		return(0.0);
		}
/* Number is denormal or zero */
#if DENORMAL
/* Handle denormal number. */
	do
		{
		u.y *= 2.0;
		i -= 1;
		k  = ( *q >> 7) & 0xff;
		}
	while( k == 0 );
	i = i + k;
#else
	*pw2 = 0;
	return( 0.0 );
#endif /* DENORMAL */
	}
i -= 0x7e;
*pw2 = i;
*q &= 0x807f;	/* strip all exponent bits */
*q |= 0x3f00;	/* mantissa between 0.5 and 1 */
return( u.y );
}





#ifdef ANSIC
float ldexpf( float x, int pw2 )
#else
float ldexpf( x, pw2 )
double x;
int pw2;
#endif
{
union
  {
    float y;
    unsigned short i[2];
  } u;
short *q;
int e;

#ifdef UNK
printf( "%s\n", unkmsg );
return(0.0);
#endif

u.y = x;
#ifdef DEC
q = &u.i[0];
#endif

#ifdef IBMPC
q = &u.i[1];
#endif
#ifdef MIEEE
q = &u.i[0];
#endif
while( (e = ( *q >> 7) & 0xff) == 0 )
	{
	if( u.y == (float )0.0 )
		{
		return( 0.0 );
		}
/* Input is denormal. */
	if( pw2 > 0 )
		{
		u.y *= 2.0;
		pw2 -= 1;
		}
	if( pw2 < 0 )
		{
		if( pw2 < -24 )
			return( 0.0 );
		u.y *= 0.5;
		pw2 += 1;
		}
	if( pw2 == 0 )
		return(u.y);
	}

e += pw2;

/* Handle overflow */
if( e > MEXP )
	{
	return( MAXNUMF );
	}

*q &= 0x807f;

/* Handle denormalized results */
if( e < 1 )
	{
#if DENORMAL
	if( e < -24 )
		return( 0.0 );
	*q |= 0x80; /* Set LSB of exponent. */
	/* For denormals, significant bits may be lost even
	   when dividing by 2.  Construct 2^-(1-e) so the result
	   is obtained with only one multiplication.  */
	u.y *= ldexpf(1.0f, e - 1);
	return(u.y);
#else
	return( 0.0 );
#endif
	}
*q |= (e & 0xff) << 7;
return(u.y);
}


/* Return 1 if the sign bit of x is 1, else 0.  */

int signbitf(x)
float x;
{
union
	{
	float f;
	short s[4];
	int i;
	} u;

u.f = x;

if( sizeof(int) == 4 )
	{
#ifdef IBMPC
	return( u.i < 0 );
#endif
#ifdef DEC
	return( u.s[1] < 0 );
#endif
#ifdef MIEEE
	return( u.i < 0 );
#endif
	}
else
	{
#ifdef IBMPC
	return( u.s[1] < 0 );
#endif
#ifdef DEC
	return( u.s[1] < 0 );
#endif
#ifdef MIEEE
	return( u.s[0] < 0 );
#endif
	}
}


/* Return 1 if x is a number that is Not a Number, else return 0.  */

int isnanf(x)
float x;
{
#ifdef NANS
union
	{
	float f;
	unsigned short s[2];
	unsigned int i;
	} u;

u.f = x;

if( sizeof(int) == 4 )
	{
#ifdef IBMPC
	if( ((u.i & 0x7f800000) == 0x7f800000)
	    && ((u.i & 0x007fffff) != 0) )
		return 1;
#endif
#ifdef DEC
	if( (u.s[1] & 0x7f80) == 0)
		{
		if( (u.s[1] | u.s[0]) != 0 )
			return(1);
		}
#endif
#ifdef MIEEE
	if( ((u.i & 0x7f800000) == 0x7f800000)
	    && ((u.i & 0x007fffff) != 0) )
		return 1;
#endif
	return(0);
	}
else
	{ /* size int not 4 */
#ifdef IBMPC
	if( (u.s[1] & 0x7f80) == 0x7f80)
		{
		if( ((u.s[1] & 0x007f) | u.s[0]) != 0 )
			return(1);
		}
#endif
#ifdef DEC
	if( (u.s[1] & 0x7f80) == 0)
		{
		if( (u.s[1] | u.s[0]) != 0 )
			return(1);
		}
#endif
#ifdef MIEEE
	if( (u.s[0] & 0x7f80) == 0x7f80)
		{
		if( ((u.s[0] & 0x000f) | u.s[1]) != 0 )
			return(1);
		}
#endif
	return(0);
	} /* size int not 4 */

#else
/* No NANS.  */
return(0);
#endif
}


/* Return 1 if x is not infinite and is not a NaN.  */

int isfinitef(x)
float x;
{
#ifdef INFINITIES
union
	{
	float f;
	unsigned short s[2];
	unsigned int i;
	} u;

u.f = x;

if( sizeof(int) == 4 )
	{
#ifdef IBMPC
	if( (u.i & 0x7f800000) != 0x7f800000)
		return 1;
#endif
#ifdef DEC
	if( (u.s[1] & 0x7f80) == 0)
		{
		if( (u.s[1] | u.s[0]) != 0 )
			return(1);
		}
#endif
#ifdef MIEEE
	if( (u.i & 0x7f800000) != 0x7f800000)
		return 1;
#endif
	return(0);
	}
else
	{
#ifdef IBMPC
	if( (u.s[1] & 0x7f80) != 0x7f80)
		return 1;
#endif
#ifdef DEC
	if( (u.s[1] & 0x7f80) == 0)
		{
		if( (u.s[1] | u.s[0]) != 0 )
			return(1);
		}
#endif
#ifdef MIEEE
	if( (u.s[0] & 0x7f80) != 0x7f80)
		return 1;
#endif
	return(0);
	}
#else
/* No INFINITY.  */
return(1);
#endif
}
