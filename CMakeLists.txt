cmake_minimum_required(VERSION 3.10)

project(baton)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

file(GLOB_RECURSE HEADERS "include/*.h" "third_party/*.hpp")
file(GLOB_RECURSE SOURCES "src/*.cc"  "src/autocomplete/*.cc")

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(baton ${HEADERS} ${SOURCES})

target_link_libraries(baton
        Qt5::Core
        Qt5::Widgets)
target_include_directories(baton PUBLIC ./include/autocomplete)
target_include_directories(baton PUBLIC ./third_party)
