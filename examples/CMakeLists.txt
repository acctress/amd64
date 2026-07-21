set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
        Zydis
        GIT_REPOSITORY https://github.com/zyantific/zydis.git
        GIT_TAG        v4.0.0
)
FetchContent_MakeAvailable(Zydis)

add_executable(amd64_examples textual_ir.cpp)
add_executable(tinyc tinyc.cpp)

target_include_directories(amd64_examples PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(amd64_examples PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/exe"
)

target_include_directories(tinyc PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tinyc PRIVATE Zydis)

set_target_properties(tinyc PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/exe"
)