cmake_minimum_required(VERSION 2.6)
project(PPS_TB_RC)

set(CAEN_LOCATION "/home/ppstb/external/CAENVMELib-2.41")
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

# CAEN stuff
include_directories("${CAEN_LOCATION}/include")
link_directories("${CAEN_LOCATION}/lib")
set(CMAKE_CXX_FLAGS "-DLINUX")
set(CMAKE_CXX_LINK_FLAGS "-lCAENVME -Wl")

add_executable(ppsFetch fetch_vme.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)

