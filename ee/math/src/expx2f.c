/*							expx2f.c
 *
 *	Exponential of squared argument
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, expx2f();
 *
 * y = expx2f( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Computes y = exp(x*x) while suppressing error amplification
 * that would ordinarily arise from the inexactness of the argument x*x.
 * 
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic    domain     # trials      peak       rms
 *   IEEE      -9.4, 9.4      10^7       1.7e-7     4.7e-8
 *
 */

/*
Cephes Math Library Release 2.9:  June, 2000
Copyright 2000 by Stephen L. Moshier
*/

extern float floorf (float);
extern float expf (float);
extern float MAXLOGF;
extern float MAXNUMF;

float expx2f (float x)
{
  float u, u1, m;

  if (x < 0)
    x = -x;

  /* Represent x as an exact multiple of 1/32 plus a residual.  */
  m = .03125f * floorf(32.0f * x + 0.5f);
  x -= m;
  /* x**2 = m**2 + 2mf + f**2 */
  u = m * m;
  u1 = 2 * m * x  +  x * x;

  if ((u+u1) > MAXLOGF)
    return (MAXNUMF);

  /* u is exact, u1 is small.  */
  u = expf(u) * expf(u1);
  return(u);
}


