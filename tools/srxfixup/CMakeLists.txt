add_executable(
  srxfixup
  src/srxfixup.c
  src/anaarg.c
  src/ring.c
  src/swapmem.c
  src/elflib.c
  src/elfdump.c
  src/mipsdis.c
  src/srxgen.c
  src/readconf.c
  src/iopfixconf.c
  src/eefixconf.c)

target_include_directories(srxfixup PRIVATE ./include)


# TODO make multiple copies and install
set(SRX_ADDITONAL_BIN eefixup erx-strip iopfixup irx-strip)
