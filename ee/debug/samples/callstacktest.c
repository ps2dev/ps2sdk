/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <stdio.h>
#include <tamtypes.h>
#include <debug.h>

////////////////////////////////////////////////////////////////////////
// A bunch of test function that we use to test the stacktrace.

int TestFunction3()
{
  unsigned int* stackTrace[256];
  int i = 0;

  ps2GetStackTrace((unsigned int*)&stackTrace,256);

  for(i = 0; i < 256; ++i)
  {
    if (stackTrace[i] == 0)
      break;

    printf("adress %d 0x%08x\n",i,(int)stackTrace[i]);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////

int TestFunction2()
{
  return TestFunction3();
}

////////////////////////////////////////////////////////////////////////

int testFunction1()
{
  return TestFunction2();
}

////////////////////////////////////////////////////////////////////////

int main()
{
  testFunction1();

  while(1)
  {
  }

  return 0;
}
