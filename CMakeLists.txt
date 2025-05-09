cmake_minimum_required(VERSION 3.27.0)

project(
    Enda
    VERSION 0.1
    DESCRIPTION "Efficient NdArray"
    HOMEPAGE_URL "%%github.com/isqiwen/enda%%"
    LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use a separate build directory.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(DevTools/CMake/Tools.cmake)
include(DevTools/CMake/StandardSettings.cmake)
include(DevTools/CMake/StaticAnalyzers.cmake)
include(DevTools/CMake/ProjectOptions.cmake)

print_project_configuration()

add_subdirectory(Source)

if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
    enable_testing()
    add_subdirectory(Test)
endif()

if(${PROJECT_NAME}_ENABLE_BENCHMARKS)
    add_subdirectory(Benchmarks)
endif()
