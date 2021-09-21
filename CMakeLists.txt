cmake_minimum_required(VERSION 3.18)
project(coromoro)

set(coromoro_source_files "src/main.cpp")

find_package(Threads REQUIRED)

include_directories("./inc")

add_compile_options(-Wall -Wextra -pedantic -O0 -g)

add_executable(coromoro ${coromoro_source_files})  # Specifies the executable to build.
target_link_libraries(coromoro Threads::Threads)
