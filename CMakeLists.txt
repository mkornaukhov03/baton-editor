cmake_minimum_required(VERSION 3.10)

project(baton)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2")

# Add your included directories here
include_directories("include/" 
                    "include/autocomplete/"
                    "third_party/")

# Add your header files here
set(HEADERS "include/autocomplete/enums.h"
            "include/autocomplete/interface.h"
            "include/autocomplete/lsp_basic.h"
            "include/autocomplete/client.h"
            "include/autocomplete/json_serializers.h")

# Add your source files here
set(SOURCES "src/main.cc"
            "src/autocomplete/json_serializers.cc"
            "src/autocomplete/lsp_basic.cc"
            "src/autocomplete/client.cc"
            "src/autocomplete/interface.cc")

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(baton ${HEADERS} ${SOURCES})

target_link_libraries(baton
        Qt5::Core
        Qt5::Widgets)
