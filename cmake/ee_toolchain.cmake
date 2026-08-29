#
# CMake platform file for PS2 EE processor
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
# Copyright (C) 2023 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Copyright (C) 2024 André Guilherme <andregui17@outlook.com>
# Copyright (C) 2024-Present PS2DEV Team
#

cmake_minimum_required(VERSION 3.10)

INCLUDE(CMakeForceCompiler)


SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR mips)

SET(CMAKE_ASM_COMPILER mips64r5900el-ps2-elf-gcc)
SET(CMAKE_C_COMPILER mips64r5900el-ps2-elf-gcc)
SET(CMAKE_CXX_COMPILER mips64r5900el-ps2-elf-g++)

find_program(CMAKE_OBJCOPY mips64r5900el-ps2-elf-objcopy)

SET(EE_CFLAGS "-D_EE -G0 -O2 -Wall -Werror -gdwarf-2 -gz" CACHE STRING "EE C compiler flags" FORCE)
SET(EE_LDFLAGS "-Wl,-zmax-page-size=128" CACHE STRING "EE linker flags" FORCE)

SET(CMAKE_TARGET_INSTALL_PREFIX $ENV{PS2DEV}/ports)

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

SET(CMAKE_C_FLAGS_INIT ${EE_CFLAGS})
SET(CMAKE_CXX_FLAGS_INIT ${EE_CFLAGS})
SET(CMAKE_EXE_LINKER_FLAGS_INIT ${EE_LDFLAGS})


SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-nostartfiles -Wl,-r -Wl,-d")

SET(PS2 TRUE)
SET(PLATFORM_PS2 TRUE)
SET(EE TRUE)
