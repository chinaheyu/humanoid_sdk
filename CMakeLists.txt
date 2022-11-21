cmake_minimum_required(VERSION 3.10)
project(humanoid_sdk C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(NOT BUILD_EXAMPLES)
    set(BUILD_EXAMPLES ON)
endif()

if(NOT BUILD_PYTHON_BINDINGS)
    set(BUILD_PYTHON_BINDINGS ON)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if (WIN32)
    add_link_options(-static)
endif ()

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/serial_lite)
add_subdirectory(third_party/pybind11)

aux_source_directory(src SDK_SRCS)
add_library(humanoid_sdk ${SDK_SRCS})
target_include_directories(humanoid_sdk PUBLIC include)
target_link_libraries(humanoid_sdk PUBLIC fmt::fmt serial)

add_subdirectory(wrappers)
add_subdirectory(examples)

unset(CMAKE_BUILD_TYPE CACHE)
unset(BUILD_EXAMPLES CACHE)
unset(BUILD_PYTHON_BINDINGS CACHE)
