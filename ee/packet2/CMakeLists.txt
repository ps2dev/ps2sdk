add_library(packet2)
target_include_directories(packet2 PUBLIC include)
target_include_directories(packet2 PRIVATE ${EE_INC})
target_sources(packet2 PRIVATE src/packet2.c src/packet2_vif.c src/erl-support.c)

file(GLOB PACKET2_INCLUDE_FILES "include/*.h*")
set_target_properties(packet2 PROPERTIES PUBLIC_HEADER "${PACKET2_INCLUDE_FILES}")
install(TARGETS packet2)
target_add_erl(packet2)