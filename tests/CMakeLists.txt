
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



add_executable(test_utilities test_utilities.cpp
    "${CMAKE_SOURCE_DIR}/console_app/src/utilities.cpp"
    tests.hpp)
target_include_directories(test_utilities PRIVATE
    "${CMAKE_SOURCE_DIR}/console_app/src"
)
target_link_libraries(test_utilities PRIVATE logger_static)

target_compile_options(test_utilities PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)

add_test(NAME UtilitiesTests COMMAND test_utilities)

add_executable(test_stat_app test_stat_app.cpp
    "${CMAKE_SOURCE_DIR}/stat_app/src/stats_collector.cpp"
    "${CMAKE_SOURCE_DIR}/stat_app/src/utilities.cpp"
    tests.hpp)
target_include_directories(test_stat_app PRIVATE
    "${CMAKE_SOURCE_DIR}/stat_app/include"
)
target_link_libraries(test_stat_app PRIVATE logger_static)
target_compile_options(test_stat_app PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
add_test(NAME StatAppTests COMMAND test_stat_app)

add_executable(test_socket_sink test_socket_sink.cpp
    "${CMAKE_SOURCE_DIR}/console_app/src/utilities.cpp"
    tests.hpp)
target_include_directories(test_socket_sink PRIVATE
    "${CMAKE_SOURCE_DIR}/console_app/src"
)
target_link_libraries(test_socket_sink PRIVATE logger_static)
target_compile_options(test_socket_sink PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wsign-conversion
    -pedantic
)
add_test(NAME SocketSinkTests COMMAND test_socket_sink)


