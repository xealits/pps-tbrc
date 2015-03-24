cmake_minimum_required(VERSION 2.6)
project(PPS_TB_RC)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(src_lib OBJECT ${sources})
include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(ppsRun main.cpp $<TARGET_OBJECTS:src_lib>)
add_executable(listener listener.cpp $<TARGET_OBJECTS:src_lib>)

