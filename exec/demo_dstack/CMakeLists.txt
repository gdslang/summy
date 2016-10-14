set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

cmake_minimum_required (VERSION 2.8)
project (demo_dstack)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)

include_directories("include/")
include_directories("../../include/")

add_executable(demo_dstack ${SOURCES})
target_link_libraries(demo_dstack summy bjutil dl gmp gtest pthread elf)
set_property(TARGET demo_dstack PROPERTY CXX_STANDARD 14)
