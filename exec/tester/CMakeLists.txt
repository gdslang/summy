set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

cmake_minimum_required (VERSION 2.8)
project (tester)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test/analysis/ismt/ismt.cpp)

include_directories("include/")
include_directories("../../include/")

add_executable(tester ${SOURCES})
target_link_libraries(tester summy -lbjutil -lcppgdsl -lgdsl-multiplex -ldl -lreadhex -lgmp -lgtest -lpthread -lelf)
set_property(TARGET tester PROPERTY CXX_STANDARD 14)
