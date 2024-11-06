#
# CMake platform file for PS2 IOP processor
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
SET(CMAKE_C_COMPILER mipsel-ps2-irx-gcc)
SET(CMAKE_CXX_COMPILER mipsel-ps2-irx-g++)
SET(CMAKE_C_COMPILER_WORKS 1) #Hack by f0bes
SET(CMAKE_CXX_COMPILER_WORKS 1) #Hack by f0bes


#
# set arch flags depending on gcc version
#
EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE IOP_CC_VERSION)

SET(IOP_ASFLAGS_TARGET "-mcpu=r3000")
INCLUDE_DIRECTORIES($ENV{PS2SDK}/iop/include $ENV{PS2SDK}/common/include $ENV{PS2SDK}/ports_iop/include)
ADD_DEFINITIONS(-D_IOP -DPS2 -D__PS2__)

SET(IOP_CFLAGS "${IOP_CFLAGS_TARGET} -O2 -G0 -fno-builtin" CACHE STRING "IOP C compiler flags" FORCE)
SET(IOP_LDFLAGS "${IOP_LDFLAGS_TARGET} -T$ENV{PS2SDK}/iop/startup/linkfile -L$ENV{PS2SDK}/iop/lib -L$ENV{PS2SDK}/ports_iop/lib -Wl,-zmax-page-size=128 -nostdlib -Os -Wall" CACHE STRING "IOP linker flags" FORCE)
SET(IOP_ASFLAGS "${IOP_ASFLAGS_TARGET} -EL -G0" CACHE STRING "IOP assembler flags" FORCE)

SET(CMAKE_C_FLAGS_INIT ${IOP_CFLAGS})
SET(CMAKE_CXX_FLAGS_INIT ${IOP_CFLAGS})
SET(CMAKE_EXE_LINKER_FLAGS_INIT ${IOP_LDFLAGS})

SET(CMAKE_TARGET_INSTALL_PREFIX $ENV{PS2DEV}/ports_iop)

SET(CMAKE_FIND_ROOT_PATH $ENV{PS2DEV} $ENV{PS2DEV}/iop $ENV{PS2DEV}/iop/iop $ENV{PS2SDK} $ENV{PS2SDK}/ports_iop)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

FUNCTION(BUILD_IOP_IMPORTS out_f in_f)
    ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
        COMMAND ${CMAKE_COMMAND} -E echo \#include \"irx_imports.h\" > ${out_f}
        COMMAND ${CMAKE_COMMAND} -E cat ${in_f} >> ${out_f}
        DEPENDS ${in_f}
        COMMENT "Creating ${out_f}" VERBATIM
    )
ENDFUNCTION()

FUNCTION(BUILD_IOP_EXPORTS out_f in_f)
    ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
        COMMAND ${CMAKE_COMMAND} -E echo \#include \"irx.h\" > ${out_f}
        COMMAND ${CMAKE_COMMAND} -E cat ${in_f} >> ${out_f}
        DEPENDS ${in_f}
        COMMENT "Creating ${out_f}" VERBATIM
    )
ENDFUNCTION()

SET(PLATFORM_PS2 TRUE)
SET(IOP TRUE)
SET(PS2 TRUE)
