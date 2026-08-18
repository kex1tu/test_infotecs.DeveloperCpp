cmake_minimum_required(VERSION 3.14)

project(infotecs_test_task VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory("logger")
add_subdirectory("console_app")
add_subdirectory("stat_app")
add_subdirectory("tests")