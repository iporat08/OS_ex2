cmake_minimum_required(VERSION 3.13)
project(ex2)

set(CMAKE_CXX_STANDARD 11)

add_executable(ex2 main.cpp uthreads.h uthreads.cpp Thread.cpp Thread.h)