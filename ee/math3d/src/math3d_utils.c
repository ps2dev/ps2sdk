/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Dan Peori <peori@oopo.net>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

 /////////////////////////////
 // STANDARD MATH FUNCTIONS //
 /////////////////////////////

 #define PI	3.141592653589793f
 #define PI2	1.570796326794896f

 float math3d_cos(float angle) { float y, t, frac, quad;

  #define P0     0.999999999781000f
  #define P1    -0.499999993585000f
  #define P2     0.041666636258000f
  #define P3    -0.001388836139900f
  #define P4     0.000024760161340f
  #define P5    -0.000000260514950f

  // Determine the absolute value of the angle.
  if (angle < 0) { angle = -angle; }

  // Determine the quadrant of the angle.
  quad = (int)(angle/PI2); /* quadrant (0 to 3) */
  frac = (angle/PI2) - quad; /* fractional part of input */

  if (quad == 0) { t = frac * PI2; } else
  if (quad == 1) { t = (1-frac) * PI2; } else
  if (quad == 2) { t = frac * PI2; } else { t = (frac-1) * PI2; }

  t *= t;
  y = P0 + (P1 * t) + (P2 * t * t) + (P3 * t * t * t) + (P4 * t * t * t * t) + (P5 * t * t * t * t * t);

  if ((quad == 2) | (quad == 1)) y=-y; /* correct sign */
  return(y);

 }

 float math3d_sin(float angle) {

  // Return the cosine of the angle.
  return math3d_cos(PI2 - angle);

 }

 float math3d_tan(float angle) {

  // Return the tangent of the angle.
  return math3d_sin(angle) / math3d_cos(angle);

 }

 float math3d_sqrt(float number) { float guess = 1.00f;
  int loop0;

  // Guess at the answer using the previous guess as a hint.
  for (loop0=0;loop0<20;loop0++) { guess = ((number / guess) + guess) / 2; }

  // Return the guess.
  return guess;

 }
