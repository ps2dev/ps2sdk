/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * VU library functions.
 */

#ifndef __LIBVUX_H__
#define __LIBVUX_H__

/*

y
|
|     z
|   /
| /
0-------x

row major
*/


#define VU_LIGHT_TYPE_FLAT			0x10

/** Float Matrix (4x4) */
typedef struct
{

	float	m[4][4];

}VU_MATRIX __attribute__((aligned(16)));

/** 3D Float Vector (128 bit) */
typedef struct
{
	float	x;
	float	y;
	float	z;
	float	w;

}VU_VECTOR __attribute__((aligned(16)));

/** 2D Screen xy and z (64 bit) */
typedef struct
{
	/** format is same as gs 0:12:4 */
	unsigned short	x; 
	/** format is same as gs 0:12:4 */
	unsigned short	y;
	unsigned int	z;

}VU_SXYZ;

/** 2D Screen xy and z with fog coefficient (64 bit) */
typedef struct
{
	/** format is same as gs 0:12:4 */
	unsigned short	x;
	/** format is same as gs 0:12:4 */
	unsigned short	y;
	unsigned int	z:24;
	unsigned char	f;

}VU_SXYZF;

/** color vector (64 bit) */
typedef struct
{
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	a;
	float			q;

}VU_CVECTOR;

/** float version of color vector(128) */
typedef struct
{
	float			r;
	float			g;
	float			b;
	float			a;

}VU_FCVECTOR;

typedef struct
{
	VU_VECTOR		direction;
	VU_FCVECTOR		color;

}VU_FLAT_LIGHT;

#ifndef ftoi4
#define ftoi4(f)				((int)((f)*16.0f))
#endif

#ifndef deg2radian
#define deg2radian(angle)		((angle*3.1415926535f)/180)
#endif

#ifndef radian2deg
#define radian2deg(radian)		((180.0f / 3.1415926535f) * (radian))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* common stuff */
extern void  VuInit(void);

extern void  VuSetGeometryXYOffset(unsigned short x, unsigned short y);
extern void  VuSetProjection(float z);
extern void  VuSetProjectionMatrix(VU_MATRIX	*projection);
extern void  VuSetProjectionType(unsigned int type);
extern void  VuSetWorldMatrix(VU_MATRIX			*world);
extern void  VuSetViewMatrix(VU_MATRIX			*view);
extern void  VuSetLocalScreenMatrix(VU_MATRIX	*m);
extern void  VuSetProjectionNearPlaneWH(unsigned int w, unsigned int h);
extern void  VuSetAmbientLight(float r, float g, float b);

/* 99% hw(vu0 macro mode) */
extern void  Vu0IdMatrix(VU_MATRIX *m);
extern void  Vu0ResetMatrix(VU_MATRIX *m);
//extern void  VuxRotMatrixX(VU_MATRIX *m, float x);
//extern void  VuxRotMatrixY(VU_MATRIX *m, float y);
//extern void  VuxRotMatrixZ(VU_MATRIX *m, float z);
//extern void  VuxRotMatrixXYZ(VU_MATRIX *m, float x,float y, float z);
extern void  Vu0TransMatrix(VU_MATRIX *m, VU_VECTOR *t);
extern void  Vu0TransMatrixXYZ(VU_MATRIX *m,float x, float y, float z);
extern void  Vu0ScaleMatrix(VU_MATRIX *m, VU_VECTOR *s);
extern void  Vu0ScaleMatrixXYZ(VU_MATRIX *m, float x, float y, float z);
extern void  Vu0MulMatrix(VU_MATRIX *m0, VU_MATRIX *m1, VU_MATRIX *out);
//extern void Vu0InverseMatrix(VU_MATRIX *in, VU_MATRIX *out);
extern void  Vu0ApplyMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out);
extern void  Vu0ApplyRotMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out);
extern void  Vu0CopyMatrix(VU_MATRIX *dest, VU_MATRIX *src);
extern float Vu0DotProduct(VU_VECTOR *v0, VU_VECTOR *v1);

