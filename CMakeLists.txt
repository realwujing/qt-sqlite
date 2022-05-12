cmake_minimum_required(VERSION 3.0.0)
project(connection VERSION 0.1.0)

# set(CMAKE_VERBOSE_MAKEFILE ON)

message(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_AUTOMOC ON)

set(CMAKE_BUILD_TYPE "Debug")

find_package(Qt5 REQUIRED COMPONENTS Core Sql)

add_executable(connection main.cpp connection.cpp)

target_link_libraries(connection Qt5::Core Qt5::Sql) 
