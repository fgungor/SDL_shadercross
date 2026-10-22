# DXC's legalization pipeline must preserve user-defined stage interfaces.
# https://github.com/libsdl-org/SDL_shadercross/issues/169
find_package(Git REQUIRED)
set(shadercross_spirv_patch "${CMAKE_CURRENT_LIST_DIR}/../external/patches/spirv-tools-preserve-interface.patch")
set(shadercross_spirv_source "${CMAKE_CURRENT_SOURCE_DIR}/external/SPIRV-Tools")
execute_process(
    COMMAND "${GIT_EXECUTABLE}" apply --check "${shadercross_spirv_patch}"
    WORKING_DIRECTORY "${shadercross_spirv_source}"
    RESULT_VARIABLE shadercross_patch_check
    OUTPUT_QUIET ERROR_QUIET
)
if(shadercross_patch_check EQUAL 0)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply "${shadercross_spirv_patch}"
        WORKING_DIRECTORY "${shadercross_spirv_source}"
        COMMAND_ERROR_IS_FATAL ANY
    )
else()
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${shadercross_spirv_patch}"
        WORKING_DIRECTORY "${shadercross_spirv_source}"
        RESULT_VARIABLE shadercross_patch_reverse_check
        OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT shadercross_patch_reverse_check EQUAL 0)
        message(FATAL_ERROR "Cannot apply the SPIRV-Tools interface-preservation patch. Check external/SPIRV-Tools for conflicting changes.")
    endif()
endif()
