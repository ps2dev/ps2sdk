Sample program to show how to use the `gprof` feature.

The requiremnts are quite easy, just adding `-g -pg` flags to the `EE_CFLAGS` and `EE_LDFLAGS` is enough to make things to work out of the box.

Firstly execute your program, then once program ends it will automatically generates a `gmon.out` file at CWD level.

In order to inspect the content of the generated file you need to use the `mips64r5900el-ps2-elf-gprof` binary.

For instance, following the next syntax:
```
mips64r5900el-ps2-elf-gprof -b {binary.elf} gmon.out
```

like:
```
mips64r5900el-ps2-elf-gprof -b gprofbasic.elf gmon.out
```


It will show something like:
```
Flat profile:

Each sample counts as 0.001 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 83.26      0.19     0.19   104728     0.00     0.00  is_prime
 16.74      0.23     0.04        1    39.00    39.00  dummy_function
  0.00      0.23     0.00        1     0.00   233.00  main
  0.00      0.23     0.00        1     0.00   194.00  sum_of_square_roots


			Call graph


granularity: each sample hit covers 2 byte(s) for 0.43% of 0.23 seconds

index % time    self  children    called     name
                0.00    0.23       1/1           _ftext [2]
[1]    100.0    0.00    0.23       1         main [1]
                0.00    0.19       1/1           sum_of_square_roots [4]
                0.04    0.00       1/1           dummy_function [5]
-----------------------------------------------
                                                 <spontaneous>
[2]    100.0    0.00    0.23                 _ftext [2]
                0.00    0.23       1/1           main [1]
-----------------------------------------------
                0.19    0.00  104728/104728      sum_of_square_roots [4]
[3]     83.3    0.19    0.00  104728         is_prime [3]
-----------------------------------------------
                0.00    0.19       1/1           main [1]
[4]     83.3    0.00    0.19       1         sum_of_square_roots [4]
                0.19    0.00  104728/104728      is_prime [3]
-----------------------------------------------
                0.04    0.00       1/1           main [1]
[5]     16.7    0.04    0.00       1         dummy_function [5]
-----------------------------------------------


Index by function name

   [5] dummy_function          [1] main
   [3] is_prime                [4] sum_of_square_roots
```

Cheers