add_library(erl)
target_include_directories(erl PUBLIC include)
target_include_directories(erl PRIVATE ${EE_INC})
target_sources(erl PRIVATE src/erl.c src/hashtab.c src/lookupa.c src/recycle.c)