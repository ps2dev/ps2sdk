/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2005 Naomi Peori <naomi@peori.ca>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
*/

/**
 * @file
 * 3D math functions.
 */

#ifndef __MATH3D_H__
#define __MATH3D_H__

#include <tamtypes.h>

typedef float VECTOR[4] __attribute__((__aligned__(16)));

typedef float MATRIX[16] __attribute__((__aligned__(16)));

#ifdef __cplusplus
extern "C" {
#endif

/* VECTOR FUNCTIONS */
/** Multiply a vector by a matrix, returning a vector. */
extern void vector_apply(VECTOR output, VECTOR input0, MATRIX input1);

/** Clamp a vector's values by cutting them off at a minimum and maximum value. */
extern void vector_clamp(VECTOR output, VECTOR input0, float min, float max);

/** Copy a vector. */
extern void vector_copy(VECTOR output, VECTOR input0);

/** Calculate the inner product of two vectors. Returns a scalar value. */
extern float vector_innerproduct(VECTOR input0, VECTOR input1);

/** Multiply two vectors together. */
extern void vector_multiply(VECTOR output, VECTOR input0, VECTOR input1);

/** Normalize a vector by determining its length and dividing its values by this value. */
extern void vector_normalize(VECTOR output, VECTOR input0);

/** Calculate the outer product of two vectors. */
extern void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1);

/** Add two vectors */
extern void vector_add(VECTOR sum, VECTOR addend, VECTOR summand);

/** Calculate the cross product of two vectors */
extern void vector_cross_product(VECTOR product, VECTOR multiplicand, VECTOR multiplier);

/** Calculates the normal of 3 vectors */
extern void vector_triangle_normal(VECTOR output, VECTOR a, VECTOR b, VECTOR c);

/* MATRIX FUNCTIONS */
/** Copy a matrix. */
extern void matrix_copy(MATRIX output, MATRIX input0);

/** Calculate the inverse of a matrix. */
extern void matrix_inverse(MATRIX output, MATRIX input0);

/** Multiply two matrices together. */
extern void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1);

/** Create a rotation matrix and apply it to the specified input matrix. */
extern void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1);

/** Create a scaling matrix and apply it to the specified input matrix. */
extern void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1);

/** Create a translation matrix and apply it to the specified input matrix. */
extern void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1);

/** Transpose a matrix. */
extern void matrix_transpose(MATRIX output, MATRIX input0);

/** Create a unit matrix. */
extern void matrix_unit(MATRIX output);

/* CREATE FUNCTIONS */

/** Create a local_world matrix given a translation and rotation.
 * Commonly used to describe an object's position and orientation.
 */
extern void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation);

/** Create a local_light matrix given a rotation.
 * Commonly used to transform an object's normals for lighting calculations.
 */
extern void create_local_light(MATRIX local_light, VECTOR rotation);

/** Create a world_view matrix given a translation and rotation.
 * Commonly used to describe a camera's position and rotation.
 */
extern void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation);

/** Create a view_screen matrix given an aspect and clipping plane values.
 * Functionally similar to the opengl function: glFrustum()
 */
extern void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far);

/** Create a local_screen matrix given a local_world, world_view and view_screen matrix.
 * Commonly used with vector_apply() to transform vertices for rendering.
 */
extern void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen);

/* CALCULATE FUNCTIONS */

/** Ambient light source. */
#define LIGHT_AMBIENT   0
/** Directional light source. */
#define LIGHT_DIRECTIONAL 1

/** Transform an array of normals by applying the specified local_light matrix. */
extern void calculate_normals(VECTOR *output, int count, VECTOR *normals, MATRIX local_light);

/** Calculate the light intensity for an array of lights given an array of normal values. */
extern void calculate_lights(VECTOR *output, int count, VECTOR *normals, VECTOR *light_directions, VECTOR *light_colours, const int *light_types, int light_count);

/** Calculate colour values given an array of light intensity values. */
extern void calculate_colours(VECTOR *output, int count, VECTOR *colours, VECTOR *lights);

/** Calculate vertex values by applying the specific local_screen matrix. */
extern void calculate_vertices(VECTOR *output, int count, VECTOR *vertices, MATRIX local_screen);

#ifdef __cplusplus
}
#endif

#endif /* __MATH3D_H__ */
