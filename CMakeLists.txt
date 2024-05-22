cmake_minimum_required(VERSION 3.5)
project(chess-engine)

# Compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")
# set(CMAKE_BUILD_TYPE Release)

# Define sources
set(SOURCES
    main.cpp
    board.cpp
    engine.cpp
    util.cpp
)

# Create executable
add_executable(CE ${SOURCES})
