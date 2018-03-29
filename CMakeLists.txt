cmake_minimum_required (VERSION 2.6)
project (Test_SeeJSON C)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wno-unused-function -Wall")
endif()

add_library(SeeJSON SHARED src/SeeJSON.c)
add_executable(Test_SeeJSON src/Test_SeeJSON.c)
target_link_libraries(Test_SeeJSON SeeJSON)