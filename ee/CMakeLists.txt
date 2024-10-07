cmake_minimum_required(VERSION 3.10)
project(ps2sdk-ee C ASM)
file(REAL_PATH ../ PS2SDKSRC_ROOT)

set(EE_INC 
    ${PS2SDKSRC_ROOT}/common/include 
    ${PROJECT_SOURCE_DIR}/kernel/include
    ${PROJECT_SOURCE_DIR}/erl/include
)

include(${PS2SDKSRC_ROOT}/cmake/utils.cmake)

add_subdirectory(debug)
add_subdirectory(dma)
add_subdirectory(draw)
add_subdirectory(eedebug)
add_subdirectory(elf-loader)
add_subdirectory(kernel)
add_subdirectory(erl)
add_subdirectory(libcglue)
add_subdirectory(rpc)
add_subdirectory(sbv)
add_subdirectory(startup)
