add_library(eedebug)
target_include_directories(eedebug PUBLIC include)
target_include_directories(eedebug PRIVATE 
    ${EE_INC} 
)

target_sources(eedebug PRIVATE 
    src/ee_dbg_low.S
    src/ee_debug.c
    src/ee_exceptions.S
    src/erl-support.c
)

file(GLOB eedebug_INCLUDE_FILES "include/*.h*")
set_target_properties(eedebug PROPERTIES PUBLIC_HEADER "${eedebug_INCLUDE_FILES}")
install(TARGETS eedebug)

target_add_erl(eedebug)