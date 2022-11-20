cmake_minimum_required(VERSION 3.10)

project(py_humanoid_sdk)

set(DEPENDENCIES humanoid_sdk)

if (pybind11_FOUND)
    pybind11_add_module(py_humanoid_sdk py_humanoid_sdk.cpp)
    target_link_libraries(py_humanoid_sdk PRIVATE humanoid_sdk)
endif ()
