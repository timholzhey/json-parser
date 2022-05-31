cmake_minimum_required(VERSION 3.20)
project(json_parser C)

set(CMAKE_C_STANDARD 99)

add_executable(json_parser main.c json_lex.c json_lex.h)
