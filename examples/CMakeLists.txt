cmake_minimum_required(VERSION 3.10)
project(humanoid_sdk_examples C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

if (BUILD_EXAMPLES)
    add_executable(cpp_demo cpp_demo.cpp)
    add_dependencies(cpp_demo humanoid_sdk)
    target_link_libraries(cpp_demo humanoid_sdk)

    install(TARGETS cpp_demo
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib/shared
            ARCHIVE DESTINATION lib/static
            PUBLIC_HEADER DESTINATION include
            )
endif()
