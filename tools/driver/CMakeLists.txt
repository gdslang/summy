cmake_minimum_required (VERSION 2.8)
project (driver)

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

add_executable(driver ${SOURCES})
target_link_libraries(driver summy bjutil cppgdsl dl gmp gtest pthread elf fmt)
set_property(TARGET driver PROPERTY CXX_STANDARD 17)
