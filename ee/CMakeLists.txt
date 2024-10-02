cmake_minimum_required(VERSION 3.10)
project(ps2sdk-ee C ASM)
file(REAL_PATH ../ PS2SDKSRC_ROOT)

set(EE_INC ${PS2SDKSRC_ROOT}/common/include ${PROJECT_SOURCE_DIR}/kernel/include ${PROJECT_SOURCE_DIR}/rpc/cdvd/include)

add_subdirectory(kernel)
add_subdirectory(debug)