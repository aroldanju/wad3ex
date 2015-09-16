cmake_minimum_required(VERSION 3.3)
project(wad3ex)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(include)
include_directories(src)

set(SOURCE_FILES src/main.cpp
        src/Image.cpp
        include/Image.h)

find_library(SDL_LIBRARY SDL2)

add_executable(wad3ex ${SOURCE_FILES})
target_link_libraries(wad3ex ${SDL_LIBRARY})