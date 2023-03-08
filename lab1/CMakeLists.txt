cmake_minimum_required(VERSION 3.22)
project(llp-lab1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(RUNTIME_OUTPUT_DIRECTORY build)
include_directories(src)
add_executable(main src/main.cpp
        src/file/file.cpp src/file/file.h
        src/structs/header.h
        src/structs/schema.h
        src/test/unit_test/unit_test.h src/test/unit_test/unit_test.cpp
)
