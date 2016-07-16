set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

cmake_minimum_required (VERSION 2.8)
project (all_f)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)

include_directories("include/")
include_directories("../../include/")

add_executable(all_f ${SOURCES})
target_link_libraries(all_f summy -lbjutil -lcppgdsl -lgdsl-multiplex -ldl -lreadhex -lgmp -lgtest -lpthread -lelf)
set_property(TARGET all_f PROPERTY CXX_STANDARD 14)
