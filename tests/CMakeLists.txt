add_executable(amd64_tests core.cpp assembler.cpp allocator.cpp register.cpp encode.cpp ir.cpp)

target_include_directories(amd64_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(amd64_tests PRIVATE Catch2::Catch2WithMain)

set_target_properties(amd64_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/exe"
)

include(CTest)
include(Catch)
catch_discover_tests(amd64_tests)