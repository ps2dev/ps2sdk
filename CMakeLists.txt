cmake_minimum_required(VERSION 3.10)
project(ps2sdk)
include(ExternalProject)

ExternalProject_Add(tools
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/tools
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/tools
    BUILD_ALWAYS true)

ExternalProject_Add(ee
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/ee
    CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/ee_toolchain.cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/ee
    BUILD_ALWAYS true)

#ExternalProject_Add(iop
#    SOURCE_DIR ${PROJECT_SOURCE_DIR}/iop
#    CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/cmake/iop_toolchain.cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/iop
#    BUILD_ALWAYS true)