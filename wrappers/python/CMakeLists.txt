cmake_minimum_required(VERSION 3.10)

project(py_humanoid_sdk)

set(DEPENDENCIES humanoid_sdk)

if (pybind11_FOUND)
    pybind11_add_module(py_humanoid_sdk py_humanoid_sdk.cpp)
    add_dependencies(py_humanoid_sdk humanoid_sdk_static)
    target_link_libraries(py_humanoid_sdk PRIVATE humanoid_sdk_static)

    install(TARGETS py_humanoid_sdk
            RUNTIME DESTINATION python_wrappers
            LIBRARY DESTINATION python_wrappers
            ARCHIVE DESTINATION python_wrappers
            PUBLIC_HEADER DESTINATION python_wrappers
            )

    if(MINGW)
        message(STATUS "Installing system-libraries: MinGW DLLs.")
        get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
        file(GLOB MINGW_DLLS ${MINGW_PATH}/*.dll)
        install(FILES ${MINGW_DLLS} DESTINATION python_wrappers)
    endif()
endif ()
