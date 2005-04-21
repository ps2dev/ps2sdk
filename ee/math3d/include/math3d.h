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

#ifndef __MATH3D_H__
#define __MATH3D_H__

 #include <tamtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

 typedef float VECTOR[4];

 typedef float MATRIX[4][4];

 typedef struct { int x, y, z; float w; } VECTORI;

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 void vector_copy(VECTOR output, VECTOR input0);

 void vector_multiply(VECTOR output, VECTOR input0, MATRIX input1);

 void vector_normalize(VECTOR output, VECTOR input0);

 void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1);

 //////////////////////
 // MATRIX FUNCTIONS //
 //////////////////////

 void matrix_copy(MATRIX output, MATRIX input0);

 void matrix_inverse(MATRIX output, MATRIX input0);

 void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1);

 void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1);

 void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1);

 void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1);

 void matrix_transpose(MATRIX output, MATRIX input0);

 void matrix_unit(MATRIX output);

 ////////////////////////
 // PIPELINE FUNCTIONS //
 ////////////////////////

 void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation);

 void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation);

 void create_view_screen(MATRIX view_screen, int max_x, int max_y, int max_z, int aspect_x, int aspect_y, float scrz, float nearz, float farz);

 void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen);

 void point_calculate(VECTORI *output, MATRIX local_screen, VECTOR *input, int count);

#ifdef __cplusplus
}
#endif

#endif
