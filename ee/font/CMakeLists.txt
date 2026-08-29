add_library(font)
target_include_directories(font PUBLIC include)
target_include_directories(font PRIVATE 
    ${EE_INC} 
)
target_link_libraries(font PRIVATE draw math3d)
target_sources(font PRIVATE 
    src/fontx.c
    src/fsfont.c
    src/erl-support.c
)

file(GLOB FONT_INCLUDE_FILES "include/*.h*")
set_target_properties(font PROPERTIES PUBLIC_HEADER "${FONT_INCLUDE_FILES}")
install(TARGETS font)

target_add_erl(font)