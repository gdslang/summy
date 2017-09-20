cmake_minimum_required (VERSION 2.8)
project (all_f)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)

include_directories("include/")
include_directories("../../include/")

add_executable(all_f ${SOURCES})
target_link_libraries(all_f summy bjutil cppgdsl dl gmp gtest pthread elf)
set_property(TARGET all_f PROPERTY CXX_STANDARD 14)
