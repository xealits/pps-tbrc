cmake_minimum_required(VERSION 2.6)
project(PPS_TB_RC)

set(GCC_COMPILE_FLAGS "-Wall -fPIC -O2")
add_definitions(${GCC_COMPILE_FLAGS})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(src_lib OBJECT ${sources})
include_directories("${PROJECT_SOURCE_DIR}/include")

# WebSocket
add_subdirectory(${PROJECT_SOURCE_DIR}/external)
include_directories("${PROJECT_SOURCE_DIR}/external")

add_executable(ppsRun main.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
add_executable(listener listener.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
add_executable(reader test/reader.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)

set(CMAKE_CXX_FLAGS "-lCAENVME -I$(TOP)/$(VMEd)/CAENVMELib -I$(TOP)/$(VMEd)/CAENVMETool -DLINUX")

add_executable(ppsFetch fetch_vme.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)

