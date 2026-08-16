enable_testing()

add_executable(test_logger test_logger.cpp
    tests.hpp)
target_link_libraries(test_logger PRIVATE logger_static)

target_compile_options(test_logger PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)

add_test(NAME LoggerTests COMMAND test_logger)



add_executable(test_thread_safe_queue test_thread_safe_queue.cpp
    tests.hpp)
target_link_libraries(test_thread_safe_queue PRIVATE logger_static)

target_compile_options(test_thread_safe_queue PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
add_test(NAME ThreadSafeQueueTests COMMAND test_thread_safe_queue)