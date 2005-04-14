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

 #include <math.h>
 #include <math3d.h>
 #include "../../math/include/protos.h"

 //////////////////////
 // VECTOR FUNCTIONS //
 //////////////////////

 int vector_copy(VECTOR output, VECTOR input0) {

  // Copy the vector.
  output[0] = input0[0];
  output[1] = input0[1];
  output[2] = input0[2];
  output[3] = input0[3];

  // End function.
  return 0;

 }

 int vector_multiply(VECTOR output, VECTOR input0, MATRIX input1) { VECTOR work;

  // Multiply the vector by the matrix.
  work[0] = (input0[0] * input1[0][0]) + (input0[1] * input1[0][1]) + (input0[2] * input1[0][2]) + (input0[3] * input1[0][3]);
  work[1] = (input0[0] * input1[1][0]) + (input0[1] * input1[1][1]) + (input0[2] * input1[1][2]) + (input0[3] * input1[1][3]);
  work[2] = (input0[0] * input1[2][0]) + (input0[1] * input1[2][1]) + (input0[2] * input1[2][2]) + (input0[3] * input1[2][3]);
  work[3] = (input0[0] * input1[3][0]) + (input0[1] * input1[3][1]) + (input0[2] * input1[3][2]) + (input0[3] * input1[3][3]);

  // Output the result.
  vector_copy(output, work);

  // End function.
  return 0;

 }

 int vector_normalize(VECTOR output, VECTOR input0) {
  float length;

  // Find the length of the vector.
  length = sqrtf(input0[0] * input0[0] + input0[1] * input0[1] + input0[2] * input0[2]);

  // Normalize the vector.
  output[0] = input0[0] / length;
  output[1] = input0[1] / length;
  output[2] = input0[2] / length;
  output[3] = 1.00f;

  // End function.
  return 0;

 }

 int vector_outerproduct(VECTOR output, VECTOR input0, VECTOR input1) { VECTOR work;

  // Calculate the outer product of the vectors.
  work[0] = (input0[1] * input1[2]) - (input0[2] * input1[1]);
  work[1] = (input0[2] * input1[0]) - (input0[0] * input1[2]);
  work[2] = (input0[0] * input1[1]) - (input0[1] * input1[0]);
  work[3] = 0.00f;

  // Output the result.
  vector_copy(output, work);

  // End function.
  return 0;

 }

 //////////////////////
 // MATRIX FUNCTIONS //
 //////////////////////

 int matrix_copy(MATRIX output, MATRIX input0) {

  // Copy the matrix.
  output[0][0] = input0[0][0]; output[0][1] = input0[0][1]; output[0][2] = input0[0][2]; output[0][3] = input0[0][3];
  output[1][0] = input0[1][0]; output[1][1] = input0[1][1]; output[1][2] = input0[1][2]; output[1][3] = input0[1][3];
  output[2][0] = input0[2][0]; output[2][1] = input0[2][1]; output[2][2] = input0[2][2]; output[2][3] = input0[2][3];
  output[3][0] = input0[3][0]; output[3][1] = input0[3][1]; output[3][2] = input0[3][2]; output[3][3] = input0[3][3];

  // End function.
  return 0;

 }

 int matrix_inverse(MATRIX output, MATRIX input0) { MATRIX work;

  // Calculate the inverse of the matrix.
  matrix_transpose(work, input0);
  work[0][3] = -(input0[0][3] * work[0][0] + input0[1][3] * work[0][1] + input0[2][3] * work[0][2]);
  work[1][3] = -(input0[0][3] * work[1][0] + input0[1][3] * work[1][1] + input0[2][3] * work[1][2]);
  work[2][3] = -(input0[0][3] * work[2][0] + input0[1][3] * work[2][1] + input0[2][3] * work[2][2]);
  work[3][0] = 0.00f; work[3][1] = 0.00f; work[3][2] = 0.00f; work[3][3] = 1.00f;

  // Output the result.
  matrix_copy(output, work);

  // End function.
  return 0;

 }

 int matrix_multiply(MATRIX output, MATRIX input0, MATRIX input1) { MATRIX work;
  int loop0, loop1;

  // Multiply the matrix by a matrix.
  for (loop0=0;loop0<4;loop0++) {
   for (loop1=0;loop1<4;loop1++) {
    work[loop0][loop1] =
     (input0[loop0][0] * input1[0][loop1]) + (input0[loop0][1] * input1[1][loop1]) +
     (input0[loop0][2] * input1[2][loop1]) + (input0[loop0][3] * input1[3][loop1]);
   }
  }

  // Output the result.
  matrix_copy(output, work);

  // End function.
  return 0;

 }

 int matrix_rotate(MATRIX output, MATRIX input0, VECTOR input1) {

  // Apply the rotation to the matrix.
  matrix_rotate_z(output, input0, input1[2]);
  matrix_rotate_y(output, output, input1[1]);
  matrix_rotate_x(output, output, input1[0]);

  // End function.
  return 0;

 }

 int matrix_rotate_x(MATRIX output, MATRIX input0, float rx) { MATRIX work;

  // Create the rotation matrix.
  matrix_unit(work);
  work[1][1] =  cosf(rx);
  work[1][2] = -sinf(rx);
  work[2][1] =  sinf(rx);
  work[2][2] =  cosf(rx);

  // Apply the rotation matrix.
  matrix_multiply(output, work, input0);

  // End function.
  return 0;

 }

 int matrix_rotate_y(MATRIX output, MATRIX input0, float ry) { MATRIX work;

  // Create the rotation matrix.
  matrix_unit(work);
  work[0][0] =  cosf(ry);
  work[2][0] =  sinf(ry);
  work[0][2] = -sinf(ry);
  work[2][2] =  cosf(ry);

  // Apply the rotation matrix.
  matrix_multiply(output, work, input0);

  // End function.
  return 0;

 }

 int matrix_rotate_z(MATRIX output, MATRIX input0, float rz) { MATRIX work;

  // Create the rotation matrix.
  matrix_unit(work);
  work[0][0] =  cosf(rz);
  work[0][1] = -sinf(rz);
  work[1][0] =  sinf(rz);
  work[1][1] =  cosf(rz);

  // Apply the rotation matrix.
  matrix_multiply(output, work, input0);

  // End function.
  return 0;

 }

 int matrix_scale(MATRIX output, MATRIX input0, VECTOR input1) { MATRIX work;

  // Create the scale matrix.
  matrix_unit(work);
  work[0][0] = input1[0];
  work[1][1] = input1[1];
  work[2][2] = input1[2];

  // Apply the scale matrix.
  matrix_multiply(output, work, input0);

  // End function.
  return 0;

 }

 int matrix_translate(MATRIX output, MATRIX input0, VECTOR input1) { MATRIX work;

  // Create the translation matrix.
  matrix_unit(work);
  work[0][3] = input1[0];
  work[1][3] = input1[1];
  work[2][3] = input1[2];

  // Apply the translation matrix.
  matrix_multiply(output, work, input0);

  // End function.
  return 0;

 }

 int matrix_transpose(MATRIX output, MATRIX input0) { MATRIX work;
  int loop0, loop1;

  // Transpose the matrix.
  for (loop0=0;loop0<4;loop0++) {
   for (loop1=0;loop1<4;loop1++) {
    work[loop0][loop1] = input0[loop1][loop0];
   }
  }

  // Output the result.
  matrix_copy(output, work);

  // End function.
  return 0;

 }

 int matrix_unit(MATRIX output) {

  // Create a unit matrix.
  output[0][0] = 1.00f; output[0][1] = 0.00f; output[0][2] = 0.00f; output[0][3] = 0.00f;
  output[1][0] = 0.00f; output[1][1] = 1.00f; output[1][2] = 0.00f; output[1][3] = 0.00f;
  output[2][0] = 0.00f; output[2][1] = 0.00f; output[2][2] = 1.00f; output[2][3] = 0.00f;
  output[3][0] = 0.00f; output[3][1] = 0.00f; output[3][2] = 0.00f; output[3][3] = 1.00f;

  // End function.
  return 0;

 }

 ////////////////////////
 // PIPELINE FUNCTIONS //
 ////////////////////////

 int create_local_world(MATRIX local_world, VECTOR translation, VECTOR rotation) {

  // Create the local_world matrix.
  matrix_unit(local_world);
  matrix_rotate(local_world, local_world, rotation);
  matrix_translate(local_world, local_world, translation);

  // End function.
  return 0;

 }

 int create_world_view(MATRIX world_view, VECTOR translation, VECTOR rotation) {
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

  // End function.
  return 0;

 }

 int create_view_screen(MATRIX view_screen, int max_x, int max_y, int max_z, int aspect_x, int aspect_y, float scrz, float nearz, float farz) { MATRIX work;
  float zmin = 1.00f;

  //
  matrix_unit(view_screen);
  view_screen[0][0] = scrz;
  view_screen[1][1] = scrz;
  view_screen[2][2] = 0.00f;
  view_screen[3][3] = 0.00f;
  view_screen[3][2] = 1.00f;
  view_screen[2][3] = 1.00f;

  //
  matrix_unit(work);
  work[0][0] = (float)((float)aspect_y / (float)aspect_x * (float)max_x / (float)max_y);
  work[1][1] = 1.00f;
  work[2][2] = farz * nearz * (-zmin + max_z) / (-nearz + farz);
  work[0][3] = 2048.0f; // Center of PS2 primitive space. (0 - 4096)
  work[1][3] = 2048.0f; // Center of PS2 primitive space. (0 - 4096)
  work[2][3] = (-max_z * nearz + zmin * farz) / (-nearz + farz);

  // Combine and output the result.
  matrix_multiply(view_screen, work, view_screen);

  // End function.
  return 0;

 }

 int create_local_screen(MATRIX local_screen, MATRIX local_world, MATRIX world_view, MATRIX view_screen) { MATRIX work;

  // Create the local_screen matrix.
  matrix_multiply(work, world_view, local_world);
  matrix_multiply(local_screen, view_screen, work);

  // End function.
  return 0;

 }

 int point_calculate(VECTORI *output, MATRIX local_screen, VECTOR *input, int count) { VECTOR work;
  float divw; int loop0;

  // For each point...
  for (loop0=0;loop0<count;loop0++) {

   // Multiply by the local_screen matrix.
   vector_multiply(work, input[loop0], local_screen);

   // Convert the result to INT(12:4).
   divw = 1 / work[3];
   output[loop0][0] = (int)(work[0] * divw) << 4;
   output[loop0][1] = (int)(work[1] * divw) << 4;
   output[loop0][2] = (int)(work[2] * divw);
   output[loop0][3] = 1.00f;

  }

  // End function.
  return 0;

 }
