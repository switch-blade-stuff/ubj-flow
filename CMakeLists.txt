cmake_minimum_required(VERSION 3.21)
project(ubjf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GenerateExportHeader)

option(UBJF_BUILD_SHARED "Enables shared library build" ON)
if (UBJF_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
else ()
    set(BUILD_SHARED_LIBS OFF)
endif ()

add_library(${PROJECT_NAME})
if (NOT MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif ()

option(UBJF_BIG_ENDIAN "Tells the library that the architecture is big-endian. By default, little-endian architectures are assumed." OFF)
if (UBJF_BIG_ENDIAN)
    add_compile_definitions(UBJF_BIG_ENDIAN)
endif ()

option(UBJF_NO_CHECKS "Disables certain internal checks that may affect performance" OFF)
if (UBJF_NO_CHECKS)
    add_compile_definitions(UBJF_DISABLE_CHECKS)
endif ()

option(UBJF_NO_SPEC_12 "Disables build of the spec. 12 UBJson syntax version" OFF)
if (UBJF_NO_SPEC_12)
    add_compile_definitions(UBJF_NO_SPEC_12)
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/ubjf/CMakeLists.txt)

# TODO: Add install target

option(UBJF_NO_TESTS "Disables unit test target for Debug build" OFF)
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND NOT UBJF_NO_TESTS)
    add_subdirectory(test)
endif ()
