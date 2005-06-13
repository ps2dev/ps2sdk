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
 // Multiply a vector by a matrix, returning a vector.

 void vector_clamp(VECTOR output, VECTOR input0, float min, float max);
 // Clamp a vector's values by cutting them off at a minimum and maximum value.

 void vector_copy(VECTOR output, VECTOR input0);
 // Copy a vector.

 float vector_innerproduct(VECTOR input0, VECTOR input1);
 // Calculate the inner product of two vectors. Returns a scalar value.

 void vector_multiply(VECTOR output, VECTOR input0, VECTOR input1);
 // Multiply two vectors together.

 void vector_normalize(VECTOR output, VECTOR input0);
 // Normalize a vector by determining its length and dividing its values by this value.

 void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1);
 // Calculate the outer product of two vectors.

 //////////////////////
 // MATRIX FUNCTIONS //
 //////////////////////

 void matrix_copy(MATRIX output, MATRIX input0);
 // Copy a matrix.

 void matrix_inverse(MATRIX output, MATRIX input0);
 // Calculate the inverse of a matrix.

 void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1);
 // Multiply two matrices together.

 void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1);
 // Create a rotation matrix and apply it to the specified input matrix.

 void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1);
 // Create a scaling matrix and apply it to the specified input matrix.

 void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1);
 // Create a translation matrix and apply it to the specified input matrix.

 void matrix_transpose(MATRIX output, MATRIX input0);
 // Transpose a matrix.

 void matrix_unit(MATRIX output);
 // Create a unit matrix.

 //////////////////////
 // CREATE FUNCTIONS //
 //////////////////////

 void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation);
 // Create a local_world matrix given a translation and rotation.
 // Commonly used to describe an object's position and orientation.

 void create_local_light(MATRIX local_light, VECTOR rotation);
 // Create a local_light matrix given a rotation.
 // Commonly used to transform an object's normals for lighting calculations.

 void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation);
 // Create a world_view matrix given a translation and rotation.
 // Commonly used to describe a camera's position and rotation.

 void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far);
 // Create a view_screen matrix given an aspect and clipping plane values.
 // Functionally similar to the opengl function: glFrustum()

 void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen);
 // Create a local_screen matrix given a local_world, world_view and view_screen matrix.
 // Commonly used with vector_apply() to transform vertices for rendering.

 /////////////////////////
 // CALCULATE FUNCTIONS //
 /////////////////////////

 #define LIGHT_AMBIENT		0	// Ambient light source.
 #define LIGHT_DIRECTIONAL	1	// Directional light source.

 void calculate_normals(VECTOR *output, int count, VECTOR *normals, MATRIX local_light);
 // Transform an array of normals by applying the specified local_light matrix.

 void calculate_lights(VECTOR *output, int count, VECTOR *normals, VECTOR *light_directions, VECTOR *light_colours, int *light_types, int light_count);
 // Calculate the light intensity for an array of lights given an array of normal values.

 void calculate_colours(VECTOR *output, int count, VECTOR *colours, VECTOR *lights);
 // Calculate colour values given an array of light intensity values.

 void calculate_vertices(VECTOR *output, int count, VECTOR *vertices, MATRIX local_screen);
 // Calculate vertex values by applying the specific local_screen matrix.

#ifdef __cplusplus
}
#endif

#endif
