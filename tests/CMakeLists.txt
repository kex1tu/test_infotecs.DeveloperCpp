enable_testing()

add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger PRIVATE logger_static)

target_compile_options(test_logger PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
    -fno-exceptions
)

add_test(NAME LoggerTests COMMAND test_logger)