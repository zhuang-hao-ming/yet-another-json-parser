cmake_minimum_required(VERSION 3.5.1)
project(json)

set(CMAKE_BUILD_TYPE Debug)

add_executable(json_test test.c json.c)