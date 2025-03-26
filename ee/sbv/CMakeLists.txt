add_library(patches)
target_include_directories(patches PUBLIC include)
target_include_directories(patches PRIVATE ${EE_INC})
target_sources(patches PRIVATE src/smem.c src/smod.c src/slib.c 
    src/patch_enable_lmb.c src/patch_disable_prefix_check.c
	src/patch_user_mem_clear.c src/patch_fileio.c src/common.c 
    src/erl-support.c
)

file(GLOB PATCHES_INCLUDE_FILES "include/*.h*")
set_target_properties(patches PROPERTIES PUBLIC_HEADER "${PATCHES_INCLUDE_FILES}")
install(TARGETS patches)

target_add_erl(patches)