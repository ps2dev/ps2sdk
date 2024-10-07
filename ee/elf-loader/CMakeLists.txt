add_subdirectory(src/loader)

add_library(elf-loader)
target_include_directories(elf-loader PUBLIC include)
target_include_directories(elf-loader PRIVATE 
    ${EE_INC} 
)

bin_include(loader loader_bin)
target_sources(elf-loader PRIVATE 
    src/elf.c
    loader_bin.c
)

file(GLOB elf-loader_INCLUDE_FILES "include/*.h*")
set_target_properties(elf-loader PROPERTIES PUBLIC_HEADER "${elf-loader_INCLUDE_FILES}")
install(TARGETS elf-loader)

target_add_erl(elf-loader)