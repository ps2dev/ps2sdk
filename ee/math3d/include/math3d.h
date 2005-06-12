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

 typedef float VECTOR[4] __attribute__((__aligned__(16)));

 typedef float MATRIX[16] __attribute__((__aligned__(16)));

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 void vector_apply(VECTOR output, VECTOR input0, MATRIX input1);

 void vector_clamp(VECTOR output, VECTOR input0, float min, float max);

 void vector_copy(VECTOR output, VECTOR input0);

 void vector_flatten(VECTOR output, VECTOR input0);

 float vector_innerproduct(VECTOR input0, VECTOR input1);

 void vector_multiply(VECTOR output, VECTOR input0, VECTOR input1);

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

 //////////////////////
 // CREATE FUNCTIONS //
 //////////////////////

 void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation);

 void create_local_light(MATRIX local_light, VECTOR rotation);

 void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation);

 void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far);

 void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen);

 /////////////////////////
 // CALCULATE FUNCTIONS //
 /////////////////////////

 #define LIGHT_AMBIENT		0	// Ambient light source.
 #define LIGHT_DIRECTIONAL	1	// Directional light source.

 void calculate_normals(VECTOR *output, int count, VECTOR *normals, MATRIX local_world);

 void calculate_lights(VECTOR *output, int count, VECTOR *normals, VECTOR *light_directions, VECTOR *light_colours, int *light_types, int light_count);

 void calculate_colours(VECTOR *output, int count, VECTOR *colours, VECTOR *lights);

 void calculate_vertices(VECTOR *output, int count, VECTOR *vertices, MATRIX local_screen);

#ifdef __cplusplus
}
#endif

#endif
