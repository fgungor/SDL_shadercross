execute_process(
    COMMAND "${SHADERCROSS}" "${SHADER}" "-DSTORAGE_KIND=${STORAGE_KIND}" "-DSAMPLE_ARRAY=${SAMPLE_ARRAY}" -o "${OUTPUT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Sparse sampler JSON compilation failed: ${stdout}${stderr}")
endif()
file(READ "${OUTPUT}" metadata)
foreach(field IN ITEMS samplers storage_textures storage_buffers uniform_buffers)
    string(JSON actual GET "${metadata}" "${field}")
    if(NOT actual EQUAL expected_${field})
        message(FATAL_ERROR "${field}: expected ${expected_${field}}, got ${actual}: ${metadata}")
    endif()
endforeach()
