enable_testing()

set(shader_sources
    "shaders/simple.vert.hlsl"
)

find_package(Python3 COMPONENTS Interpreter)

set(includeable_shader_sources)
foreach(src IN LISTS shader_sources)
    get_filename_component(src_name "${src}" NAME)
    set(src_path "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
    set(hdr_path "${CMAKE_CURRENT_SOURCE_DIR}/shaders/hinc/${src_name}.h")
    list(APPEND includeable_shader_sources "${hdr_path}")
    if(Python3_VERSION VERSION_GREATER_EQUAL "3.11")
        add_custom_command(OUTPUT "${hdr_path}"
            COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/embed.py"
                -i "${src_path}" -o "${hdr_path}" -z
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embed.py" "${src_path}"
        )
    endif()
endforeach()

find_package(SDL3 REQUIRED COMPONENTS SDL3_test)

add_executable(testshadercross
    main.c
    ${includeable_shader_sources}
)
sdl_add_warning_options(testshadercross)
target_compile_definitions(testshadercross PRIVATE SDL_SHADERCROSS_TEST_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
target_link_libraries(testshadercross PRIVATE SDL3_shadercross::SDL3_shadercross)
target_link_libraries(testshadercross PRIVATE SDL3::SDL3_test)
target_link_libraries(testshadercross PRIVATE SDL3::SDL3)

if(TARGET dxc)
    set(raw_array_spirv "${CMAKE_CURRENT_BINARY_DIR}/sparse-raw-arrays.frag.spv")
    add_custom_command(OUTPUT "${raw_array_spirv}"
        COMMAND "$<TARGET_FILE:dxc>" -spirv -fspv-preserve-interface -E main -T ps_6_0
            -DSTORAGE_KIND=1 -DSAMPLE_ARRAY=1 -DLAST_ARRAY_UNUSED=1
            "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sparse-samplers.frag.hlsl"
            -Fo "${raw_array_spirv}"
        DEPENDS dxc "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sparse-samplers.frag.hlsl"
        VERBATIM
    )
    add_custom_target(shadercross_test_raw_arrays DEPENDS "${raw_array_spirv}")
    add_dependencies(testshadercross shadercross_test_raw_arrays)
    target_compile_definitions(testshadercross PRIVATE SDL_SHADERCROSS_TEST_RAW_ARRAY_SPIRV="${raw_array_spirv}")
endif()

set(test_args)
if(SDLSHADERCROSS_TESTS_TRACKMEM)
    list(APPEND test_args "--trackmem")
endif()
add_test(NAME testshadercross COMMAND testshadercross ${test_args})
set_property(TEST testshadercross PROPERTY TIMEOUT 30)

if(UNIX AND NOT APPLE)
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "LD_LIBRARY_PATH=path_list_prepend:${SDL3_shadercross_BINARY_DIR}/vkd3d-prefix/lib")
    if(SDLSHADERCROSS_INSTALL_RUNTIME)
        set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "LD_LIBRARY_PATH=path_list_prepend:$<TARGET_FILE_DIR:DirectXShaderCompiler::dxcompiler>")
    endif()
elseif(WIN32)
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:DirectXShaderCompiler::dxcompiler>")
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SDL3::SDL3>")
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SDL3_shadercross::SDL3_shadercross>")
    if(TARGET spirv-cross-c-shared)
        set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:spirv-cross-c-shared>")
    endif()
endif()
if(SDLSHADERCROSS_TESTS_TRACKMEM)
    set_property(TEST testshadercross PROPERTY FAIL_REGULAR_EXPRESSION "Total: [0-9]+\\.[0-9]+ Kb in [1-9][0-9]* allocations")
endif()

if(TARGET shadercross AND SDLSHADERCROSS_DXC)
    get_property(shadercross_test_environment TEST testshadercross PROPERTY ENVIRONMENT_MODIFICATION)
    function(add_sparse_sampler_json_test name storage_kind sample_array samplers textures buffers)
        set(shader "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sparse-samplers.frag.hlsl")
        if(ARGC GREATER 6)
            set(shader "${ARGV6}")
        endif()
        add_test(NAME "shadercross_sparse_json_${name}" COMMAND "${CMAKE_COMMAND}"
            "-DSHADERCROSS=$<TARGET_FILE:shadercross>"
            "-DSHADER=${shader}"
            "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/sparse-${name}.json"
            "-DSTORAGE_KIND=${storage_kind}"
            "-DSAMPLE_ARRAY=${sample_array}"
            "-Dexpected_samplers=${samplers}"
            "-Dexpected_storage_textures=${textures}"
            "-Dexpected_storage_buffers=${buffers}"
            "-Dexpected_uniform_buffers=4"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check-sparse-json.cmake"
        )
        set_tests_properties("shadercross_sparse_json_${name}" PROPERTIES
            ENVIRONMENT_MODIFICATION "${shadercross_test_environment}"
            TIMEOUT 10
        )
    endfunction()
    add_sparse_sampler_json_test(plain 0 0 7 0 0)
    add_sparse_sampler_json_test(storage 1 0 7 1 1)
    add_sparse_sampler_json_test(storage_gaps 2 0 7 3 3)
    add_sparse_sampler_json_test(arrays 0 1 8 0 0)
    add_sparse_sampler_json_test(arrays_storage 1 1 8 1 1)
    if(TARGET dxc)
        add_sparse_sampler_json_test(raw_arrays 1 1 8 1 1 "${raw_array_spirv}")
    endif()
endif()
