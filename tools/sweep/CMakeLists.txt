cmake_minimum_required (VERSION 2.8)
project (sweep)

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)

include_directories("include/")
include_directories("../../include/")

function(dep dirname)
  if(EXISTS ${CMAKE_SOURCE_DIR}/dependencies/${dirname}/CMakeLists.txt)
    add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/${dirname} ${dirname})
  endif()
endfunction(dep)

dep(fmt)

add_executable(sweep ${SOURCES})
target_link_libraries(sweep summy bjutil cppgdsl dl gmp gtest pthread elf fmt)
set_property(TARGET sweep PROPERTY CXX_STANDARD 17)
