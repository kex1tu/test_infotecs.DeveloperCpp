# базовая настройка
cmake_minimum_required(VERSION 3.14)

project(logger_lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# либа стат
add_library(logger_static STATIC
    src/logger.cpp
    src/file_sink.cpp
    src/socket_sink.cpp
)
# либа дин
add_library(logger_shared SHARED
    src/logger.cpp
    src/file_sink.cpp
    src/socket_sink.cpp
)

# включение папок
target_include_directories(logger_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(logger_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(logger_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(logger_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)



target_compile_options(logger_static PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
target_compile_options(logger_shared PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
