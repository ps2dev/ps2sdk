add_library(input)
target_include_directories(input PUBLIC include)
target_include_directories(input PRIVATE 
    ${EE_INC} 
)

target_link_libraries(input PRIVATE pad)

target_sources(input PRIVATE 
    src/input.c
    src/erl-support.c
)

file(GLOB INPUT_INCLUDE_FILES "include/*.h*")
set_target_properties(input PROPERTIES PUBLIC_HEADER "${INPUT_INCLUDE_FILES}")
install(TARGETS input)

target_add_erl(input)

# inputx
add_library(inputx)
target_include_directories(inputx PUBLIC include)
target_include_directories(inputx PRIVATE 
    ${EE_INC} 
)
target_compile_definitions(inputx PRIVATE _XINPUT)

target_link_libraries(inputx PRIVATE pad multitap)

target_sources(inputx PRIVATE 
    src/input.c
    src/erl-support.c
)

install(TARGETS inputx)
target_add_erl(inputx)