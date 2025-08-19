add_library(gs)
target_include_directories(gs PUBLIC include)
target_include_directories(gs PRIVATE 
    ${EE_INC} 
)
target_sources(gs PRIVATE 
    src/dma.c
    src/DoubleBuff.c
    src/draw.c
    src/libgs.c
    src/lowlevel.c
    src/packets.c
    src/primitives.c
    src/ResetPath.s
    src/sync.c
    src/texture.c
    src/Zbuffer.c
    src/erl-support.c
)

file(GLOB GS_INCLUDE_FILES "include/*.h*")
set_target_properties(gs PROPERTIES PUBLIC_HEADER "${GS_INCLUDE_FILES}")
install(TARGETS gs)

target_add_erl(gs)