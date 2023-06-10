cmake_minimum_required(VERSION 3.20)
project(hw3 C)

set(CMAKE_C_STANDARD 99)

add_executable(hw3 queue.c request.c request.h output.c server.c segel.c segel.h)
