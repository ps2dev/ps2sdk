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
 #include <string.h>
 #include <floatlib.h>

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 void vector_copy(VECTOR output, VECTOR input0) {

  // Copy the vector.
  output[0] = input0[0];
  output[1] = input0[1];
  output[2] = input0[2];
  output[3] = input0[3];

 }

 void vector_multiply(VECTOR output, VECTOR input0, MATRIX input1) {
  VECTOR work;

  // Multiply the vector by the matrix.
  work[0] = (input0[0] * input1[0x00]) + (input0[1] * input1[0x04]) + (input0[2] * input1[0x08]) + (input0[3] * input1[0x0C]);
  work[1] = (input0[0] * input1[0x01]) + (input0[1] * input1[0x05]) + (input0[2] * input1[0x09]) + (input0[3] * input1[0x0D]);
  work[2] = (input0[0] * input1[0x02]) + (input0[1] * input1[0x06]) + (input0[2] * input1[0x0A]) + (input0[3] * input1[0x0E]);
  work[3] = (input0[0] * input1[0x03]) + (input0[1] * input1[0x07]) + (input0[2] * input1[0x0B]) + (input0[3] * input1[0x0F]);

  // Output the result.
  vector_copy(output, work);

 }

 void vector_normalize(VECTOR output, VECTOR input0) {
  VECTOR work;

  // Find the length of the vector.
  work[3] = sqrtf(input0[0] * input0[0] + input0[1] * input0[1] + input0[2] * input0[2]);

  // Normalize the vector.
  work[0] = (input0[0] / work[3]);
  work[1] = (input0[1] / work[3]);
  work[2] = (input0[2] / work[3]);
  work[3] = 1.00f;

  // Output the result.
  vector_copy(output, work);

 }

 void vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1) {
  VECTOR work;

  // Calculate the outer product.
  work[0] = (input0[1] * input1[2]) - (input0[2] * input1[1]);
  work[1] = (input0[2] * input1[0]) - (input0[0] * input1[2]);
  work[2] = (input0[0] * input1[1]) - (input0[1] * input1[0]);
  work[3] = 0.00f;

  // Output the result.
  vector_copy(output, work);

 }

 //////////////////////
 // MATRIX FUNCTIONS //
 //////////////////////

 void matrix_copy(MATRIX output, MATRIX input0) {

  // Copy the matrix.
  output[0x00] = input0[0x00];
  output[0x01] = input0[0x01];
  output[0x02] = input0[0x02];
  output[0x03] = input0[0x03];
  output[0x04] = input0[0x04];
  output[0x05] = input0[0x05];
  output[0x06] = input0[0x06];
  output[0x07] = input0[0x07];
  output[0x08] = input0[0x08];
  output[0x09] = input0[0x09];
  output[0x0A] = input0[0x0A];
  output[0x0B] = input0[0x0B];
  output[0x0C] = input0[0x0C];
  output[0x0D] = input0[0x0D];
  output[0x0E] = input0[0x0E];
  output[0x0F] = input0[0x0F];

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
  MATRIX work;

  // Multiply the matrices together.
  work[0x00] = (input0[0x00] * input1[0x00]) + (input0[0x04] * input1[0x01]) + (input0[0x08] * input1[0x02]) + (input0[0x0C] * input1[0x03]);
  work[0x01] = (input0[0x01] * input1[0x00]) + (input0[0x05] * input1[0x01]) + (input0[0x09] * input1[0x02]) + (input0[0x0D] * input1[0x03]);
  work[0x02] = (input0[0x02] * input1[0x00]) + (input0[0x06] * input1[0x01]) + (input0[0x0A] * input1[0x02]) + (input0[0x0E] * input1[0x03]);
  work[0x03] = (input0[0x03] * input1[0x00]) + (input0[0x07] * input1[0x01]) + (input0[0x0B] * input1[0x02]) + (input0[0x0F] * input1[0x03]);
  work[0x04] = (input0[0x00] * input1[0x04]) + (input0[0x04] * input1[0x05]) + (input0[0x08] * input1[0x06]) + (input0[0x0C] * input1[0x07]);
  work[0x05] = (input0[0x01] * input1[0x04]) + (input0[0x05] * input1[0x05]) + (input0[0x09] * input1[0x06]) + (input0[0x0D] * input1[0x07]);
  work[0x06] = (input0[0x02] * input1[0x04]) + (input0[0x06] * input1[0x05]) + (input0[0x0A] * input1[0x06]) + (input0[0x0E] * input1[0x07]);
  work[0x07] = (input0[0x03] * input1[0x04]) + (input0[0x07] * input1[0x05]) + (input0[0x0B] * input1[0x06]) + (input0[0x0F] * input1[0x07]);
  work[0x08] = (input0[0x00] * input1[0x08]) + (input0[0x04] * input1[0x09]) + (input0[0x08] * input1[0x0A]) + (input0[0x0C] * input1[0x0B]);
  work[0x09] = (input0[0x01] * input1[0x08]) + (input0[0x05] * input1[0x09]) + (input0[0x09] * input1[0x0A]) + (input0[0x0D] * input1[0x0B]);
  work[0x0A] = (input0[0x02] * input1[0x08]) + (input0[0x06] * input1[0x09]) + (input0[0x0A] * input1[0x0A]) + (input0[0x0E] * input1[0x0B]);
  work[0x0B] = (input0[0x03] * input1[0x08]) + (input0[0x07] * input1[0x09]) + (input0[0x0B] * input1[0x0A]) + (input0[0x0F] * input1[0x0B]);
  work[0x0C] = (input0[0x00] * input1[0x0C]) + (input0[0x04] * input1[0x0D]) + (input0[0x08] * input1[0x0E]) + (input0[0x0C] * input1[0x0F]);
  work[0x0D] = (input0[0x01] * input1[0x0C]) + (input0[0x05] * input1[0x0D]) + (input0[0x09] * input1[0x0E]) + (input0[0x0D] * input1[0x0F]);
  work[0x0E] = (input0[0x02] * input1[0x0C]) + (input0[0x06] * input1[0x0D]) + (input0[0x0A] * input1[0x0E]) + (input0[0x0E] * input1[0x0F]);
  work[0x0F] = (input0[0x03] * input1[0x0C]) + (input0[0x07] * input1[0x0D]) + (input0[0x0B] * input1[0x0E]) + (input0[0x0F] * input1[0x0F]);

  // Output the result.
  matrix_copy(output, work);

 }

 void matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the z-axis rotation.
  matrix_unit(work);
  work[0x00] =  cosf(input1[2]);
  work[0x01] = -sinf(input1[2]);
  work[0x04] =  sinf(input1[2]);
  work[0x05] =  cosf(input1[2]);
  matrix_multiply(output, work, input0);

  // Apply the y-axis rotation.
  matrix_unit(work);
  work[0x00] =  cosf(input1[1]);
  work[0x02] =  sinf(input1[1]);
  work[0x08] = -sinf(input1[1]);
  work[0x0A] =  cosf(input1[1]);
  matrix_multiply(output, work, output);

  // Apply the x-axis rotation.
  matrix_unit(work);
  work[0x05] =  cosf(input1[0]);
  work[0x06] = -sinf(input1[0]);
  work[0x09] =  sinf(input1[0]);
  work[0x0A] =  cosf(input1[0]);
  matrix_multiply(output, work, output);

 }

 void matrix_scale(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the scale factor.
  matrix_unit(work);
  work[0x00] = input1[0];
  work[0x05] = input1[1];
  work[0x0A] = input1[2];
  matrix_multiply(output, work, input0);

 }

 void matrix_translate(MATRIX output, MATRIX input0, VECTOR input1) {
  MATRIX work;

  // Apply the translation.
  matrix_unit(work);
  work[0x0C] = input1[0];
  work[0x0D] = input1[1];
  work[0x0E] = input1[2];
  matrix_multiply(output, work, input0);

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

  // Create the world_view matrix.
  matrix_unit(world_view);
  matrix_translate(world_view, world_view, translation);
  matrix_rotate(world_view, world_view, rotation);

 }

 void create_view_clip(MATRIX view_clip, float aspect, float left, float right, float bottom, float top, float near, float far) {

  // Apply the aspect ratio adjustment.
  left = (left * aspect); right = (right * aspect);

  // Create the view_clip matrix.
  matrix_unit(view_clip);
  view_clip[0x00] = (2 * near) / (right - left);
  view_clip[0x05] = (2 * near) / (top - bottom);
  view_clip[0x08] = (right + left) / (right - left);
  view_clip[0x09] = (top + bottom) / (top - bottom);
  view_clip[0x0A] = (far + near) / (far - near);
  view_clip[0x0B] = -1.00f;
  view_clip[0x0E] = (2 * far * near) / (far - near);
  view_clip[0x0F] = 0.00f;

 }

 void create_local_clip(MATRIX local_clip, MATRIX local_world, MATRIX world_view, MATRIX view_clip) {

  // Create the local_screen matrix.
  matrix_unit(local_clip);
  matrix_multiply(local_clip, local_world, local_clip);
  matrix_multiply(local_clip, world_view,  local_clip);
  matrix_multiply(local_clip, view_clip,   local_clip);

 }

 void create_clip_screen(MATRIX clip_screen, float max_x, float max_y, float max_z) {

  // Create the clip_screen matrix.
  matrix_unit(clip_screen);
  clip_screen[0x00] = (max_x / 2);
  clip_screen[0x0C] = (max_x / 2);
  clip_screen[0x05] = (max_y / 2);
  clip_screen[0x0D] = (max_y / 2);
  clip_screen[0x0A] = (max_z / 2);
  clip_screen[0x0E] = (max_z / 2);

 }

 void point_calculate(VECTORI *output, VECTOR *input, int count, MATRIX local_clip, MATRIX clip_screen) {
  VECTOR work; int loop0;

  // For each coordinate...
  for (loop0=0;loop0<count;loop0++) {

   // Apply the local_clip matrix.
   vector_multiply(work, input[loop0], local_clip);

   // If the X coordinate is not clipped...
   if (fabsf(work[0]) < fabsf(work[3])) {

    // If the Y coordinate is not clipped...
    if (fabsf(work[1]) < fabsf(work[3])) {

     // If the Z coordinate is not clipped...
     if (fabsf(work[2]) < fabsf(work[3])) {

      // Apply the clip_screen matrix.
      vector_multiply(work, work, clip_screen);

      // Normalize and convert the result.
      output[loop0].x = (work[0] / work[3]) * 16;
      output[loop0].y = (work[1] / work[3]) * 16;
      output[loop0].z = (work[2] / work[3]);
      output[loop0].w = work[3];

     // Else, set the coordinate as clipped.
     } else { output[loop0].w = 0.00f; }

    // Else, set the coordinate as clipped.
    } else { output[loop0].w = 0.00f; }

   // Else, set the coordinate as clipped.
   } else { output[loop0].w = 0.00f; }

  }

 }

 void point_calculate_vu0(VECTORI *output, VECTOR *input, int count, MATRIX local_clip, MATRIX clip_screen) {
  asm __volatile__ (
   "lqc2		vf01, 0x00(%3)		\n"
   "lqc2		vf02, 0x10(%3)		\n"
   "lqc2		vf03, 0x20(%3)		\n"
   "lqc2		vf04, 0x30(%3)		\n"
   "lqc2		vf05, 0x00(%4)		\n"
   "lqc2		vf06, 0x10(%4)		\n"
   "lqc2		vf07, 0x20(%4)		\n"
   "lqc2		vf08, 0x30(%4)		\n"
   "1:						\n" // 1: BEGIN LOOP
   "lqc2		vf09, 0x00(%1)		\n"
   "vmulax.xyzw		ACC, vf01, vf09		\n"
   "vmadday.xyzw	ACC, vf02, vf09		\n"
   "vmaddaz.xyzw	ACC, vf03, vf09		\n"
   "vmaddw.xyzw		vf10, vf04, vf09	\n"
   "vclipw.xyz		vf10, vf10		\n"
   "cfc2		$10, $18		\n"
   "beq			$10, $0, 3f		\n"
   "2:						\n" // 2: CLIP FAILED
   "sqc2		vi00, 0x00(%0)		\n"
   "j			4f			\n"
   "3:						\n" // 3: CLIP PASSED
   "vmulax.xyzw		ACC, vf05, vf10		\n"
   "vmadday.xyzw	ACC, vf06, vf10		\n"
   "vmaddaz.xyzw	ACC, vf07, vf10		\n"
   "vmaddw.xyzw		vf11, vf08, vf10	\n"
   "vdiv		Q, vf0w, vf11w		\n"
   "vwaitq					\n"
   "vmulq.xyz		vf11, vf11, Q		\n"
   "vftoi4.xy		vf12, vf11		\n"
   "vftoi0.z		vf12, vf11		\n"
   "vmove.w		vf12, vf11		\n"
   "sqc2		vf12, 0x00(%0)		\n"
   "4:						\n" // 4: END LOOP
   "addi		%0, 0x10		\n"
   "addi		%1, 0x10		\n"
   "addi		%2, -1			\n"
   "bne			$0, %2, 1b		\n"
   : : "r" (output), "r" (input), "r" (count), "r" (local_clip), "r" (clip_screen) : "$10"
  );

 }
