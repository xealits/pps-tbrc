cmake_minimum_required(VERSION 2.6)
project(PPS_TB_RC)

set(CAEN_LOCATION "/usr/lib")
set(GCC_COMPILE_FLAGS "-Wall -fPIC -O2 -g -lsqlite3")
set(GCC_LINK_FLAGS "-lsqlite3")
add_definitions(${GCC_COMPILE_FLAGS})

file(GLOB vme_sources ${PROJECT_SOURCE_DIR}/src/VME*.cpp)
file(GLOB nim_sources ${PROJECT_SOURCE_DIR}/src/NIM*.cpp)
add_library(det_lib OBJECT ${vme_sources} ${nim_sources})

# File reader
file(GLOB reader_sources ${PROJECT_SOURCE_DIR}/src/FileReader.cpp)
add_library(reader_lib OBJECT ${reader_sources})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM sources ${vme_sources})
list(REMOVE_ITEM sources ${nim_sources})
list(REMOVE_ITEM sources ${reader_sources})
add_library(src_lib OBJECT ${sources})
set_property(TARGET src_lib PROPERTY LINK_FLAGS "-lsqlite3")

include_directories("${PROJECT_SOURCE_DIR}/include")

# WebSocket
add_subdirectory(${PROJECT_SOURCE_DIR}/external)
include_directories("${PROJECT_SOURCE_DIR}/external")

add_executable(ppsRun main.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
set_property(TARGET ppsRun PROPERTY LINK_FLAGS "-lsqlite3")
add_executable(listener listener.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
set_property(TARGET listener PROPERTY LINK_FLAGS "-lsqlite3")

# Here have tests
add_subdirectory(test EXCLUDE_FROM_ALL)

# Here have DQM clients
add_subdirectory(dqm EXCLUDE_FROM_ALL)

# CAEN stuff
add_library(caen SHARED IMPORTED)
set_property(TARGET caen PROPERTY IMPORTED_LOCATION "${CAEN_LOCATION}/libCAENVME.so")

# Copy the XML configuration files to the config/ folder
file(GLOB config_files RELATIVE ${PROJECT_SOURCE_DIR} config/*.xml)
foreach(_script ${config_files})
  configure_file(${_script} ${PROJECT_BINARY_DIR}/${_script} COPYONLY)
endforeach()

set(CMAKE_CXX_FLAGS "-DLINUX")

add_executable(ppsFetch fetch_vme.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:det_lib> $<TARGET_OBJECTS:ws_lib>)
target_link_libraries(ppsFetch caen)
set_property(TARGET ppsFetch PROPERTY LINK_FLAGS "-lCAENVME -ltinyxml2 -lsqlite3")

add_executable(HVsettings change_hv_settings.cpp $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:det_lib> $<TARGET_OBJECTS:ws_lib>)
target_link_libraries(HVsettings caen)
set_property(TARGET HVsettings PROPERTY LINK_FLAGS "-lCAENVME -ltinyxml2 -lsqlite3")
