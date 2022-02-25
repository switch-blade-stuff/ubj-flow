cmake_minimum_required(VERSION 3.22)
project(ubjf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

include(GenerateExportHeader)

option(UBJF_BUILD_SHARED "Enables shared library build" ON)
if (UBJF_BUILD_SHARED)
    set(BUILD_SHARED_LIBS ON)
else ()
    set(BUILD_SHARED_LIBS OFF)
endif ()

add_library(${PROJECT_NAME})

option(UBJF_BIG_ENDIAN "Tells the library that the architecture is big-endian. By default, little-endian architectures are assumed." OFF)
if (UBJF_BIG_ENDIAN)
    add_compile_definitions(UBJF_BIG_ENDIAN)
endif ()

option(UBJF_NO_CHECKS "Disables certain internal checks that may affect performance" OFF)
if (UBJF_NO_CHECKS)
    add_compile_definitions(UBJF_DISABLE_CHECKS)
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/ubjf/CMakeLists.txt)

add_subdirectory(test)

# TODO: Add install target

