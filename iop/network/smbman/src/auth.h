/*
  Copyright 2009-2010, jimmikaelkael
  Licenced under Academic Free License version 3.0
*/

#ifndef __AUTH_H__
#define __AUTH_H__

// function prototypes
extern unsigned char *LM_Password_Hash(const unsigned char *password, unsigned char *cipher);
extern unsigned char *NTLM_Password_Hash(const unsigned char *password, unsigned char *cipher);
extern unsigned char *LM_Response(const unsigned char *LMhash, unsigned char *chal, unsigned char *cipher);

#endif
