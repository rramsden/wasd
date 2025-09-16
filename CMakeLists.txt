cmake_minimum_required(VERSION 3.28)
project(wasd C)

set(CMAKE_C_STANDARD 99)

file(GLOB SOURCES "src/*.c")
file(GLOB HEADERS "src/*.h")

add_custom_target(run_as_admin
        COMMAND powershell -Command "Start-Process -Verb runAs -FilePath $<TARGET_FILE:wasd>"
        DEPENDS wasd
        WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_executable(wasd src/main.c ${SOURCES} ${HEADERS}
        src/grid.c
        src/grid.h)
