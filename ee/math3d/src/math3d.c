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

 #include <math3d.h>
 #include <floatlib.h>

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 void vector_copy(VECTOR output, VECTOR input0) {
  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "sqc2		vf1, 0x00(%0)	\n"
   : : "r" (output), "r" (input0)
  );
 }

 void vector_multiply(VECTOR output, VECTOR input0, MATRIX input1) {
  MATRIX work;

  // Transpose the input matrix.
  matrix_transpose(work, input1);

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
   : : "r" (output), "r" (input0), "r" (work)
  );

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
   "lqc2                vf1, 0x00(%1)   \n"
   "lqc2                vf2, 0x00(%2)   \n"
   "vopmula.xyz         ACC, vf1, vf2   \n"
   "vopmsub.xyz         vf2, vf2, vf1   \n"
   "vsub.w              vf2, vf0, vf0   \n"
   "sqc2                vf2, 0x00(%0)   \n"
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

 void matrix_inverse(MATRIX output, MATRIX input0) { MATRIX work;

  // Calculate the inverse of the matrix.
  matrix_transpose(work, input0);
  work[0][3] = -(input0[0][3] * work[0][0] + input0[1][3] * work[0][1] + input0[2][3] * work[0][2]);
  work[1][3] = -(input0[0][3] * work[1][0] + input0[1][3] * work[1][1] + input0[2][3] * work[1][2]);
  work[2][3] = -(input0[0][3] * work[2][0] + input0[1][3] * work[2][1] + input0[2][3] * work[2][2]);
  work[3][0] = 0.00f; work[3][1] = 0.00f; work[3][2] = 0.00f; work[3][3] = 1.00f;

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
  work[0][0] =  cosf(input1[2]);
  work[0][1] = -sinf(input1[2]);
  work[1][0] =  sinf(input1[2]);
  work[1][1] =  cosf(input1[2]);
  matrix_multiply(output, work, input0);

  // Apply the y-axis rotation.
  matrix_unit(work);
  work[0][0] =  cosf(input1[1]);
  work[0][2] = -sinf(input1[1]);
  work[2][0] =  sinf(input1[1]);
  work[2][2] =  cosf(input1[1]);
  matrix_multiply(output, work, output);

  // Apply the x-axis rotation.
  matrix_unit(work);
  work[1][1] =  cosf(input1[0]);
  work[1][2] = -sinf(input1[0]);
  work[2][1] =  sinf(input1[0]);
  work[2][2] =  cosf(input1[0]);
  matrix_multiply(output, work, output);

 }

 void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the scale.
  matrix_unit(work);
  work[0][0] = input1[0];
  work[1][1] = input1[1];
  work[2][2] = input1[2];
  matrix_multiply(output, work, input0);

 }

 void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the translation.
  matrix_unit(work);
  work[0][3] = input1[0];
  work[1][3] = input1[1];
  work[2][3] = input1[2];
  matrix_multiply(output, work, input0);

 }

 void matrix_transpose(MATRIX output, MATRIX input0) {
  MATRIX work;
  int loop0, loop1;

  // Transpose the matrix.
  for (loop0=0;loop0<4;loop0++) {
   for (loop1=0;loop1<4;loop1++) {
    work[loop0][loop1] = input0[loop1][loop0];
   }
  }

  // Output the result.
  matrix_copy(output, work);

 }

 void matrix_unit(MATRIX output) {

  // Create a unit matrix.
  output[0][0] = 1.00f; output[0][1] = 0.00f; output[0][2] = 0.00f; output[0][3] = 0.00f;
  output[1][0] = 0.00f; output[1][1] = 1.00f; output[1][2] = 0.00f; output[1][3] = 0.00f;
  output[2][0] = 0.00f; output[2][1] = 0.00f; output[2][2] = 1.00f; output[2][3] = 0.00f;
  output[3][0] = 0.00f; output[3][1] = 0.00f; output[3][2] = 0.00f; output[3][3] = 1.00f;

 }

 ////////////////////////
 // PIPELINE FUNCTIONS //
 ////////////////////////

 void create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation) {

  // Create the local_world matrix.
  matrix_unit(local_world);
  matrix_rotate(local_world, local_world, rotation);
  matrix_translate(local_world, local_world, translation);

 }

 void create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation) {
  VECTOR direction_x = { 1.00f, 0.00f, 0.00f, 1.00f };
  VECTOR direction_y = { 0.00f, 1.00f, 0.00f, 1.00f };
  VECTOR direction_z = { 0.00f, 0.00f, 1.00f, 1.00f };

  // Create the world_view matrix.
  matrix_unit(world_view);
  vector_outerproduct(direction_x, direction_y, direction_z);
  vector_normalize(world_view[0], direction_x);
  vector_normalize(world_view[2], direction_z);
  vector_outerproduct(world_view[1], world_view[2], world_view[0]);
  matrix_rotate(world_view, world_view, rotation);
  matrix_translate(world_view, world_view, translation);
  matrix_inverse(world_view, world_view);

 }

 void create_view_screen(MATRIX view_screen, int max_x, int max_y, int max_z, int aspect_x, int aspect_y, float scrz, float nearz, float farz) {
  MATRIX work;
  float zmin = 1.00f;

  //
  matrix_unit(view_screen);
  view_screen[0][0] = scrz;
  view_screen[1][1] = scrz;
  view_screen[2][2] = 0.00f;
  view_screen[2][3] = 1.00f;
  view_screen[3][2] = 1.00f;
  view_screen[3][3] = 0.00f;

  //
  matrix_unit(work);
  work[0][0] = (float)((float)aspect_y / (float)aspect_x * (float)max_x / (float)max_y);
  work[1][1] = 1.00f;
  work[2][2] = farz * nearz * (max_z - 1.00f) / (farz - nearz);
  work[0][3] = 2048.0f; // Center of PS2 primitive space. (0 - 4096)
  work[1][3] = 2048.0f; // Center of PS2 primitive space. (0 - 4096)
  work[2][3] = (-max_z * nearz + farz) / (-nearz + farz);

  // Combine and output the result.
  matrix_multiply(view_screen, work, view_screen);

 }

 void create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen) {
  MATRIX work;

  // Create the local_screen matrix.
  matrix_multiply(work, world_view, local_world);
  matrix_multiply(local_screen, view_screen, work);

 }

 void point_calculate(VECTORI *output, MATRIX local_screen, VECTOR *input, int count) {
  MATRIX work;

  // Transpose the local_screen matrix.
  matrix_transpose(work, local_screen);

  asm __volatile__ (
   "lqc2		vf1, 0x00(%1)	\n"
   "lqc2		vf2, 0x10(%1)	\n"
   "lqc2		vf3, 0x20(%1)	\n"
   "lqc2		vf4, 0x30(%1)	\n"
   "1:					\n"
   "lqc2		vf5, 0x00(%2)	\n"
   "vmulaw		ACC, vf4, vf0	\n"
   "vmaddax		ACC, vf1, vf5	\n"
   "vmadday		ACC, vf2, vf5	\n"
   "vmaddz		vf6, vf3, vf5	\n"
   "vdiv		Q, vf0w, vf6w	\n"
   "vwaitq				\n"
   "vmulq.xyz		vf6, vf6, Q	\n"
   "vftoi4.xy		vf7, vf6	\n"
   "vftoi0.z		vf7, vf6	\n"
   "sqc2		vf7, 0x00(%0)	\n"
   "addi		%2, 0x10	\n"
   "addi		%0, 0x10	\n"
   "addi		%3, -1		\n"
   "bne			$0, %3, 1b	\n"
   : : "r" (output), "r" (work), "r" (input), "r" (count)
  );

 }
