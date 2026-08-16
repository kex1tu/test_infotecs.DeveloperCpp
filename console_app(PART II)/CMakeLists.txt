cmake_minimum_required(VERSION 3.14)


project(console_app VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(console_app
    src/main.cpp
    src/utilities.cpp
)

target_include_directories(console_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(console_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(console_app PRIVATE logger_static)
target_link_libraries(console_app PRIVATE -pthread)
target_compile_options(console_app PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
