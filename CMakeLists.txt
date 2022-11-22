cmake_minimum_required(VERSION 3.10)
project(humanoid_sdk C CXX)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED BUILD_EXAMPLES)
    set(BUILD_EXAMPLES ON)
endif()

if(NOT DEFINED BUILD_PYTHON_BINDINGS)
    set(BUILD_PYTHON_BINDINGS ON)
endif()

if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif ()

if (UNIX AND NOT APPLE)
    add_compile_options(-fPIC)
endif ()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/shared)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/static)
set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_module")

include(PrintCmakeVariables)

print_cmake_variables()

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/serial_lite)
add_subdirectory(third_party/pybind11)

file(GLOB SDK_HDRS include/*.*)
aux_source_directory(src SDK_SRCS)

add_library(humanoid_sdk_static STATIC ${SDK_SRCS})
add_dependencies(humanoid_sdk_static fmt::fmt-header-only serial)
set_target_properties(humanoid_sdk_static PROPERTIES OUTPUT_NAME humanoid_sdk)
target_include_directories(humanoid_sdk_static PUBLIC include)
target_link_libraries(humanoid_sdk_static PUBLIC fmt::fmt-header-only serial)

add_library(humanoid_sdk_shared SHARED ${SDK_SRCS})
add_dependencies(humanoid_sdk_shared fmt::fmt-header-only serial)
set_target_properties(humanoid_sdk_shared PROPERTIES OUTPUT_NAME humanoid_sdk)
target_include_directories(humanoid_sdk_shared PUBLIC include)
target_link_libraries(humanoid_sdk_shared PUBLIC fmt::fmt-header-only serial)

add_library(humanoid_sdk_interface INTERFACE)
set_target_properties(humanoid_sdk_interface PROPERTIES PUBLIC_HEADER "${SDK_HDRS}")

if (BUILD_SHARED_LIBS)
    add_library(humanoid_sdk ALIAS humanoid_sdk_shared)
else()
    add_library(humanoid_sdk ALIAS humanoid_sdk_static)
endif ()

add_subdirectory(wrappers)
add_subdirectory(examples)

install(TARGETS humanoid_sdk_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/shared
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include)

install(TARGETS humanoid_sdk_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/shared
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include)

install(TARGETS humanoid_sdk_interface
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/shared
        ARCHIVE DESTINATION lib/static
        PUBLIC_HEADER DESTINATION include)

if(MINGW)
    message(STATUS "Installing system-libraries: MinGW DLLs.")
    get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
    file(GLOB MINGW_DLLS ${MINGW_PATH}/*.dll)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${MINGW_DLLS})
    include(InstallRequiredSystemLibraries)
endif()

