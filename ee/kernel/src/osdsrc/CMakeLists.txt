add_executable(osdsrc)

target_sources(osdsrc PRIVATE src/ExecPS2.c src/osd.c src/osdsrc.c)
target_compile_definitions(osdsrc PRIVATE -DREUSE_EXECPS2)
target_compile_options(osdsrc PRIVATE -fno-tree-loop-distribute-patterns -mno-gpopt)
target_include_directories(osdsrc PRIVATE ${EE_INC})
target_link_options(osdsrc PRIVATE -nostdlib -nostartfiles -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile -s)