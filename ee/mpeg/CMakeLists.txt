add_library(mpeg)
target_include_directories(mpeg PUBLIC include)
target_include_directories(mpeg PRIVATE ${EE_INC})
target_sources(mpeg PRIVATE src/libmpeg.c src/libmpeg_core_c.c src/erl-support.c)

file(GLOB MPEG_INCLUDE_FILES "include/*.h*")
set_target_properties(mpeg PROPERTIES PUBLIC_HEADER "${MPEG_INCLUDE_FILES}")
install(TARGETS mpeg)
target_add_erl(mpeg)