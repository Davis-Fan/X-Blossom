cmake_minimum_required(VERSION 3.19)
project(Blossom)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(Blossom main.cpp blossom.h graph.h functions.cpp
        functions_for_par_0.cpp
        stopwatch.h
        functions_for_par_1.cpp
        functions_for_par_2.cpp
        par_test.cpp
        par_200.cpp
)
