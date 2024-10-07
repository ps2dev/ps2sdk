# Compiles the same source file multiple times with different defines
function(compile_multiple target srcfile)
    cmake_parse_arguments(PARSE_ARGV 2 "arg" "" "" "OBJECTS")

    foreach(obj ${arg_OBJECTS})
        add_library(${obj} OBJECT ${srcfile})
        get_filename_component(def ${obj} NAME_WLE)
        target_compile_definitions(${obj} PRIVATE "F_${def}")

        get_target_property(target_id ${target} INCLUDE_DIRECTORIES)
        target_include_directories(${obj} PRIVATE ${target_id})

        target_link_libraries(${target} PRIVATE ${obj})
    endforeach()
endfunction()

# Add an erl output for a given target
function(target_add_erl target)
    add_custom_command(OUTPUT "lib${target}.erl"
        COMMAND ${CMAKE_C_COMPILER} -nostdlib -Wl,-r -Wl,-d -o "lib${target}.erl" $<TARGET_OBJECTS:${target}>
        COMMAND ${CMAKE_STRIP} --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment lib${target}.erl
        DEPENDS ${target}
        COMMAND_EXPAND_LISTS
    )
    add_custom_target(${target}_erl ALL
        DEPENDS lib${target}.erl
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${target}.erl
        DESTINATION lib
    )
endfunction()

# Generates a C array of the binary output of a target
# objcopy -Obinary <elf> <bin> && bin2c <bin> <output_name.c> <target_name>
macro(bin_include from_target output_name)
add_custom_command(OUTPUT "${output_name}.c"
  COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${from_target}> "${output_name}.bin"
  COMMAND bin2c "${output_name}.bin" "${output_name}.c" "${from_target}"
  BYPRODUCTS "${output_name}.bin"
  DEPENDS ${from_target}
)
endmacro()
