cmake_minimum_required(VERSION 3.28)
project(wasd C)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "src/*.c")
file(GLOB HEADERS "src/*.h")

add_executable(wasd src/main.c ${SOURCES} ${HEADERS})
