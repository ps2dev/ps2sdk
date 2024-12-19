add_library(profglue)
target_include_directories(profglue PUBLIC include)
target_include_directories(profglue PRIVATE 
    ${EE_INC} 
)
target_sources(profglue PRIVATE 
    src/mcount.S
    src/prof.c
)

file(GLOB PROFGLUE_INCLUDE_FILES "include/*.h*")
set_target_properties(profglue PROPERTIES PUBLIC_HEADER "${PROFGLUE_INCLUDE_FILES}")
install(TARGETS profglue)