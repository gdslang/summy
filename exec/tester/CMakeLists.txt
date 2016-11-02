cmake_minimum_required (VERSION 2.8)
project (tester)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test/analysis/ismt/ismt.cpp)

include_directories("include/")
include_directories("../../include/")

function(dep dirname)
  if(EXISTS ${CMAKE_SOURCE_DIR}/dependencies/${dirname}/CMakeLists.txt)
    add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/${dirname} ${dirname})
  endif()
endfunction(dep)

dep("${CMAKE_SOURCE_DIR}/dependencies/${dirname}/CMakeLists.txt")

add_executable(tester ${SOURCES})
target_link_libraries(tester summy bjutil cppgdsl gmp gtest pthread elf)
set_property(TARGET tester PROPERTY CXX_STANDARD 14)
