cmake_minimum_required(VERSION 3.14)

project(fifo)

set(CMAKE_C_STANDARD   11)
set(CMKAE_CXX_STANDARD 14)

include_directories(include)

# if it is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  enable_testing()

  add_subdirectory(test)
  add_subdirectory(lib/googletest)
endif()

set(LIB_NAME ${PROJECT_NAME}_lib)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true include/*.h src/*.c)

add_library(${LIB_NAME} STATIC ${SOURCES})
target_include_directories(${LIB_NAME} PUBLIC include)

