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





void Vu0IdMatrix(VU_MATRIX *m)
{

	Vu0ResetMatrix(m);
}




void Vu0ResetMatrix(VU_MATRIX *m)
{
	asm __volatile__(
	"vmr32.xyzw  vf18, vf00			\n"
	"sqc2        vf00, 0x30(%0)		\n"
	"vmr32.xyzw  vf17, vf18			\n"
	"sqc2        vf18, 0x20(%0)		\n"
	"vmr32.xyzw  vf16, vf17			\n"
	"sqc2        vf17, 0x10(%0)		\n"
	"sqc2        vf16, 0x00(%0)		\n"

	: : "r"(m)
    );
}



/*
void VuxRotMatrix(VU_MATRIX *m, VU_VECTOR *r)
{
	VU_MATRIX	mx,my,mz, m0;

	VuxResetMatrix(m);
	mx = my = mz = m0 = *m;


	VuxRotMatrixX(&mx, r->x);
	VuxRotMatrixY(&my, r->y);
	VuxRotMatrixZ(&mz, r->z);

	VuxMulMatrix(&mx, &my, &m0);
	VuxMulMatrix(&m0, &mz, m);
}
*/







void Vu0TransMatrix(VU_MATRIX *m, VU_VECTOR *t)
{

	asm __volatile__ (
   "lqc2		vf1,  0(%1)	\n"		// load 1 qword from 't' to vu's vf1
   "sqc2		vf1, 48(%0)	\n"		// store vf1 data in 'm' with 48 bytes offset which is m[3][0]
   
   : : "r" (m), "r" (t)
   );	
}



void Vu0TransMatrixXYZ(VU_MATRIX *m,float x, float y, float z)
{
	VU_VECTOR t;

	t.x	= x;
	t.y	= y;
	t.z	= z;
	t.w	= 1.0f;

	Vu0TransMatrix(m, &t);
}


/*
	THIS SCALE IS CROP, CLEAN IT UP ************************(99999999(****************************
 */

void Vu0ScaleMatrix(VU_MATRIX *m, VU_VECTOR *s)
{

	asm __volatile__ (
	"lqc2		vf1,   0(%1)	\n"		// load 1 qword from 't' to vu's vf1
	"lqc2		vf10,  0(%0)	\n"		// load m[0][0]
	"lqc2		vf11, 16(%0)	\n"		// load m[1][0]
	"lqc2		vf12, 32(%0)	\n"		// load m[2][0]
 //  "lqc2		vf123,48(%0)	\n"		// load m[3][0]

	"vmulx.x	vf10, vf10, vf1x	\n"	// multiply [0][0] by s->x and store in [0][0]
	"vmuly.y	vf11, vf11, vf1y	\n"	
	"vmulz.z	vf12, vf12, vf1z	\n"	
	

	"sqc2		vf10,  0(%0)	\n"		// store v m[0][0]
	"sqc2		vf11, 16(%0)	\n"		// store v m[1][0]
	"sqc2		vf12, 32(%0)	\n"		// store v m[2][0]
//   "sqc2		vf1, 48(%0)	\n"		// store v m[3][0]
   
   : : "r" (m), "r" (s)
   );

}







void Vu0ScaleMatrixXYZ(VU_MATRIX *m, float x, float y, float z)
{
	VU_VECTOR	s;
	s.x = x;
	s.y = y;
	s.z = z;
	s.w = 1.0f;
	
	Vu0ScaleMatrix(m, &s);

}




void Vu0MulMatrix(VU_MATRIX *m0, VU_MATRIX *m1, VU_MATRIX *out)
{
	
	asm __volatile__ (
   "lqc2		vf1, 0x00(%0)	\n"
   "lqc2		vf2, 0x10(%0)	\n"
   "lqc2		vf3, 0x20(%0)	\n"
   "lqc2		vf4, 0x30(%0)	\n"
   "lqc2		vf5, 0x00(%1)	\n"
   "lqc2		vf6, 0x10(%1)	\n"
   "lqc2		vf7, 0x20(%1)	\n"
   "lqc2		vf8, 0x30(%1)	\n"
   "vmulax.xyzw		ACC, vf5, vf1	\n"
   "vmadday.xyzw	ACC, vf6, vf1	\n"
   "vmaddaz.xyzw	ACC, vf7, vf1	\n"
   "vmaddw.xyzw		vf1, vf8, vf1	\n"
   "vmulax.xyzw		ACC, vf5, vf2	\n"
   "vmadday.xyzw	ACC, vf6, vf2	\n"
   "vmaddaz.xyzw	ACC, vf7, vf2	\n"
   "vmaddw.xyzw		vf2, vf8, vf2	\n"
   "vmulax.xyzw		ACC, vf5, vf3	\n"
   "vmadday.xyzw	ACC, vf6, vf3	\n"
   "vmaddaz.xyzw	ACC, vf7, vf3	\n"
   "vmaddw.xyzw		vf3, vf8, vf3	\n"
   "vmulax.xyzw		ACC, vf5, vf4	\n"
   "vmadday.xyzw	ACC, vf6, vf4	\n"
   "vmaddaz.xyzw	ACC, vf7, vf4	\n"
   "vmaddw.xyzw		vf4, vf8, vf4	\n"
   "sqc2		vf1, 0x00(%2)	\n"
   "sqc2		vf2, 0x10(%2)	\n"
   "sqc2		vf3, 0x20(%2)	\n"
   "sqc2		vf4, 0x30(%2)	\n"
   : :  "r" (m0), "r" (m1), "r" (out)
  );
}










