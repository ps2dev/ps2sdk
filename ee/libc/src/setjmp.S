/* This is a simple version of setjmp and longjmp.

   This version does NOT save the floating point register, which is
   wrong, but I don't know how to cleanly handle machines without a
   floating point coprocessor.

   Ian Lance Taylor, Cygnus Support, 13 May 1993.  */

#define O_S0	0
#define O_S1	16
#define O_S2	32
#define O_S3	48
#define O_S4	64
#define O_S5	80
#define O_S6	96
#define O_S7	112
#define O_S8	128
#define O_SP	144
#define O_RA	160

/* int setjmp (jmp_buf);  */
	.globl	setjmp
	.ent	setjmp
	.weak	setjmp
setjmp:
	.frame	$sp,0,$31

	sq	$16,O_S0($4)	/* $s0 */
	sq	$17,O_S1($4)	/* $s1 */
	sq	$18,O_S2($4)	/* $s2 */
	sq	$19,O_S3($4)	/* $s3 */
	sq	$20,O_S4($4)	/* $s4 */
	sq	$21,O_S5($4)	/* $s5 */
	sq	$22,O_S6($4)	/* $s6 */
	sq	$23,O_S7($4)	/* $s7 */
	sq	$30,O_S8($4)	/* $s8 */

	sq	$sp,O_SP($4)
	sq	$31,O_RA($4)

	move	$2,$0

	j	$31

	.end	setjmp

/* volatile void longjmp (jmp_buf, int);  */
	.globl	longjmp
	.ent	longjmp
	.weak	longjmp
longjmp:
	.frame	$sp,0,$31

	lq	$16,O_S0($4)	/* $s0 */
	lq	$17,O_S1($4)	/* $s1 */
	lq	$18,O_S2($4)	/* $s2 */
	lq	$19,O_S3($4)	/* $s3 */
	lq	$20,O_S4($4)	/* $s4 */
	lq	$21,O_S5($4)	/* $s5 */
	lq	$22,O_S6($4)	/* $s6 */
	lq	$23,O_S7($4)	/* $s7 */
	lq	$30,O_S8($4)	/* $s8 */

	lq	$sp,O_SP($4)
	lq	$31,O_RA($4)

	bne	$5,$0,1f
	li	$5,1
1:
	move	$2,$5

	j	$31

	.end longjmp
