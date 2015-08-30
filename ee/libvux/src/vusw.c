/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <libvux.h>

#include "vux.h"


#include <floatlib.h>













void VuxIdMatrix(VU_MATRIX *m)
{

	VuxResetMatrix(m);
}




void VuxResetMatrix(VU_MATRIX *m)
{

	m->m[0][0]=1.0f; m->m[0][1]=0.0f; m->m[0][2]=0.0f; m->m[0][3]=0.0f;
	m->m[1][0]=0.0f; m->m[1][1]=1.0f; m->m[1][2]=0.0f; m->m[1][3]=0.0f;
	m->m[2][0]=0.0f; m->m[2][1]=0.0f; m->m[2][2]=1.0f; m->m[2][3]=0.0f;
	m->m[3][0]=0.0f; m->m[3][1]=0.0f; m->m[3][2]=0.0f; m->m[3][3]=1.0f;
}




void VuxRotMatrix(VU_MATRIX *m, VU_VECTOR *r)
{
	VU_MATRIX	mx,my,mz, m0;

	VuxResetMatrix(m);
	mx = my = mz = m0 = *m;


	VuxRotMatrixX(&mx, r->x);
	VuxRotMatrixY(&my, r->y);
	VuxRotMatrixZ(&mz, r->z);

//	VuxMulMatrix(&my, &mx, &m0);
//	VuxMulMatrix(&mz, &m0, m);
	VuxMulMatrix(&mz, &mx, &m0);
	VuxMulMatrix(&my, &m0, m);
}






void VuxRotMatrixX(VU_MATRIX *m, float x)
{
	float cs,sn;

	cs = cosf(x);
	sn = sinf(x);


	m->m[1][1] = cs;
	m->m[1][2] = sn;
	m->m[2][1] = -sn;
	m->m[2][2] = cs;


/*  colum major(left hand)
	m->m[1][1] = cs;
	m->m[1][2] = -sn;
	m->m[2][1] = sn;
	m->m[2][2] = cs;
*/
}






void VuxRotMatrixY(VU_MATRIX *m, float y)
{
	float cs,sn;

	cs = cosf(y);
	sn = sinf(y);


	m->m[0][0] = cs;
	m->m[0][2] = -sn;
	m->m[2][0] = sn;
	m->m[2][2] = cs;


/*	colum major(left hand)
	m->m[0][0] = cs;
	m->m[0][2] = sn;
	m->m[2][0] = -sn;
	m->m[2][2] = cs;

	*/
}






void VuxRotMatrixZ(VU_MATRIX *m, float z)
{
	float cs,sn;

	cs = cosf(z);
	sn = sinf(z);


	m->m[0][0] = cs;
	m->m[0][1] = sn;
	m->m[1][0] = -sn;
	m->m[1][1] = cs;


	/* colum major(left hand)
	m->m[0][0] = cs;
	m->m[0][1] = -sn;
	m->m[1][0] = sn;
	m->m[1][1] = cs;
*/
}







void VuxRotMatrixXYZ(VU_MATRIX *m, float x,float y, float z)
{
	VU_MATRIX	mx,my,mz, m0;

	VuxResetMatrix(m);
	mx = my = mz = m0 = *m;

	VuxRotMatrixX(&mx, x);
	VuxRotMatrixY(&my, y);
	VuxRotMatrixZ(&mz, z);



//	VuxMulMatrix(&mx, &my, &m0);
//	VuxMulMatrix(&m0, &mz, m);

//	VuxMulMatrix(&mz, &my, &m0);
//	VuxMulMatrix(&mx, &m0, m);

	VuxMulMatrix(&mz, &mx, &m0);
	VuxMulMatrix(&my, &m0, m);
}









void VuxTransMatrix(VU_MATRIX *m, VU_VECTOR *t)
{

	m->m[3][0] =  t->x;
	m->m[3][1] =  t->y;
	m->m[3][2] =  t->z;
	m->m[3][3] =  1.0f;
}





/*move matrix to given xyz position*/
void VuxTransMatrixXYZ(VU_MATRIX *m,float x, float y, float z)
{

	m->m[3][0] =  x;
	m->m[3][1] =  y;
	m->m[3][2] =  z;
	m->m[3][3] =  1.0f;
}






void VuxScaleMatrix(VU_MATRIX *m, VU_VECTOR *s)
{
	VU_MATRIX	w;

	VuxResetMatrix(&w);

	w.m[0][0] = s->x;
	w.m[1][1] = s->y;
	w.m[2][2] = s->z;

	VuxMulMatrix(m, &w, m);
}




