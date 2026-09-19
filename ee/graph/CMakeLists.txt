add_library(graph)
target_include_directories(graph PUBLIC include)
target_include_directories(graph PRIVATE 
    ${EE_INC} 
)
target_link_libraries(graph PRIVATE cdvd)
target_sources(graph PRIVATE 
    src/graph_config.c
    src/graph_mode.c
    src/graph_vram.c
    src/graph.c
    src/erl-support.c
)

file(GLOB GRAPH_INCLUDE_FILES "include/*.h*")
set_target_properties(graph PROPERTIES PUBLIC_HEADER "${GRAPH_INCLUDE_FILES}")
install(TARGETS graph)

target_add_erl(graph)