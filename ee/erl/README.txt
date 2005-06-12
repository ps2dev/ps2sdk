  As unusual, let's have some bits of documentation here.
  
  An ERL file is a relocatable ELF. It can be directly generated from ee-ld
  using some switches. The ERL file isn't loadable directly, from the PS2
  kernel. It acts as a DLL file, for those who are used to windows API. To be
  exact, a .erl file is just the same as a .o file. Think of it as a "merged"
  .o file, except it does have some extra code here and there.
  
  The actual relocations supported are:
  
    R_MIPS_26
    R_MIPS_32
    R_MIPS_HI16
    R_MIPS_LO16

  The GP relocation isn't supported, which means the code you want to load
  shouldn't use the $gp register at all (that is, be compiled with the -G0
  switch.) because I don't want to handle real-time GP section memory
  allocation...
  
  Also, weak symbols are just considered the same way as global symbols.
  Furthermore, during symbol collision, the oldest symbol always win. Please
  note that symbol collision doesn't generate an error, only a warning if the
  loader is compiled in DEBUG mode.
  
  In order to compile your code into an erl file, you simply have to do:
  
    ee-gcc -Wl,-r -G0 -nostartfiles -o my_lib.erl file1.o file2.o ...

  That will create an elf file which is readable thru the functions in the erl
  system. The -Wl,-r is to tell ee-ld to do "incremental linking".
  
  If you want to strip it, the best way to do it without breaking it is:
  
    ee-strip --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment my.erl
  
  
  When loading an erl, the loader will parse the headers of the file, malloc()
  a buffer to hold the code/data, and then, will relocate and export all the
  symbols it can find.
  
  For example, if your code is the following:
  
    void hello(void) {
	printf("Hello world!\n");
    }

  Your erl file will contain an exported "hello" symbol, and when the erl gets
  loaded, the loader will look for the "printf" symbol into all the previously
  exported symbols. If it doesn't find it, it will simply put it into a pool of
  "loosy relocations". When a new symbol is exported, the code checks in the
  loosy pool if it was used here. If it does, the code will immediately fix the
  relocation. That means you can load two inter dependant erl files. But be
  careful: you won't be able to unload cyclic-dependant erl files.
  
  Remember one thing: if you don't want a symbol to be exported, it has to be
  explicitely defined as 'static'.
  
  It also mean that some symbols may be not relocated. So, if your dependancy
  tree isn't good, or for a huge lot of various reasons, you might end up with
  unrelocated pieces of code, which might result into things like "jal 0".
  
  Note that you can actually do things like:
  
    void some_imported_function(int);
    
    if (some_imported_function) {
	some_imported_function(x);
    }

  
  Your code can export whatever symbols it want, but there's only a small list
  of specific symbols:
  
    erl_id -- should be a char * string, containing the current erl name.
    erl_dependancies -- should be a char ** string array, containing a list of
                        the dependancies of that erl file.
    _start -- basically acts the same as the IRX's _start function.

  They are optionnal, but are highly recommanded, especially erl_id.


  Since your "host" binary (that is, the one which load the erl files) isn't
  an erl file itself, there's no automatic export done here. So, if you want
  to run the previous erl file, you first have to register "printf" so that
  it would be able to work. The actual way to export symbols from your main
  binary is thru the erl_add_global_symbol() function.
    
    
  About general loading/unloading: when you load or unload an erl file, it
  will look the erl_dependancies strings arrays, and call the _init_load_erl()
  function on each of them. This is a function pointer which, by default,
  points to _init_load_erl_from_file(). Don't hesitate to change it if you need
  to have a different "module loading" behavior.

  The _init_load_erl_from_file() is a wrapper that will check if the erl isn't
  already loaded, and then, prefix the global "_init_erl_prefix" string to the
  filename. That path should contain the trailing slash.
  
  So, if in the dependancy strings, you have "libc", it will try to load the
  "libc.erl" file located in the erl prefix.
  
  During module unloading, the dependancies are unloaded in reverse order.
  Module unloading won't occur if there are other dependancies on it, or if the
  ERL_FLAG_STICKY flag has been put in the flag field of the erl_record.
  
  
  Something else: during loading, the _init function, if it exists, will be
  called, and during unloading, the _fini function is called. These two
  functions are not to be exported by you. The code here is introduced by gcc
  and ld "automagically" from the various crt*.o files that are in the gcc
  directory: /usr/local/ps2dev/ee/lib/gcc-lib/ee/3.2.2
  
  Even if some basic test code was run, there might be some problems linked to
  the usage of non-ps2dev specific code. And don't even think about not having
  it: it is necessary to call the main() function, or to process ctor/dtor
  section if you have some.
  
  
  Some symbols will also never be exported, such as the erl_id, or
  erl_dependancies, but erl_copyright and erl_version as well. You can still
  access them using the local symbol query function though.
  
  
  Also, if you are sure that a loaded module (say, a plugin) won't be useful
  anymore, you can flush its symbols table. That way, all the memory took by
  the various strings, hash tables and other things will be freed.


  Finally, as a kind of "debug" purpose, you can "resolve" an address in
  memory, so that it tells you if it belongs to a loaded erl file, and if yes
  which one.



  Last thing worth mentionning: that code uses some hashing table stuff from
  somebody else: http://burtleburtle.net/bob/hash/hashtab.html
  
  The functions herein his code are quite useful imho, it's worth having a
  look at them. Please not that his functions are globally exported, so, you
  may use them directly if you use the liberl.a file.