void VuxScaleMatrixXYZ(VU_MATRIX *m, float x, float y, float z)
{
	VU_MATRIX	w;

	VuxResetMatrix(&w);

	w.m[0][0] = x;
	w.m[1][1] = y;
	w.m[2][2] = z;

	VuxMulMatrix(m, &w, m);

}






void VuxMulMatrix(VU_MATRIX *m0, VU_MATRIX *m1, VU_MATRIX *out)
{
	int i,j,k;
    float* pA = (float*)m1;
    float* pB = (float*)m0;
	float* pQ = (float*)out;
    float  pM[16];

    for(i=0;i<16;i++)pM[i]=0;

    for(  i=0; i<4; i++ )
        for(  j=0; j<4; j++ )
            for(  k=0; k<4; k++ )
                pM[4*i+j] += pA[4*k+j] * pB[4*i+k];

    for(i=0;i<16;i++)pQ[i] = pM[i];
}





void VuxInverseMatrix(VU_MATRIX *in, VU_MATRIX *mat)
{
	int i,j;
	static float	*m,*out;
	float tmp[12]; /* temp array for pairs */
	float src[16]; /* array of transpose source matrix */
	float det; /* determinant */

	m	= (float *)in;
	out	= (float *)mat;


	/* transpose matrix */
	for ( i = 0; i < 4; i++)
	{
		src[i] = m[i*4];
		src[i + 4] = m[i*4 + 1];
		src[i + 8] = m[i*4 + 2];
		src[i + 12] = m[i*4 + 3];
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	out[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	out[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	out[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	out[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	out[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	out[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	out[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	out[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	out[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	out[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	out[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	out[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	out[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	out[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	out[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	out[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];

	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	/* calculate second 8 elements (cofactors) */
	out[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	out[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	out[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	out[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	out[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	out[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	out[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	out[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	out[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	out[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	out[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	out[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	out[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	out[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	out[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	out[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	/* calculate determinant */
	det=src[0]*out[0]+src[1]*out[1]+src[2]*out[2]+src[3]*out[3];
	/* calculate matrix inverse */
	det = 1/det;
	for (j = 0; j < 16; j++)
		out[j] *= det;
}





void VuxCopyMatrix(VU_MATRIX *dest, VU_MATRIX *src)
{
	int i,j;

	for(i=0;i<4;i++)
	{
		for(j=0;j<4;j++)
		{

			dest->m[i][j] = src->m[i][j];
		}

	}
}



void VuxApplyMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out)
{


	out->x = m->m[0][0]*v0->x + m->m[1][0]*v0->y + m->m[2][0]*v0->z + m->m[3][0]*v0->w;
	out->y = m->m[0][1]*v0->x + m->m[1][1]*v0->y + m->m[2][1]*v0->z + m->m[3][1]*v0->w;
	out->z = m->m[0][2]*v0->x + m->m[1][2]*v0->y + m->m[2][2]*v0->z + m->m[3][2]*v0->w;
	out->w = m->m[0][3]*v0->x + m->m[1][3]*v0->y + m->m[2][3]*v0->z + m->m[3][3]*v0->w;
}




void VuxApplyRotMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out)
{

	out->x = m->m[0][0]*v0->x + m->m[1][0]*v0->y + m->m[2][0]*v0->z;
	out->y = m->m[0][1]*v0->x + m->m[1][1]*v0->y + m->m[2][1]*v0->z;
	out->z = m->m[0][2]*v0->x + m->m[1][2]*v0->y + m->m[2][2]*v0->z;
	out->w = 1.0f;
}





float VuxDotProduct(VU_VECTOR *v0, VU_VECTOR *v1)
{

    return (v0->x*v1->x + v0->y*v1->y + v0->z*v1->z);
}





VU_VECTOR VuxCrossProduct(VU_VECTOR *v0, VU_VECTOR *v1)
{
	VU_VECTOR	ret;

	ret.x = v0->y*v1->z - v0->z*v1->y;
	ret.y = v0->z*v1->x - v0->x*v1->z;
	ret.z = v0->x*v1->y - v0->y*v1->x;

	return ret;
}





void VuxCrossProduct0(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *out)
{


	out->x = v0->y*v1->z - v0->z*v1->y;
	out->y = v0->z*v1->x - v0->x*v1->z;
	out->z = v0->x*v1->y - v0->y*v1->x;
}





void VuxVectorNormal(VU_VECTOR *v)
{
   float m = sqrtf(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);

   v->x /= m;
   v->y /= m;
   v->z /= m;
   v->w /= m;
}




void VuxVectorNormal0(VU_VECTOR *in, VU_VECTOR *out)
{
   float m = sqrtf(in->x*in->x + in->y*in->y + in->z*in->z + in->w*in->w);

   out->x /= m;
   out->y /= m;
   out->z /= m;
   out->w /= m;
}












/**/


void VuxApplyMatrixLS(VU_VECTOR *v0, VU_VECTOR *out)
{



//	Vu0ApplyMatrix(&VuLocalScreenMatrix, v0, out);
	VuxApplyMatrix(&VuLocalScreenMatrix, v0, out);
}




void VuxApplyRotMatrixLS(VU_VECTOR *v0, VU_VECTOR *out)
{


	VuxApplyRotMatrix(&VuLocalScreenMatrix, v0, out);

}








void VuxMakeLocalScreenMatrix(VU_MATRIX *out, VU_MATRIX *world, VU_MATRIX *view)
{

	VuxMulMatrix(world, view,  out);		// then multiply it by world

}







void VuxMakeLocalScreenMatrix2(VU_MATRIX *out, VU_MATRIX *world, VU_MATRIX *view, VU_MATRIX *projection)
{
	VU_MATRIX	work;



	VuxMulMatrix(world,	view,		&work);		// then multiply it by world
	VuxMulMatrix(&work,	projection,	out);		// mul projection
}











void VuxMakeViewMatrix(VU_MATRIX *out, VU_VECTOR *rot, VU_VECTOR *pos, VU_VECTOR *scale)
{
	VU_MATRIX	work;


	VuxResetMatrix(&work);
	VuxRotMatrix(&work, rot);
	VuxTransMatrix(&work, pos);
	VuxScaleMatrix(&work, scale);


	VuxInverseMatrix(&work, out);
}






void VuxMakeLookAtViewMatrix(VU_MATRIX *out, VU_VECTOR *eye, VU_VECTOR *target, VU_VECTOR *up)
{
	static VU_VECTOR	xaxis, yaxis, zaxis;


	zaxis.x = target->x - eye->x;
	zaxis.y = target->y - eye->y;
	zaxis.z = target->z - eye->z;
	VuxVectorNormal(&zaxis);

	xaxis = VuxCrossProduct(up, &zaxis);

	VuxVectorNormal(&xaxis);

	yaxis = VuxCrossProduct(&zaxis, &xaxis);

	out->m[0][0] = xaxis.x;
	out->m[0][1] = yaxis.x;
	out->m[0][2] = zaxis.x;
	out->m[0][3] = 0.0f;

	out->m[1][0] = xaxis.y;
	out->m[1][1] = yaxis.y;
	out->m[1][2] = zaxis.y;
	out->m[1][3] = 0.0f;

	out->m[2][0] = xaxis.z;
	out->m[2][1] = yaxis.z;
	out->m[2][2] = zaxis.z;
	out->m[2][3] = 0.0f;

	out->m[3][0] = -VuxDotProduct(&xaxis, eye);
	out->m[3][1] = -VuxDotProduct(&yaxis, eye);
	out->m[3][2] = -VuxDotProduct(&zaxis, eye);
	out->m[3][3] = 1.0f;


	/*******************************************
	** the created matrix is already inverted **
	*******************************************/

}







void VuxMakeProjectionMatrix(VU_MATRIX *proj, float near_plane_w, float near_plane_h, float near_plane_z, float far_plane_z)
{

	VuxIdMatrix(proj);



	proj->m[0][0] = (1.0f*near_plane_z)/near_plane_w;
    proj->m[1][1] = (1.0f*near_plane_z)/near_plane_h;
	proj->m[2][2] = far_plane_z/(far_plane_z-near_plane_z);
	proj->m[2][3] = 1.0f;
	proj->m[3][2] = near_plane_z*far_plane_z/(near_plane_z-far_plane_z);



}






void VuxUpdateLocalScreenMatrix(void)
{
	VU_MATRIX lsm;


	if(vu_projection_type==0)		// use vu_projection
	{
		VuxMakeLocalScreenMatrix(&lsm, &VuWorldMatrix, &VuViewMatrix);
	}
	else /*if(vu_projection_type==1)*/	// use projection matrix
	{
		VuxMakeLocalScreenMatrix2(&lsm, &VuWorldMatrix, &VuViewMatrix, &VuPrjectionMatrix);
	}



	VuSetLocalScreenMatrix(&lsm);
}










void VuxRotTrans(VU_VECTOR *v0, VU_VECTOR *out)
{

	VuxApplyMatrixLS(v0, out);
}




void VuxRotTrans3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2,  VU_VECTOR *tv0, VU_VECTOR *tv1, VU_VECTOR *tv2)
{

	// v0
	VuxApplyMatrixLS(v0, tv0);
	// v1
	VuxApplyMatrixLS(v1, tv1);
	// v2
	VuxApplyMatrixLS(v2, tv2);
}



void VuxRotTransN(VU_VECTOR *verts,  VU_VECTOR *tverts, unsigned int num_verts)
{
	unsigned int i;


	for(i=0;i<num_verts;i++)
	{
		VuxApplyMatrixLS(&verts[i], &tverts[i]);
	}
}




void VuxPers(VU_VECTOR *v0, VU_SXYZ *sxyz0)
{



	if(vu_projection_type==0)
	{

		sxyz0->x = ftoi4( (vu_projection * v0->x / (v0->z))		+vu_offset_x);
		sxyz0->y = ftoi4(-(vu_projection * v0->y / (v0->z))		+vu_offset_y);
		sxyz0->z = 0xffffff-(short)(float)v0->z;
	}
	else	// use projection matrix
	{

		sxyz0->x = ftoi4( (((v0->x)/(v0->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz0->y = ftoi4(-(((v0->y)/(v0->w))*vu_near_plane_h)	+vu_offset_y);

		sxyz0->z =  (int)(-(v0->z/v0->w) * 0xffff);
	}
}





void VuxPers3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2)
{



	if(vu_projection_type==0)
	{
		//v0
		sxyz0->x = ftoi4( (vu_projection * v0->x / (v0->z))		+vu_offset_x);
		sxyz0->y = ftoi4(-(vu_projection * v0->y / (v0->z))		+vu_offset_y);
		sxyz0->z = 0xffffff	 - (short)(float)v0->z;

		//v1
		sxyz1->x = ftoi4( (vu_projection * v1->x / (v1->z))		+vu_offset_x);
		sxyz1->y = ftoi4(-(vu_projection * v1->y / (v1->z))		+vu_offset_y);
		sxyz1->z = 0xffffff - (short)(float)v1->z;

		//v2
		sxyz2->x = ftoi4( (vu_projection * v2->x / (v2->z))		+vu_offset_x);
		sxyz2->y = ftoi4(-(vu_projection * v2->y / (v2->z))		+vu_offset_y);
		sxyz2->z = 0xffffff - (short)(float)v2->z;
	}
	else	// use projection matrix
	{
		sxyz0->x = ftoi4( (((v0->x)/(v0->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz0->y = ftoi4(-(((v0->y)/(v0->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz0->z =  (int)(-(v0->z/v0->w) * 0xffff);

		sxyz1->x = ftoi4( (((v1->x)/(v1->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz1->y = ftoi4(-(((v1->y)/(v1->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz1->z =  (int)(-(v1->z/v1->w) * 0xffff);

		sxyz2->x = ftoi4( (((v2->x)/(v2->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz2->y = ftoi4(-(((v2->y)/(v2->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz2->z =  (int)(-(v2->z/v2->w) * 0xffff);
	}
}






void VuxPersN(VU_VECTOR *verts, VU_SXYZ *sxyz, unsigned int num_verts)
{
	unsigned int	i;


	for(i=0;i<num_verts;i++)
	{
		VuxPers(&verts[i], &sxyz[i]);
	}
}






int VuxPersClip3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2)
{



	if(vu_projection_type==0)
	{
		//v0
		sxyz0->x = ftoi4( (vu_projection * v0->x / (v0->z))		+vu_offset_x);
		sxyz0->y = ftoi4(-(vu_projection * v0->y / (v0->z))		+vu_offset_y);
		sxyz0->z = 0xffffff - (short)(float)v0->z;

		//v1
		sxyz1->x = ftoi4( (vu_projection * v1->x / (v1->z))		+vu_offset_x);
		sxyz1->y = ftoi4(-(vu_projection * v1->y / (v1->z))		+vu_offset_y);
		sxyz1->z = 0xffffff - (short)(float)v1->z;

		//v2
		sxyz2->x = ftoi4( (vu_projection * v2->x / (v2->z))		+vu_offset_x);
		sxyz2->y = ftoi4(-(vu_projection * v2->y / (v2->z))		+vu_offset_y);
		sxyz2->z = 0xffffff - (short)(float)v2->z;
	}
	else	// use projection matrix
	{
		sxyz0->x = ftoi4( (((v0->x)/(v0->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz0->y = ftoi4(-(((v0->y)/(v0->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz0->z =  (int)(-(v0->z/v0->w) * 0xffff);

		sxyz1->x = ftoi4( (((v1->x)/(v1->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz1->y = ftoi4(-(((v1->y)/(v1->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz1->z =  (int)(-(v1->z/v1->w) * 0xffff);

		sxyz2->x = ftoi4( (((v2->x)/(v2->w))*vu_near_plane_w)	+vu_offset_x);
		sxyz2->y = ftoi4(-(((v2->y)/(v2->w))*vu_near_plane_h)	+vu_offset_y);
		sxyz2->z =  (int)(-(v2->z/v2->w) * 0xffff);
	}


	return VuxClipSxyz(sxyz0, sxyz1, sxyz2);
}






float VuxRotTransPers(VU_VECTOR *v0, VU_SXYZ *sxyz0)
{
	VU_VECTOR	tv0;

	VuxRotTrans(v0, &tv0);
	VuxPers(&tv0	, sxyz0);

	return tv0.z;
}





float VuxRotTransPers3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2)
{
	VU_VECTOR	tv0,tv1,tv2;

	VuxRotTrans3(v0, v1, v2,  &tv0, &tv1, &tv2);
	VuxPers3(&tv0, &tv1, &tv2, sxyz0, sxyz1, sxyz2);

	return tv0.z;
}






void VuxRotTransPersN(VU_VECTOR *verts, VU_SXYZ *sxyz, unsigned int num_verts)
{
	unsigned int i;



	for(i=0;i<num_verts;i++)
	{
		VuxRotTransPers(&verts[i], &sxyz[i]);
	}

}





int VuxRotTransPersClip3(VU_VECTOR *v0, VU_VECTOR *v1, VU_VECTOR *v2, VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2)
{
	VU_VECTOR	tv0,tv1,tv2;

	VuxRotTrans3(v0, v1, v2,  &tv0, &tv1, &tv2);
	VuxPers3(&tv0, &tv1, &tv2, sxyz0, sxyz1, sxyz2);


	return VuxClipSxyz(sxyz0, sxyz1, sxyz2);
}









int VuxClipSxyz(VU_SXYZ *sxyz0, VU_SXYZ *sxyz1, VU_SXYZ *sxyz2)
{

	return (sxyz1->x - sxyz0->x) * (sxyz2->y - sxyz0->y) - (sxyz2->x - sxyz0->x) * (sxyz1->y - sxyz0->y);
}



int VuxClipW(VU_VECTOR *tv0)
{
	int	ret = 0;

	return ret;
}








/**********************************************
* LIGHTING
*
*
***********************************************/



int VuxLightNormal(VU_VECTOR *normal, VU_CVECTOR *col0, void *light, unsigned int light_type, VU_CVECTOR *out0)
{
	float			dot;
	VU_FCVECTOR		final;
	VU_FCVECTOR		c0;
	VU_FLAT_LIGHT	*f_light;


	c0.r = col0->r * 0.0078125f;
	c0.g = col0->g * 0.0078125f;
	c0.b = col0->b * 0.0078125f;


	if(light_type == VU_LIGHT_TYPE_FLAT)
	{
		f_light = (VU_FLAT_LIGHT *)light;

		dot = -VuxDotProduct(normal, &f_light->direction);
		if(dot <0.0f)dot = 0.0f;



		final.r = vu_light_ambient.r + (c0.r * dot * f_light->color.r);
		final.g = vu_light_ambient.g + (c0.g * dot * f_light->color.g);
		final.b = vu_light_ambient.b + (c0.b * dot * f_light->color.b);
	}
	else
	{
		final.r = vu_light_ambient.r + c0.r;
		final.g = vu_light_ambient.g + c0.g;
		final.b = vu_light_ambient.b + c0.b;
	}



	out0->r =final.r*128.0f;
	out0->g =final.g*128.0f;
	out0->b =final.b*128.0f;
	out0->a =0x80;
	out0->q =1.0f;


	return 0;
}























/*EOF*/
