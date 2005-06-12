  This erl-loader is here to serve as a "core" system for any erl-based
  application. You can either use it bare, or take the code and put it into
  your own application. Its design is to keep the less possible symbols in it,
  but there's still quite a lot.
  
  So, now, you can design your erl-based application this way: create a
  main.erl file (or any other name you want) with a main() function in it.
  Don't forget to put the right dependancies in it (read the samples in the erl
  directory below to get an idea on how to do all of that), then simply run
  erl-loader without argument (or with the name of your erl file if it's not
  "main.erl")
  
  Try it with the samples. Put erl-loader.elf, hello.erl, libc.erl and
  libkernel.erl in the same location, and then, launch erl-loader.elf using
  ps2client for example:
  
    ps2client host:erl-loader.elf hello.erl

  This should automagically load hello.erl, libc.erl and libkernel.erl.
  
  Have fun.
