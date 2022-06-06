cmake_minimum_required(VERSION 3.16)
project(json_parser C)

set(CMAKE_C_STANDARD 99)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/testlib/
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/
    ${CMAKE_CURRENT_SOURCE_DIR}/json/
)

add_executable(
        json_parser
        main.c
        json/json_lex.c
        testlib/testlib.c
        tests/test_json_lex.c
        json/json_parse.c
        json/json.c
        tests/test_json_parse.c)
