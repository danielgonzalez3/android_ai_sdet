cmake_minimum_required(VERSION 3.16)
project(gpqa_eval LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gpqa_eval src/gpqa_eval.cpp)

# Enable more warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(gpqa_eval PRIVATE -Wall -Wextra -Wpedantic)
endif() 