void Vu0InverseMatrix(VU_MATRIX *in, VU_MATRIX *out)
{





}





void Vu0ApplyMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out)
{
	/*
	out->x = m->m[0][0]*v0->x + m->m[1][0]*v0->y + m->m[2][0]*v0->z + m->m[3][0]*v0->w;
	out->y = m->m[0][1]*v0->x + m->m[1][1]*v0->y + m->m[2][1]*v0->z + m->m[3][1]*v0->w;
	out->z = m->m[0][2]*v0->x + m->m[1][2]*v0->y + m->m[2][2]*v0->z + m->m[3][2]*v0->w;
	out->w = m->m[0][3]*v0->x + m->m[1][3]*v0->y + m->m[2][3]*v0->z + m->m[3][3]*v0->w;
	
	*/

	asm __volatile__(
        "lqc2            vf20,  0x00(%1)	\n"
        "lqc2            vf16,  0x00(%0)	\n"
        "lqc2            vf17,  0x10(%0)	\n"
        "lqc2            vf18,  0x20(%0)	\n"
        "lqc2            vf19,  0x30(%0)	\n"
        "vmulax.xyzw     ACC,   vf16,vf20	\n"
        "vmadday.xyzw    ACC,   vf17,vf20	\n"
        "vmaddaz.xyzw    ACC,   vf18,vf20	\n"
        "vmaddw.xyzw     vf20,  vf19,vf20	\n"
        "sqc2            vf20,0x00(%2)	\n"
        
        : : "r"(m), "r"(v0), "r"(out)
    );




}




void Vu0ApplyRotMatrix(VU_MATRIX *m, VU_VECTOR *v0, VU_VECTOR *out)
{
	/*	
	out->x = m->m[0][0]*v0->x + m->m[1][0]*v0->y + m->m[2][0]*v0->z;
	out->y = m->m[0][1]*v0->x + m->m[1][1]*v0->y + m->m[2][1]*v0->z;
	out->z = m->m[0][2]*v0->x + m->m[1][2]*v0->y + m->m[2][2]*v0->z;
	*/

	asm __volatile__(
        "lqc2            vf20,  0x00(%1)	\n"
        "lqc2            vf16,  0x00(%0)	\n"
        "lqc2            vf17,  0x10(%0)	\n"
        "lqc2            vf18,  0x20(%0)	\n"
        "vmulax.xyz		 ACC,   vf16,vf20	\n"
        "vmadday.xyz	 ACC,   vf17,vf20	\n"
        "vmaddz.xyz		 vf20,  vf18,vf20	\n"
		"vmulw.w		 vf20,	vf0, vf0	\n"	// out->w = 1.0f
        "sqc2            vf20,	0x00(%2)	\n" // copy result to out
        
        : : "r"(m), "r"(v0), "r"(out)
    );

}





void Vu0CopyMatrix(VU_MATRIX *dest, VU_MATRIX *src)
{
	
	asm __volatile__ (
   "lqc2		vf1,   0(%1)	\n"		// load 1 qword from ee
   "lqc2		vf2,  16(%1)	\n"		// load 1 qword from ee
   "lqc2		vf3,  32(%1)	\n"		// load 1 qword from ee
   "lqc2		vf4,  48(%1)	\n"		// load 1 qword from ee

   "sqc2		vf1,   0(%0)	\n"		// store 1 qword in ee
   "sqc2		vf2,  16(%0)	\n"		// store 1 qword in ee
   "sqc2		vf3,  32(%0)	\n"		// store 1 qword in ee
   "sqc2		vf4,  48(%0)	\n"		// store 1 qword in ee
   
   : : "r" (dest), "r" (src)
   );	
}






float Vu0DotProduct(VU_VECTOR *v0, VU_VECTOR *v1)
{
	float ret;
   
	/*	ret = (v0.x*v1.x + v0.y*v1.y + v0.z*v1.z);*/

	asm __volatile__ (
   "lqc2		vf1, 0(%0)		\n"		// load 1 qword from ee
   "lqc2		vf2, 0(%1)		\n"		// load 1 qword from ee
   
   "vmul.xyz	vf3, vf1, vf2	\n"		// mul v0 by v1 
   
   "vaddy.x		vf3, vf3, vf3y	\n"		// add x+y and store in x
   "vaddz.x		vf3, vf3, vf3z	\n"		// add z+x and store in x

   "qmfc2		$2, vf3			\n"		// copy vector to ee reg

	"sw			$2, 0(%2)		\n"		// copy reg to mem
   
   : : "r" (v0), "r" (v1), "r" (&ret)
   );	

	return ret;
}












/*EOF*/
