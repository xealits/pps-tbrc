cmake_minimum_required(VERSION 2.6)
project(PPS_TB_RC)

set(CAEN_LOCATION "/opt/xdaq/lib")
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

# Here have tests
add_subdirectory(test EXCLUDE_FROM_ALL)

# CAEN stuff
add_library(caen SHARED IMPORTED)
set_property(TARGET caen PROPERTY IMPORTED_LOCATION "${CAEN_LOCATION}/libCAENVME.so")
set(CMAKE_CXX_FLAGS "-DLINUX")
#set(CMAKE_CXX_LINK_FLAGS "-lCAENVME -Wl")
set(CMAKE_CXX_LINK_FLAGS "-lCAENVME")

add_executable(ppsFetch fetch_vme.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
target_link_libraries(ppsFetch caen)
