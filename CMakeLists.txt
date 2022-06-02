cmake_minimum_required(VERSION 3.16)
project(json_parser C)

set(CMAKE_C_STANDARD 99)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/testlib/
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
)

add_executable(
        json_parser
        main.c
        json_lex.c
        testlib/testlib.c
        tests/test_lex.c
)
