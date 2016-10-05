#ifndef __MATH_H__
#define __MATH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Not sure if this is right. */
#ifndef HUGE_VAL
#define HUGE_VAL ((double)1.0/0.0)
#endif

/* Defined */
extern float acosf(float);
extern float asinf(float);
extern float atanf(float);
extern float atan2f(float,float);
extern float ceilf(float);
extern float coshf(float);
extern float expf(float);
extern float floorf(float);
extern float frexpf(float, int*);
extern float ldexpf(float, int);
extern float logf(float);
extern float log10f(float);
extern float powf(float, float);
extern float sinhf(float);
extern float tanf(float);
extern float tanhf(float);

/* Undefined */
extern float fmodf(float, float);
extern float modff(float, float*);
extern int finitef(float);

extern double acos (double);
extern double asin (double);
extern double atan (double);
extern double atan2 (double, double);
extern double cos (double);
extern double sin (double);
extern double tan (double);
extern double cosh (double);
extern double sinh (double);
extern double tanh (double);
extern double exp (double);
extern double frexp (double, int *);
extern double ldexp (double, int);
extern double log (double);
extern double log10 (double);
extern double modf (double, double *);
extern double pow (double, double);
extern double sqrt (double);
extern double ceil (double);
extern double fabs (double);
extern double floor (double);
extern double fmod (double, double);

#ifdef __cplusplus
}
#endif

#endif /*__MATH_H__*/

