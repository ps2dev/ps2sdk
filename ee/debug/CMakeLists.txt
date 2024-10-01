add_library(debug)
target_include_directories(debug PRIVATE ${PS2SDKSRC_ROOT}/common/include)
target_sources(debug PRIVATE 
    src/callstack.c
    src/callstackget.S
    src/erl-support.c
    src/font.c
    src/hwbp.S
    src/scr_printf.c
    src/screenshot.c
)

file(GLOB DEBUG_INCLUDE_FILES "include/*.h*")
set_target_properties(debug PROPERTIES PUBLIC_HEADER "${DEBUG_INCLUDE_FILES}")
target_link_libraries(kernel)

install(TARGETS debug)
