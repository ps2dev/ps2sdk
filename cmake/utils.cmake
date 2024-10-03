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

