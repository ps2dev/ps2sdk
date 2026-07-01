add_library(math3d)
target_include_directories(math3d PUBLIC include)
target_include_directories(math3d PRIVATE 
    ${EE_INC} 
)
target_link_libraries(math3d PRIVATE graph)
target_sources(math3d PRIVATE 
    src/math3d.c
    src/erl-support.c
)

file(GLOB MATH3D_INCLUDE_FILES "include/*.h*")
set_target_properties(math3d PROPERTIES PUBLIC_HEADER "${MATH3D_INCLUDE_FILES}")
install(TARGETS math3d)

target_add_erl(math3d)