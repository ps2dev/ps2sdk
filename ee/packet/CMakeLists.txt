add_library(packet)
target_include_directories(packet PUBLIC include)
target_include_directories(packet PRIVATE ${EE_INC})
target_sources(packet PRIVATE src/packet.c src/erl-support.c)

file(GLOB PACKET_INCLUDE_FILES "include/*.h*")
set_target_properties(packet PROPERTIES PUBLIC_HEADER "${PACKET_INCLUDE_FILES}")
install(TARGETS packet)
target_add_erl(packet)