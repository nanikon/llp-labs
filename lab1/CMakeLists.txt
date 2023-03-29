cmake_minimum_required(VERSION 3.22)
project(llp-lab1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(RUNTIME_OUTPUT_DIRECTORY build)
include_directories(src)
add_executable(main src/main.cpp
        src/file/file.cpp src/file/file.h
        src/structs/header.h src/structs/header.cpp 
        src/structs/block.h src/structs/block.cpp 
        src/structs/schema.h src/structs/schema.cpp
        src/structs/node.h src/structs/node.cpp
        src/api/api.h src/api/api_file.cpp src/api/api_node.cpp src/api/api_schema.cpp src/api/api_search.cpp
        src/test/unit_test/unit_test.h src/test/unit_test/unit_test.cpp
)
