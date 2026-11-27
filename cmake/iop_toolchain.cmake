#
# CMake platform file for PS2 EE processor
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
# Copyright (C) 2023 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Copyright (C) 2024 André Guilherme <andregui17@outlook.com>
# Copyright (C) 2024-Present PS2DEV Team
#

cmake_minimum_required(VERSION 3.0)

INCLUDE(CMakeForceCompiler)
if(DEFINED ENV{PS2SDK})
    SET(PS2SDK $ENV{PS2SDK})
else()
    message(FATAL_ERROR "The environment variable PS2SDK needs to be defined.")
endif()

if(DEFINED ENV{PS2DEV})
    SET(PS2DEV $ENV{PS2DEV})
else()
    message(FATAL_ERROR "The environment variable PS2DEV needs to be defined.")
endif()

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR mips)

SET(CMAKE_C_COMPILER mips64r5900el-ps2-elf-gcc)
SET(CMAKE_CXX_COMPILER mips64r5900el-ps2-elf-g++)

SET(EE_CFLAGS "-I$ENV{PS2SDK}/ee/include -I$ENV{PS2SDK}/common/include -I$ENV{PS2SDK}/ports/include -D_EE -DPS2 -D__PS2__ -O2 -G0" CACHE STRING "EE C compiler flags" FORCE)
SET(EE_LDFLAGS "-L$ENV{PS2SDK}/ee/lib -L$ENV{PS2DEV}/gsKit/lib -L$ENV{PS2SDK}/ports/lib -Wl,-zmax-page-size=128 -T$ENV{PS2SDK}/ee/startup/linkfile" CACHE STRING "EE linker flags" FORCE)

SET(CMAKE_TARGET_INSTALL_PREFIX $ENV{PS2DEV}/ports)

SET(CMAKE_FIND_ROOT_PATH $ENV{PS2DEV} $ENV{PS2DEV}/ee $ENV{PS2DEV}/ee/ee $ENV{PS2SDK} $ENV{PS2SDK}/ports)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

SET(CMAKE_C_FLAGS_INIT ${EE_CFLAGS})
SET(CMAKE_CXX_FLAGS_INIT ${EE_CFLAGS})
SET(CMAKE_EXE_LINKER_FLAGS_INIT ${EE_LDFLAGS})


SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-nostartfiles -Wl,-r -Wl,-d")

SET(PS2 TRUE)
SET(PLATFORM_PS2 TRUE)
SET(EE TRUE)
