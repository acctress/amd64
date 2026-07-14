add_executable(amd64_examples textual_ir.cpp)

target_include_directories(amd64_examples PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(amd64_examples PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/exe"
)