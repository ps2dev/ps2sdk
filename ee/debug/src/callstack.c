/*
 * $Xorg: getretmips.c,v 1.4 2001/02/09 02:06:19 xorgcvs Exp $
 *
Copyright 1992, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 * cleaned up slighly by emoon and added ee related opcodes and checking.
 */

/* Return stack generation for MIPS processors
 * This is tricky as MIPS stack frames aren't
 * easily unrolled -- we look for pc restoration
 * and stack adjustment instructions beyond the return
 * address to discover the correct values
 */

/* lw $31,const($sp) is : 100 011 11101 11111 const */
/*                        1000 1111 1011 1111       */

#define RESTORE_RETURNVAL	0x8fbf0000
#define RESTORE_RETURNVAL_MASK	0xffff0000

/* ld $31,const($sp) is : 110 111 11101 11111 const */
/*                        1101 1111 1011 1111       */

#define RESTORE_RETURNVAL2	0xdfbf0000

/* lq $31,const($sp) is : 011110 11101 11111 const */
/* ee Related             0111 1011 1011 1111      */

#define RESTORE_RETURNVAL3  0x7bbf0000

/* addiu $sp, $sp, const is 001 001 11101 11101 const */
/*                          0010 0111 1011 1101 const */

#define ADJUST_STACKP_C		0x27bd0000
#define ADJUST_STACKP_C_MASK	0xffff0000

/* addu $sp, $sp, $at is 000 000 11101 00001 11101 00000 100 001  */
/*                       0000 0011 1010 0001 1110 1000 0010 0001 */

#define ADJUST_STACKP_V		0x03a1e821
#define ADJUST_STACKP_V_MASK	0xffffffff

/* lui $at, const is 001 111 00000 00001 const */
/*		     0011 1100 0000 0001 const */

#define SET_UPPER_C		0x3c010000
#define SET_UPPER_C_MASK	0xffff0000

/* ori $at, $at, const is 001 101 00001 00001 const */
/*                        0011 0100 0010 0001 const */

#define OR_LOWER_C		0x34210000
#define OR_LOWER_C_MASK		0xffff0000

/* ori $at, $zero, const is 001 101 00000 00001 const */
/*                          0011 0100 0000 0001 const */

#define SET_LOWER_C		0x34010000
#define SET_LOWER_C_MASK	0xffff0000

/* jr $ra */
#define RETURN	    0x03e00008

#define CALL(f)	    (0x0c000000 | (((int) (f)) >> 2))

/*
 * This computation is expensive, so we cache the results;
 * a simple hash function and straight-forward replacement.
 */

#define HASH_SIZE   256

typedef struct _returnCache 
{
    unsigned int   *returnAddress;
    int	    raOffset;
    int	    spAdjust;
} ReturnCacheRec, *ReturnCachePtr;

static ReturnCacheRec	returnCache[HASH_SIZE];

#define HASH(ra)    ((((int) (ra)) >> 2) & (HASH_SIZE - 1))

typedef int Bool;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern unsigned int* ps2GetReturnAddress();
extern unsigned int* ps2GetStackPointer();
extern int  main();

void ps2GetStackTrace(unsigned int* results,int max)
{
  unsigned int*   ra;
  unsigned int*   ra_limit;
  unsigned int*   sp;
  unsigned int    inst;
  unsigned int    mainCall;
  unsigned short  const_upper;
  unsigned short  const_lower;
  int ra_offset;
  int sp_adjust;
  Bool found_ra_offset, found_sp_adjust;
  Bool found_const_upper, found_const_lower;
  ReturnCachePtr  rc;
  
  ra = ps2GetReturnAddress();
  sp = ps2GetStackPointer();
  mainCall = CALL(main);
  
  while (ra && max) 
  {
    rc = &returnCache[HASH(ra)];
    if (rc->returnAddress != ra)
    {
      found_ra_offset = FALSE;
      found_sp_adjust = FALSE;
      found_const_upper = FALSE;
      found_const_lower = FALSE;
      const_upper = 0;
      const_lower = 0;
      rc->returnAddress = ra;
      ra_limit = (unsigned int *) 0x200000;
      ra_offset = 0;
      sp_adjust = -1;
      
      while ((!found_ra_offset || !found_sp_adjust) && ra < ra_limit)
      {
        inst = *ra;
        /* look for the offset of the PC in the stack frame */
        if ((inst & RESTORE_RETURNVAL_MASK) == RESTORE_RETURNVAL)
        {
          ra_offset = inst & ~RESTORE_RETURNVAL_MASK;
          found_ra_offset = TRUE;
        }
        else if ((inst & RESTORE_RETURNVAL_MASK) == RESTORE_RETURNVAL2)
        {
            ra_offset = inst & ~RESTORE_RETURNVAL_MASK;
            found_ra_offset = TRUE;
        }
        else if ((inst & RESTORE_RETURNVAL_MASK) == RESTORE_RETURNVAL3)
        {
            ra_offset = inst & ~RESTORE_RETURNVAL_MASK;
            found_ra_offset = TRUE;
        }
        else if ((inst & ADJUST_STACKP_C_MASK) == ADJUST_STACKP_C)
        {
          sp_adjust = inst & ~ADJUST_STACKP_C_MASK;
          found_sp_adjust = TRUE;
        }
        else if ((inst & ADJUST_STACKP_V_MASK) == ADJUST_STACKP_V)
        {
          sp_adjust = 0;
          found_sp_adjust = TRUE;
        }
        else if ((inst & SET_UPPER_C_MASK) == SET_UPPER_C)
        {
          const_upper = inst & ~SET_UPPER_C_MASK;
          const_lower = 0;
          found_const_upper = TRUE;
        }
        else if ((inst & OR_LOWER_C_MASK) == OR_LOWER_C)
        {
          const_lower = inst & ~OR_LOWER_C_MASK;
          found_const_lower = TRUE;
        }
        else if ((inst & SET_LOWER_C_MASK) == SET_LOWER_C)
        {
          const_lower = inst & ~SET_LOWER_C_MASK;
          const_upper = 0;
          found_const_lower = TRUE;
        }
        else if (inst == RETURN)
          ra_limit = ra + 2;
          ra++;
      }
      
      if (sp_adjust == 0 && (found_const_upper || found_const_lower))
        sp_adjust = (const_upper << 16) | const_lower;
        rc->raOffset = ra_offset;
        rc->spAdjust = sp_adjust;
      }
      /* if something went wrong, punt */
      if (rc->spAdjust <= 0) 
      {
        *results++ = 0;
        break;
      }
            
      ra = (unsigned int *) sp[rc->raOffset>>2];
      sp += rc->spAdjust >> 2;
      
      if (ra == 0)
      {
        *results++ = 0;
        break;
      }

      *results++ = ((unsigned int) ra) - 8;
      if (ra[-2] == mainCall)
      {
        *results++ = 0;
        break;
      }
    max--;
  }
}

