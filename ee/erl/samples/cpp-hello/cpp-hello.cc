/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Hello sample
*/

#include <stdio.h>
#include <erl.h>

char * erl_id = "cpp-hello";
char * erl_dependancies[] = {
    "libc",
    0
};

class cpp_hello {
  public:
      cpp_hello();
      ~cpp_hello();
};

cpp_hello my_cpp_hello;

cpp_hello::cpp_hello() {
    printf("Hello world, from constructor.\n");
}

cpp_hello::~cpp_hello() {
    printf("Hello world, from destructor.\n");
}

extern "C" {
int _start(int argc, char ** argv) {
    return 1;
}
}
