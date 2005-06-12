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

 #include <tamtypes.h>

 #include <graph.h>
 #include <math3d.h>
 #include <string.h>
 #include <floatlib.h>

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 void vector_apply(VECTOR output, VECTOR input0, MATRIX input1) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%2)	\n"
   "lqc2		vf2, 0x10(%2)	\n"
   "lqc2		vf3, 0x20(%2)	\n"
   "lqc2		vf4, 0x30(%2)	\n"
   "lqc2		vf5, 0x00(%1)	\n"
   "vmulaw		ACC, vf4, vf0	\n"
   "vmaddax		ACC, vf1, vf5	\n"
   "vmadday		ACC, vf2, vf5	\n"
   "vmaddz		vf6, vf3, vf5	\n"
   "sqc2		vf6, 0x00(%0)	\n"
   : : "r" (output), "r" (input0), "r" (input1)
  );
 }

 void vector_clamp(VECTOR output, VECTOR input0, float min, float max) {
  VECTOR work;

  // Copy the vector.
  vector_copy(work, input0);

  // Clamp the minimum values.
  if (work[0] < min) { work[0] = min; }
  if (work[1] < min) { work[1] = min; }
  if (work[2] < min) { work[2] = min; }
  if (work[3] < min) { work[3] = min; }

  // Clamp the maximum values.
  if (work[0] > max) { work[0] = max; }
  if (work[1] > max) { work[1] = max; }
  if (work[2] > max) { work[2] = max; }
  if (work[3] > max) { work[3] = max; }

  // Output the result.
  vector_copy(output, work);

 }

 void vector_copy(VECTOR output, VECTOR input0) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "sqc2		vf1, 0x00(%0)	\n"
   : : "r" (output), "r" (input0)
  );
 }

 float vector_innerproduct(VECTOR input0, VECTOR input1) {
  VECTOR work0, work1;

  // Normalize the first vector.
  work0[0] = (input0[0] / input0[3]);
  work0[1] = (input0[1] / input0[3]);
  work0[2] = (input0[2] / input0[3]);
  work0[3] = 1.00f;

  // Normalize the second vector.
  work1[0] = (input1[0] / input1[3]);
  work1[1] = (input1[1] / input1[3]);
  work1[2] = (input1[2] / input1[3]);
  work1[3] = 1.00f;

  // Return the inner product.
  return (work0[0] * work1[0]) + (work0[1] * work1[1]) + (work0[2] * work1[2]);

 }

 void vector_multiply(VECTOR output, VECTOR input0, VECTOR input1) {
  VECTOR work;

  // Multiply the vectors together.
  work[0] = input0[0] * input1[0];
  work[1] = input0[1] * input1[1];
  work[2] = input0[2] * input1[2];
  work[3] = input0[3] * input1[3];

  // Output the result.
  vector_copy(output, work);

 }

 void vector_normalize(VECTOR output, VECTOR input0) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "vmul.xyz		vf2, vf1, vf1	\n"
   "vmulax.w		ACC, vf0, vf2	\n"
   "vmadday.w		ACC, vf0, vf2	\n"
   "vmaddz.w		vf2, vf0, vf2	\n"
   "vrsqrt		Q, vf0w, vf2w	\n"
   "vsub.w		vf1, vf0, vf0	\n"
   "vwaitq				\n"
   "vmulq.xyz		vf1, vf1, Q	\n"
   "sqc2		vf1, 0x00(%0)	\n"
   : : "r" (output), "r" (input0)
  );
 }

 void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "lqc2		vf2, 0x00(%2)	\n"
   "vopmula.xyz		ACC, vf1, vf2	\n"
   "vopmsub.xyz		vf2, vf2, vf1	\n"
   "vsub.w		vf2, vf0, vf0	\n"
   "sqc2		vf2, 0x00(%0)	\n"
   : : "r" (output), "r" (input0), "r" (input1)
  );
 }

 //////////////////////
 // MATRIX FUNCTIONS //
 //////////////////////

 void matrix_copy(MATRIX output, MATRIX input0) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "lqc2		vf2, 0x10(%1)	\n"
   "lqc2		vf3, 0x20(%1)	\n"
   "lqc2		vf4, 0x30(%1)	\n"
   "sqc2		vf1, 0x00(%0)	\n"
   "sqc2		vf2, 0x10(%0)	\n"
   "sqc2		vf3, 0x20(%0)	\n"
   "sqc2		vf4, 0x30(%0)	\n"
   : : "r" (output), "r" (input0)
  );
 }

 void matrix_inverse(MATRIX output, MATRIX input0) {
  MATRIX work;

  // Calculate the inverse of the matrix.
  matrix_transpose(work, input0);
  work[0x03] = 0.00f;
  work[0x07] = 0.00f;
  work[0x0B] = 0.00f;
  work[0x0C] = -(input0[0x0C] * work[0x00] + input0[0x0D] * work[0x04] + input0[0x0E] * work[0x08]);
  work[0x0D] = -(input0[0x0C] * work[0x01] + input0[0x0D] * work[0x05] + input0[0x0E] * work[0x09]);
  work[0x0E] = -(input0[0x0C] * work[0x02] + input0[0x0D] * work[0x06] + input0[0x0E] * work[0x0A]);
  work[0x0F] = 1.00f;

  // Output the result.
  matrix_copy(output, work);

 }

 void matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "lqc2		vf2, 0x10(%1)	\n"
   "lqc2		vf3, 0x20(%1)	\n"
   "lqc2		vf4, 0x30(%1)	\n"
   "lqc2		vf5, 0x00(%2)	\n"
   "lqc2		vf6, 0x10(%2)	\n"
   "lqc2		vf7, 0x20(%2)	\n"
   "lqc2		vf8, 0x30(%2)	\n"
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
   "sqc2		vf1, 0x00(%0)	\n"
   "sqc2		vf2, 0x10(%0)	\n"
   "sqc2		vf3, 0x20(%0)	\n"
   "sqc2		vf4, 0x30(%0)	\n"
   : : "r" (output), "r" (input0), "r" (input1)
  );
 }

 void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the z-axis rotation.
  matrix_unit(work);
  work[0x00] =  cosf(input1[2]);
  work[0x01] =  sinf(input1[2]);
  work[0x04] = -sinf(input1[2]);
  work[0x05] =  cosf(input1[2]);
  matrix_multiply(output, input0, work);

  // Apply the y-axis rotation.
  matrix_unit(work);
  work[0x00] =  cosf(input1[1]);
  work[0x02] = -sinf(input1[1]);
  work[0x08] =  sinf(input1[1]);
  work[0x0A] =  cosf(input1[1]);
  matrix_multiply(output, output, work);

  // Apply the x-axis rotation.
  matrix_unit(work);
  work[0x05] =  cosf(input1[0]);
  work[0x06] =  sinf(input1[0]);
  work[0x09] = -sinf(input1[0]);
  work[0x0A] =  cosf(input1[0]);
  matrix_multiply(output, output, work);

 }

 void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the scaling.
  matrix_unit(work);
  work[0x00] = input1[0];
  work[0x05] = input1[1];
  work[0x0A] = input1[2];
  matrix_multiply(output, input0, work);

 }

 void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the translation.
  matrix_unit(work);
  work[0x0C] = input1[0];
  work[0x0D] = input1[1];
  work[0x0E] = input1[2];
  matrix_multiply(output, input0, work);

 }

 void matrix_transpose(MATRIX output, MATRIX input0) {
  MATRIX work;

  // Transpose the matrix.
  work[0x00] = input0[0x00];
  work[0x01] = input0[0x04];
  work[0x02] = input0[0x08];
  work[0x03] = input0[0x0C];
  work[0x04] = input0[0x01];
  work[0x05] = input0[0x05];
  work[0x06] = input0[0x09];
  work[0x07] = input0[0x0D];
  work[0x08] = input0[0x02];
  work[0x09] = input0[0x06];
  work[0x0A] = input0[0x0A];
  work[0x0B] = input0[0x0E];
  work[0x0C] = input0[0x03];
  work[0x0D] = input0[0x07];
  work[0x0E] = input0[0x0B];
  work[0x0F] = input0[0x0F];

  // Output the result.
  matrix_copy(output, work);

 }

 void matrix_unit(MATRIX output) {

  // Create a unit matrix.
  memset(output, 0, sizeof(MATRIX));
  output[0x00] = 1.00f;
  output[0x05] = 1.00f;
  output[0x0A] = 1.00f;
  output[0x0F] = 1.00f;

 }

 //////////////////////
 // CREATE FUNCTIONS //
 //////////////////////

 void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation) {

  // Create the local_world matrix.
  matrix_unit(local_world);
  matrix_rotate(local_world, local_world, rotation);
  matrix_translate(local_world, local_world, translation);

 }

 void create_local_light(MATRIX local_light, VECTOR rotation) {

  // Create the local_light matrix.
  matrix_unit(local_light);
  matrix_rotate(local_light, local_light, rotation);

 }

 void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation) {
  VECTOR work0, work1;

  // Reverse the translation.
  work0[0] = -translation[0];
  work0[1] = -translation[1];
  work0[2] = -translation[2];
  work0[3] = translation[3];

  // Reverse the rotation.
  work1[0] = -rotation[0];
  work1[1] = -rotation[1];
  work1[2] = -rotation[2];
  work1[3] = rotation[3];

  // Create the world_view matrix.
  matrix_unit(world_view);
  matrix_translate(world_view, world_view, work0);
  matrix_rotate(world_view, world_view, work1);

 }

 void create_view_screen(MATRIX view_screen, float aspect, float left, float right, float bottom, float top, float near, float far) {

  // Apply the aspect ratio adjustment.
  left = (left * aspect); right = (right * aspect);

  // Create the view_screen matrix.
  matrix_unit(view_screen);
  view_screen[0x00] = (2 * near) / (right - left);
  view_screen[0x05] = (2 * near) / (top - bottom);
  view_screen[0x08] = (right + left) / (right - left);
  view_screen[0x09] = (top + bottom) / (top - bottom);
  view_screen[0x0A] = (far + near) / (far - near);
  view_screen[0x0B] = -1.00f;
  view_screen[0x0E] = (2 * far * near) / (far - near);
  view_screen[0x0F] = 0.00f;

 }

 void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen) {

  // Create the local_screen matrix.
  matrix_unit(local_screen);
  matrix_multiply(local_screen, local_screen, local_world);
  matrix_multiply(local_screen, local_screen, world_view);
  matrix_multiply(local_screen, local_screen, view_screen);

 }

 /////////////////////////
 // CALCULATE FUNCTIONS //
 /////////////////////////

 void calculate_normals(VECTOR *output, int count, VECTOR *normals, MATRIX local_world) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%3)	\n"
   "lqc2		vf2, 0x10(%3)	\n"
   "lqc2		vf3, 0x20(%3)	\n"
   "lqc2		vf4, 0x30(%3)	\n"
   "1:					\n"
   "lqc2		vf6, 0x00(%2)	\n"
   "vmulaw		ACC, vf4, vf0	\n"
   "vmaddax		ACC, vf1, vf6	\n"
   "vmadday		ACC, vf2, vf6	\n"
   "vmaddz		vf7, vf3, vf6	\n"
   "vdiv		Q, vf0w, vf7w	\n"
   "vwaitq				\n"
   "vmulq.xyzw		vf7, vf7, Q	\n"
   "sqc2		vf7, 0x00(%0)	\n"
   "addi		%0, 0x10	\n"
   "addi		%2, 0x10	\n"
   "addi		%1, -1		\n"
   "bne			$0, %1, 1b	\n"
   : : "r" (output), "r" (count), "r" (normals), "r" (local_world)
  );
 }

 void calculate_lights(VECTOR *output, int count, VECTOR *normals, VECTOR *light_direction, VECTOR *light_colour, int *light_type, int light_count) {
  int loop0, loop1; float intensity = 0.00f;

  // Clear the output values.
  memset(output, 0, sizeof(VECTOR) * count);

  // For each normal...
  for (loop0=0;loop0<count;loop0++) {

   // For each light...
   for (loop1=0;loop1<light_count;loop1++) {

    // If this is an ambient light...
    if (light_type[loop1] == LIGHT_AMBIENT)  {

     // Set the intensity to full.
     intensity = 1.00f;

    // Else, if this is a directional light...
    } else if (light_type[loop1] == LIGHT_DIRECTIONAL)  {

     // Get the light intensity.
     intensity = -vector_innerproduct(normals[loop0], light_direction[loop1]);

     // Clamp the minimum intensity.
     if (intensity < 0.00f) { intensity = 0.00f; }

    // Else, this is an invalid light type.
    } else { intensity = 0.00f; }

    // If the light has intensity...
    if (intensity > 0.00f) {

     // Add the light value.
     output[loop0][0] += (light_colour[loop1][0] * intensity);
     output[loop0][1] += (light_colour[loop1][1] * intensity);
     output[loop0][2] += (light_colour[loop1][2] * intensity);
     output[loop0][3] = 1.00f;

    }

   }

  }

 }

 void calculate_colours(VECTOR *output, int count, VECTOR *colours, VECTOR *lights) {
  int loop0;

  // For each colour...
  for (loop0=0;loop0<count;loop0++) {

   // Apply the light value to the colour.
   output[loop0][0] = (colours[loop0][0] * lights[loop0][0]);
   output[loop0][1] = (colours[loop0][1] * lights[loop0][1]);
   output[loop0][2] = (colours[loop0][2] * lights[loop0][2]);

   // Clamp the colour value.
   vector_clamp(output[loop0], output[loop0], 0.00f, 1.99f);

  }

 }

 void calculate_vertices(VECTOR *output, int count, VECTOR *vertices, MATRIX local_screen) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%3)	\n"
   "lqc2		vf2, 0x10(%3)	\n"
   "lqc2		vf3, 0x20(%3)	\n"
   "lqc2		vf4, 0x30(%3)	\n"
   "1:					\n"
   "lqc2		vf6, 0x00(%2)	\n"
   "vmulaw		ACC, vf4, vf0	\n"
   "vmaddax		ACC, vf1, vf6	\n"
   "vmadday		ACC, vf2, vf6	\n"
   "vmaddz		vf7, vf3, vf6	\n"
   "vclipw.xyz		vf7, vf7	\n" // FIXME: Clip detection is still kinda broken.
   "cfc2		$10, $18	\n"
   "beq			$10, $0, 3f	\n"
   "2:					\n"
   "sqc2		vi00, 0x00(%0)	\n"
   "j			4f		\n"
   "3:					\n"
   "vdiv		Q, vf0w, vf7w	\n"
   "vwaitq				\n"
   "vmulq.xyz		vf7, vf7, Q	\n"
   "sqc2		vf7, 0x00(%0)	\n"
   "4:					\n"
   "addi		%0, 0x10	\n"
   "addi		%2, 0x10	\n"
   "addi		%1, -1		\n"
   "bne			$0, %1, 1b	\n"
   : : "r" (output), "r" (count), "r" (vertices), "r" (local_screen) : "$10"
  );
 }
