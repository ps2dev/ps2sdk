add_executable(eenull)
target_sources(eenull PRIVATE src/eenull.s)
target_compile_options(eenull PRIVATE -mno-gpopt)
target_link_options(eenull PRIVATE -nostdlib -nostartfiles -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile -s)