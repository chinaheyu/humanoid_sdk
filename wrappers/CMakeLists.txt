cmake_minimum_required(VERSION 3.10)

project(humanoid_sdk_wrappers)

set(DEPENDENCIES humanoid_sdk)

if (BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
