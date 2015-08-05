project(DQM CXX)
# DQM modules needing a ROOT instance
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/share/root/cmake)
find_package(ROOT)

if (ROOT_FOUND)
  include_directories(${ROOT_INCLUDE_DIR})
  link_directories(${ROOT_LIBRARY_DIR})
  
  if(${ROOT_VERSION} LESS 6.0)
    set(GCC_COMPILE_FLAGS "-Wno-shadow -fPIC")
  else()
    set(GCC_COMPILE_FLAGS "-Wno-shadow -fPIC -std=c++11")
  endif()
  add_definitions(${GCC_COMPILE_FLAGS})
endif()

function(ADD_DQM exec)
  add_executable(${exec} ${PROJECT_SOURCE_DIR}/${exec}.cpp $<TARGET_OBJECTS:reader_lib> $<TARGET_OBJECTS:src_lib> $<TARGET_OBJECTS:ws_lib>)
  target_link_libraries(${exec} ${ROOT_LIBRARIES} ${ROOT_COMPONENT_LIBRARIES})
  #set_property(TARGET ${exec} PROPERTY EXCLUDE_FROM_ALL true)
  set_property(TARGET ${exec} PROPERTY LINK_FLAGS "-lsqlite3")
endfunction()

if (ROOT_FOUND)
  add_dqm(gastof)
  add_dqm(quartic)
  add_dqm(daq)
  add_dqm(db)
endif()

