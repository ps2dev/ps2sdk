add_library(netman)
target_include_directories(netman PRIVATE ${EE_INC} src/include)
target_sources(netman PRIVATE src/netman.c src/rpc_server.c src/rpc_client.c
                              src/erl-support.c)

install(TARGETS netman)
target_add_erl(netman)
