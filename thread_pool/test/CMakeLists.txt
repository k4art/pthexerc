# Google Test

set(BINARY ${CMAKE_PROJECT_NAME}_test)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cc)

set(SOURCES ${TEST_SOURCES})

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest_main)

# Sample 1
add_executable(sample1)
target_link_libraries(sample1 PUBLIC ${CMAKE_PROJECT_NAME}_lib)
target_sources(sample1 PRIVATE usecases/sample1.c)

