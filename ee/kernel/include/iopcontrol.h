/*      
  _____     ___ ____ 
   ____|   |    ____|      PSX2 OpenSource Project
  |     ___|   |____       (c) 2003 Marcus R. Brown (mrbrown@0xd6.org)
  ------------------------------------------------------------------------
  iopcontrol.h
                           IOP reset and status routines.
*/

#ifndef IOPCONTROL_H
#define IOPCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 Resets IOP, arg can be NULL or a module that will be loaded afterwards.
 @param arg a const character pointer for path to module or NULL
 @param mode 0x100 for magicgate anything else for no magicgate. 
 @return 1 for success or 0 for failure.
 */
int SifIopReset(const char *arg, int mode);
/*!
 @return 1 for success or 0 for failure.
 */ 
int SifIopIsAlive(void);
/*!
 @return 1 for success or 0 for failure.
 */ 
int SifIopSync(void);

/*!
 This is retained for compatibility.
 @return 1 for success or 0 for failure.
 */
int SifResetIop();

#ifdef __cplusplus
}
#endif

#endif /* IOPCONTROL_H */
