cmake_minimum_required(VERSION 3.5)

project(udes-replay
        VERSION 1.0
        LANGUAGES C CXX
        DESCRIPTION "système d'écoute et de rediffusion")

set(CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

find_library(TERMKEY_LIB termkey)

add_executable(udes-replay main.cpp ${SOURCES})

target_link_libraries(udes-replay ${TERMKEY_LIB})

target_include_directories(udes-replay PUBLIC ${PROJECT_SOURCE_DIR}/include)