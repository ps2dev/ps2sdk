add_library(draw)
target_include_directories(draw PUBLIC include)
target_include_directories(draw PRIVATE 
    ${EE_INC} 
    ${PROJECT_SOURCE_DIR}/math3d/include
    ${PROJECT_SOURCE_DIR}/dma/include
)

target_sources(draw PRIVATE 
    src/draw_environment.c
    src/draw.c
    src/draw2d.c
    src/draw3d.c
    src/erl-support.c
)

file(GLOB DRAW_INCLUDE_FILES "include/*.h*")
set_target_properties(draw PROPERTIES PUBLIC_HEADER "${DRAW_INCLUDE_FILES}")
install(TARGETS draw)

target_add_erl(draw)