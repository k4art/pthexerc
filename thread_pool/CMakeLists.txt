cmake_minimum_required(VERSION 3.14)

project(thread_pool)

set(CMAKE_C_STANDARD   11)
set(CMKAE_CXX_STANDARD 14)

include_directories(include)
include_directories(src)

enable_testing()

add_subdirectory(../fifo fifo)

add_subdirectory(test)
add_subdirectory(lib/googletest)

set(LIB_NAME ${PROJECT_NAME}_lib)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true include/*.h src/*.c)

add_library(${LIB_NAME} STATIC ${SOURCES})

target_link_libraries(${LIB_NAME} PUBLIC fifo_lib)