/* 100% sw */
extern void  VuxIdMatrix(VU_MATRIX *m);
extern void  VuxResetMatrix(VU_MATRIX *m);
extern void  VuxRotMatrix(VU_MATRIX *m, VU_VECTOR *v);
extern void  VuxRotMatrixX(VU_MATRIX *m, float x);
extern void  VuxRotMatrixY(VU_MATRIX *m, float y);
extern void  VuxRotMatrixZ(VU_MATRIX *m, float z);
extern void  VuxRotMatrixXYZ(VU_MATRIX *m, float x,float y, float z);
extern void  VuxTransMatrix(VU_MATRIX *m, VU_VECTOR *v0);
extern void  VuxTransMatrixXYZ(VU_MATRIX *m,float x, float y, float z);
extern void  VuxScaleMatrix(VU_MATRIX *m, VU_VECTOR *s);
extern void  VuxScaleMatrixXYZ(VU_MATRIX *m, float x, float y, float z);
extern void  VuxMulMatrix(VU_MATRIX *m0, VU_MATRIX *m1,  VU_MATRIX *out);
extern void  VuxInverseMatrix(VU_MATRIX *in, VU_MATRIX *mat);
extern void  VuxCopyMatrix(VU_MATRIX *dest, VU_MATRIX *src);
extern void  VuxApplyMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out);
extern void  VuxApplyRotMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out);
extern float VuxDotProduct(VU_VECTOR *v0, VU_VECTOR *v1);
extern VU_VECTOR  VuxCrossProduct(VU_VECTOR *v0, VU_VECTOR *v1);
extern void  VuxCrossProduct0(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *out);
extern void  VuxVectorNormal(VU_VECTOR *v);
extern void  VuxVectorNormal0(VU_VECTOR *in, VU_VECTOR *out);

extern void  VuxApplyMatrixLS(VU_VECTOR *v0, VU_VECTOR *out);
extern void  VuxApplyRotMatrixLS(VU_VECTOR *v0, VU_VECTOR *out);

extern void  VuxMakeLocalScreenMatrix(VU_MATRIX *out, VU_MATRIX *world, VU_MATRIX *view);
extern void  VuxMakeLocalScreenMatrix2(VU_MATRIX *out, VU_MATRIX *world, VU_MATRIX *view, VU_MATRIX *projection);

extern void  VuxMakeViewMatrix(VU_MATRIX *out, VU_VECTOR *rot, VU_VECTOR *pos, VU_VECTOR *scale);
extern void  VuxMakeLookAtViewMatrix(VU_MATRIX *out, VU_VECTOR *eye, VU_VECTOR *target, VU_VECTOR *up);
extern void  VuxMakeProjectionMatrix(VU_MATRIX *proj, float near_plane_w, float near_plane_h, float near_plane_z, float far_plane_z);

/** update lsm using view, world, proj */
extern void  VuxUpdateLocalScreenMatrix(void);

extern void  VuxRotTrans(VU_VECTOR *v0, VU_VECTOR *out);
extern void  VuxRotTrans3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2,  VU_VECTOR *tv0, VU_VECTOR *tv1, VU_VECTOR *tv2);
extern void  VuxRotTransN(VU_VECTOR *verts,  VU_VECTOR *tverts, unsigned int num_verts);
extern void  VuxPers(VU_VECTOR *v0, VU_SXYZ *sxyz0);
extern void  VuxPers3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2);
extern void  VuxPersN(VU_VECTOR *verts, VU_SXYZ *sxyz, unsigned int num_verts);
extern int   VuxPersClip3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2);
extern float VuxRotTransPers(VU_VECTOR *v0, VU_SXYZ *sxyz0);
extern float VuxRotTransPers3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2);
extern void  VuxRotTransPersN(VU_VECTOR *verts, VU_SXYZ *sxyz, unsigned int num_verts);
extern int   VuxRotTransPersClip3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2);

extern int   VuxClipSxyz(VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2);

/*lighting*/
extern int VuxLightNormal(VU_VECTOR *normal, VU_CVECTOR *col0, void *light, unsigned int light_type, VU_CVECTOR *out0);

extern VU_MATRIX	VuWorldMatrix;
extern VU_MATRIX	VuViewMatrix;
extern VU_MATRIX	VuPrjectionMatrix;
extern VU_MATRIX	VuLocalScreenMatrix;

#ifdef __cplusplus
}
#endif

#endif /* __LIBVUX_H__ */
