cmake_minimum_required(VERSION 3.10)

project(baton)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -fsanitize=address -fsanitize=undefined")
set(CMAKE_BUILD_RPATH "build/")

file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(baton ${HEADERS} ${SOURCES})

