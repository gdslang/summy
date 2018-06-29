cmake_minimum_required (VERSION 2.8)
project (summy)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

file(GLOB_RECURSE SOURCES
    "src/**.cpp"
)
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/analysis/ismt/cvc_context.cpp"
  "${CMAKE_SOURCE_DIR}/src/analysis/ismt/ismt.cpp"
  "${CMAKE_SOURCE_DIR}/src/analysis/ismt/smt_builder.cpp"
  "${CMAKE_SOURCE_DIR}/src/analysis/ismt/smt_def_builder.cpp"
  "${CMAKE_SOURCE_DIR}/src/analysis/ismt/smt_value_builder.cpp")

include_directories("include/")

function(dep pack dirname)
  if(EXISTS "${CMAKE_SOURCE_DIR}/dependencies/${dirname}/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/${dirname})
  else()
    find_package(${pack} REQUIRED)
  endif()
endfunction(dep)

dep(gdsl gdsl-toolkit)
dep(bjutil bjutil)

#if(NOT TARGET bjutil)
#  ...

add_library(summy ${SOURCES})
set_property(TARGET summy PROPERTY CXX_STANDARD 17)
target_link_libraries(summy bjutil cppgdsl)

add_subdirectory(tools/sweep)
add_subdirectory(tools/legacy/demo_dstack)
add_subdirectory(tools/tester)

enable_testing()
add_test(all tools/tester/tester)
