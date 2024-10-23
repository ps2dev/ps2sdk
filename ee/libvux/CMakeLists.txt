add_library(vux)
target_include_directories(vux PUBLIC include)
target_include_directories(vux PRIVATE ${EE_INC})
target_sources(vux PRIVATE src/vuhw.c src/vusw.c src/vux.c src/erl-support.c)

file(GLOB VUX_INCLUDE_FILES "include/*.h*")
set_target_properties(vux PROPERTIES PUBLIC_HEADER "${VUX_INCLUDE_FILES}")
install(TARGETS vux)
target_add_erl(vux)