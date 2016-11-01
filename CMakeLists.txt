set(CMAKE_CXX_COMPILER "clang++")

cmake_minimum_required (VERSION 2.8)
project (summy)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)

include_directories("include/")

if(EXISTS "${CMAKE_SOURCE_DIR}/dependencies")
  add_subdirectory(dependencies/gdsl-toolkit)
  add_subdirectory(dependencies/bjutil)
else()
  find_package(gdsl REQUIRED)
  find_package(bjutil REQUIRED)
endif()

#if(NOT TARGET bjutil)
#  ...

add_library(summy ${SOURCES})
set_property(TARGET summy PROPERTY CXX_STANDARD 14)
target_link_libraries(summy bjutil cppgdsl)

add_subdirectory(exec/demo_dstack)
add_subdirectory(exec/all_f)
add_subdirectory(exec/tester)

enable_testing()
add_test(all exec/tester/tester)
