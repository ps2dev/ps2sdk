
#ifndef _FLOATLIB_H_
#define _FLOATLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float r;     /* real part */
  float i;     /* imaginary part */
 }cmplxf;

float acosf( float );
float acoshf( float );
void airyf( float, float *, float *, float *, float * );
float asinf( float );
float asinhf( float );
float atanf( float );
float atanhf( float );
float atan2f( float, float );
float bdtrcf( int, int, float );
float bdtrf( int, int, float );
float bdtrif( int, int, float );
float betaf( float, float );
float cbrtf( float );
float chbevlf( float, float *, int );
float chdtrcf(float, float);
float chdtrf(float, float);
float chdtrif( float, float );
float cabsf(cmplxf *);
void caddf( cmplxf *, cmplxf *, cmplxf * );
void cacosf( cmplxf *, cmplxf * );
void casinf( cmplxf *, cmplxf * );
void catanf( cmplxf *, cmplxf * );
void ccosf( cmplxf *, cmplxf * );
void ccotf( cmplxf *, cmplxf * );
void cdivf( cmplxf *, cmplxf *, cmplxf * );
float ceilf( float );
void cexpf( cmplxf *, cmplxf * );
void clogf( cmplxf *, cmplxf * );
void cmovf( cmplxf *, cmplxf * );
void cmulf( cmplxf *, cmplxf *, cmplxf * );
void cnegf( cmplxf * );
float cosdgf( float );
float cosf(float);
float coshf(float);
float cot( float );
float cotdgf( float );
void csubf( cmplxf *, cmplxf *, cmplxf * );
void csinf( cmplxf *, cmplxf * );
void csqrtf( cmplxf *, cmplxf * );
void ctanf( cmplxf *, cmplxf * );
float dawsnf(float);
float ellief( float, float );
float ellikf( float, float );
float ellpef(float);
float ellpjf( float, float, float *, float *, float *, float * );
float ellpkf(float);
float expf(float);
float expf2(float);
float exp10f(float);
float expnf( int, float );
float fabsf( float );
float facf( int );
float fdtrcf( int, int, float );
float fdtrf( int, int, int );
float fdtrif( int, int, int );
float floorf(float);
void fresnlf( float, float *, float * );
float frexpf(float, int *);
float gammaf(float);
float gdtrf( float, float, float );
float gdtrcf( float, float, float );
float hyp2f1f( float, float, float, float );
float hyp2f0f(float, float, float, int, float *);
float hypergf( float, float, float );
float i0f( float );
float i0ef( float );
float i1f( float );
float i1ef( float );
float igamcf(float, float);
float igamf(float, float);
float igamif(float, float);
float incbetf(float, float, float);
float incbif( float, float, float );
float ivf( float, float );
float j0f( float );
float j1f( float );
float jnf( int, float );
float jvf( float, float );
float k0f( float );
float k1f( float );
float knf( int, float );
float ldexpf(float, int);
float lgamf(float);
float logf( float );
float log2f( float );
float log10f( float );
float nbdtrcf( int, int, float );
float nbdtrf( int, int, float );
float ndtrf( float );
float ndtrif( float );
float onef2f( float, float, float, float, float * );
float pdtrcf( int, float );
float pdtrf( int, float );
float pdtrif( int, float );
float polevlf( float, float *, int );
float p1evlf( float, float *, int );
float powf(float, float);
float powif(float, int);
float psif( float );
float rgammaf( float );
float rsqrtf( float, float );
int shichif( float, float *, float * );
int sicif( float, float *, float * );
float sindgf( float );
float sinf( float );
float sinhf( float );
float spencef( float );
float sqrtf( float );
float stdtrf( int, float );
float struvef( float, float );
float tandgf( float );
float tanf( float );
float tanhf( float );
float threef0f( float, float, float, float, float * );
float ynf( int, float );
float yvf( float, float );
float zetacf( float );
float zetaf( float, float );

#ifdef __cplusplus
}
#endif

#endif
